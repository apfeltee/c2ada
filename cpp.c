/*
 * Routines to implement a C preprocessor
 */

#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errors.h"
#include "files.h"
#include "hash.h"
#include "buffer.h"
#include "cpp.h"
#include "cpp_hide.h"
#include "cpp_eval.h"
#include "allocate.h"
#include "units.h"
#include "configure.h"

extern int translate_comments;

/* from scan.c */
extern comment_block_pt fetch_comment_block(void);

#undef NULL
#define NULL            0L

#ifndef MAX_SEARCH
#define MAX_SEARCH      32
#endif

#define SYNC_RST_CSP    0
#define SYNC_INC_CSP    1

struct cpp_file_t {
    char *	path;
    char *	text;	       /* file buffer */
    off_t	file_size;
    int		fd;
    int		reference_count;
    int	        is_header;
    cpp_file_pt next_cpp_file;
};


static cpp_file_t *      open_files;
static scan_position_t * curpos;
boolean                  current_unit_is_header;

unsigned char cpp_char_class[128];
static char   class_initialized;

#define add(buf,c)              buf_add(buf, (int)(c))
#define cond_add(buf,c)         {if (buf) {add((buf), (c));}}

#define UNHANDLED()             unhandled(__FILE__, __LINE__)

static buffer_t cppbuf;
static boolean  buffer_initialized;

static char *search_paths[MAX_SEARCH];
static int   search_index;

static char *system_search_paths[MAX_SEARCH] = {"/usr/include"};
static int system_search_index = 1;

/* at_file_start is set TRUE when a new file is opened,
 * and FALSE once the first token is read from the file.
 */
boolean at_file_start;

/* TBD: this is a temporary flag to bypass the handling of
 * macros as constants until the whole facility works better.
 */
boolean do_const_macros = TRUE;


/*
 * The following vars control the state of conditionals
 */
static cpp_control_state_t control_state;

#define scanning()          (control_state.cur_scope == control_state.gen_scope)
#define skipping()          (control_state.cur_scope != control_state.gen_scope)
#define parsing()           (control_state._parsing)

#define want_comments(x)    ((translate_comments) ? x : NULL)

static char *last_file;
static int last_line;

#define FN                  fname()
#define LN                  fline()

static int grok_actual_param(int);
static boolean is_const_macro(macro_t * mac);


static char*
fname(void)
    /* The name of the current source file. */
{
    if (curpos == NULL) {
        assert(last_file != NULL);
        return last_file;
    }
    if (curpos->scan_pos == 0) return NULL;
    return file_name(curpos->scan_pos);
}

static int
fline()
{
    if (curpos == NULL) {
        assert(last_line != 0);
        return last_line;
    }
    return line_number(curpos->scan_pos);
}

static void
range_check(n, max, msg)
    int n, max;
    char *msg;
{
    if (n >= max) {
        fatal(FN, LN, "%s: %d > %d", msg, n+1, max);
    }
}

static void
unhandled(file, line)
    char *file;
    int line;
{
    fatal(FN, LN, "Unhandled case %s:%d", file, line);
}

static void
bad_directive()
{
    if (scanning()) {
        error(FN, LN, "Undefined preprocessor directive");
    }
}

static void
bad_include()
{
    error(FN,LN,"Malformed include directive");
}

static void
unexpected(msg)
    char *msg;
{
    error(FN, LN, "Unexpected %s", msg);
}

static void
unexpected_eof(msg)
    char *msg;
{
    char buf[128];
    sprintf(buf, "end of file %s", msg);
    unexpected(buf);
}

static char*
charstr(c)
    int c;
{
    if (is_eof(c))      return "end of file";
    if (is_eol(c))      return "end of line";
    if (is_alpha(c))    return "identifier";
    if (is_digit(c))    return "number";
    if (is_white(c))    return "white space";
    if (is_punct(c))    return "puctuation";
    return "crap";
}

static void
expected(msg, c)
    char *msg;
    int c;
{
    error(FN, LN, "Expected %s got %s", msg, charstr(c));
}

static int
levels_nested()
{
    scan_position_t *p;
    int level = 0;

    for (p = curpos; p; p = p->scan_next) {
        if (p->scan_kind == scan_file) {
            level++;
        }
    }

    return level;
}

static void
add_position_directive(buf, new_line)
    buffer_t *buf;
    int new_line;
{
    char b[40];

    sprintf(b, "\n# %d \"", LN);
    buf_add_str(buf, b);
    buf_add_str(buf, FN);
    sprintf(b, "\" %d", levels_nested());
    buf_add_str(buf, b);
    if (new_line) {
        add(buf, '\n');
    }
}


static void
init_char_class()
{
    int i;

    if (class_initialized) return;

    cpp_char_class[0] = END_INPUT;
    cpp_char_class[PARAM_START] = PARAM_START;
    cpp_char_class['#'] = MSTART;
    cpp_char_class['_'] = ALPHA;
    cpp_char_class['$'] = ALPHA;

    for (i = 'g'; i <= 'z'; i++) {
        cpp_char_class[i] = ALPHA;
    }

    for (i = 'G'; i <= 'Z'; i++) {
        cpp_char_class[i] = ALPHA;
    }

    for (i = 'A'; i <= 'F'; i++) {
        cpp_char_class[i] = ALPHA | XDIGIT;
    }

    for (i = 'a'; i <= 'f'; i++) {
        cpp_char_class[i] = ALPHA | XDIGIT;
    }

    for (i = '0'; i <= '7'; i++) {
        cpp_char_class[i] = DIGIT | XDIGIT;
    }

    cpp_char_class['8'] = DIGIT | XDIGIT;
    cpp_char_class['9'] = DIGIT | XDIGIT;

    cpp_char_class[' '] = WHITE;
    cpp_char_class['\t'] = WHITE;
    cpp_char_class['\f'] = WHITE;
    cpp_char_class['\v'] = WHITE;
    cpp_char_class['\n'] = END_OF_LINE;
    cpp_char_class['\r'] = END_OF_LINE;

    class_initialized = 1;
}

static void
init_cppbuf()
{
    if (buffer_initialized) {
        return;
    }
    buf_init(&cppbuf);
}


static void
rm_file_from_list(f)
    cpp_file_t *f;
{
    cpp_file_t *p, *last;

    for (p = open_files, last = NULL; p; p = p->next_cpp_file) {
        if (p == f) break;
        last = p;
    }
    assert(p == f);
    if (last == NULL) {
        open_files = f->next_cpp_file;
    }
    else {
        last->next_cpp_file = f->next_cpp_file;
    }
}

static cpp_file_t*
find_open_file(char * path)
{
    cpp_file_t *f;

    for (f = open_files; f; f = f->next_cpp_file) {
        assert(f->next_cpp_file != f);
        if (! strcmp(f->path, path)) {
            return f;
        }
    }

    return NULL;
}

void
cpp_search_path(path)
    char *path;
{
    if (search_index >= MAX_SEARCH) {
        fatal(__FILE__,__LINE__,"Too many search paths added");
    }
    search_paths[search_index++] = path;
}

void
cpp_system_search_path(char * path)
    /* Set the path to use for  #include <blah.h>  directives. */
    /* /usr/include is always the last element in the list of paths */
{
    if (system_search_index >= MAX_SEARCH) {
        fatal(__FILE__,__LINE__,"Too many system search paths added");
    }
    system_search_paths[system_search_index-1] = path;
    system_search_paths[system_search_index++] = "/usr/include";
}

int
in_system_search_path( char * path )
    /*
     * Tests whether argument path is in any system_search_paths directory.
     * Returns length of directory prefix part of pathname, if so;
     * otherwise returns 0.
     */
{
    int    n;
    char * syspath;
    int    syspathlen;
    int    result;
    for (n=0; n<system_search_index; n++) {
        syspath = system_search_paths[n];
        syspathlen = strlen(syspath);
        if (strncmp(path, syspath, syspathlen)==0) {
            result = syspathlen;
            while (path[result]=='/') result++;
            return result;
        }
    }
    return 0;
}



static cpp_file_t*
attempt_open(char * path)
{
    cpp_file_t *f;
    char *mapaddr;
    size_t fsize;
    int fd;

    f = find_open_file(path);

    if (f != NULL) {
        f->reference_count++;
        return f;
    }

    if ((fd = open(path, O_RDONLY)) == -1) {
        /* Can't access for reading */
        return NULL;
    }

    if ((fsize = sizeof_file(fd)) == -1) {
        close(fd);
        return NULL;
    }

    mapaddr = (char*) map_file(fd, fsize);

    if (mapaddr == (char*)-1) {
        /* failed mmap */
        close(fd);
        return NULL;
    }

    f = NEW(cpp_file_t);
    f->path = new_string(path);
    f->fd = fd;
    f->text = mapaddr;
    f->file_size = fsize;
    f->reference_count = 1;
    assert(open_files != f);
    f->next_cpp_file = open_files;
    open_files = f;

    return f;
}

static char* local_copy(buffer_t*buf, char* str, int size); /*fwd ref */

static cpp_file_t *
attempt_open_buf( buffer_t * lbuf )
    /* Like attempt_open, but takes a buffer_t* argument. */
{
    char * path;
    cpp_file_t * result;
    char   ident[128];   /* local string holder */

    path = local_copy( lbuf, ident, sizeof(ident) );
    result = attempt_open(path);
    if (path!= ident) {
        free(path);
    }
    return result;
}

static cpp_file_t *
attempt_open_w_searchpaths(char * name,
                           buffer_t * lbuf,
                           char *dirs[],
                           int ndirs)
    /*
     * Try to find a filename <name> in any of the <ndirs> directories
     * <dirs>. <lbuf> is a scratch buffer.
     */
{
    int i;
    cpp_file_t * result;

    for (i = 0; i< ndirs; i++) {
        buf_init(lbuf);
        buf_add_str(lbuf, dirs[i]);
        buf_add_str(lbuf, "/");
        buf_add_str(lbuf, name);
        result = attempt_open_buf(lbuf);
        if (result) return result;
    }
    return 0;
}




static void
push_file(buffer_t *buf, cpp_file_t * f)
{
    scan_position_t *pos;
    char *           dot;

    pos = NEW(scan_position_t);
    pos->scan_kind  = scan_file;
    pos->scan.file  = f;
    pos->scan_index = 0;
    pos->scan_pos   = add_file(f->path);
    pos->scan_next  = curpos;

    curpos = pos;

    dot = strrchr(f->path, '.');
    assert((dot != NULL) && ((dot[1] == 'c') || (dot[1] == 'h')));
    current_unit_is_header = f->is_header = (dot[1] == 'h');

    add_position_directive(buf,1);
    at_file_start = TRUE;
}


int
cpp_open(path)
    char *path;
{
    cpp_file_t *f;

    init_char_class();
    init_cppbuf();

    control_state.skip_else = 0;
    control_state.cur_scope = 0;
    control_state.gen_scope = 0;
    control_state._parsing = 0;

    f = attempt_open(path);

    if (f == NULL) {
        return 1;
    }

    push_file(&cppbuf,f);

    return 0;
}

void
cpp_cleanup()
{
    cpp_file_t *f, *next;

    curpos = NULL;

    for (f = open_files; f; f = next) {
        next = f->next_cpp_file;
        unmap_file(f->text, f->file_size);
        close(f->fd);
        deallocate(f);
    }

    open_files = NULL;

    buf_destroy(&cppbuf);
}

void
cpp_set_state(newpos, new_state, savepos, save_state)
    scan_position_t *newpos, **savepos;
    cpp_control_state_t *new_state, *save_state;
{
    *savepos = curpos;
    *save_state = control_state;
    control_state = *new_state;
    curpos = newpos;
}

static void
kill_file(f)
    cpp_file_t *f;
{
    unmap_file(f->text, f->file_size);
    close(f->fd);
    rm_file_from_list(f);
    deallocate(f);
}

static void
finished_with(scan_position_t * s)
{
    cpp_file_t * f;

    last_file = file_name(s->scan_pos);
    last_line = line_number(s->scan_pos);

    switch (s->scan_kind) {
      case scan_file:
        f = curpos->scan.file;
        deallocate(s);
        f->reference_count--;
        if (f->reference_count == 0) {
            kill_file(f);
        }
        break;
      case scan_macro_expansion:
      case scan_text:
        deallocate(s);
        break;
      default:
        assert(0);
        break;
    }
}

static void
unget_char()
{
    if (curpos == NULL) return;
    assert(curpos->scan_index > 0);
    curpos->scan_index--;
}


static int
next_char(void)
    /* Return the next character from the current input source */
{
    register scan_position_t *cp = curpos; /* Get global curpos into a reg */
    cpp_file_t *              f;
    macro_t *                 m;
    scan_position_t *         next;
    scan_kind_t               kind;
    int                       result;

top:
    if (cp == NULL)
    return 0;

    kind = cp->scan_kind;
    switch (kind) {
    case scan_file:
        f = cp->scan.file;
        if (cp->scan_index >= f->file_size) {
            /* done with file; pop to out file */
            next = cp->scan_next;
            finished_with(cp);
            set_cur_unit_trailer_comment(fetch_comment_block());
            cp = curpos = next;
            if (next != NULL) {
                current_unit_is_header =
                    next->scan.file->is_header;
            }
            goto top; /* return next_char(); */
        }
        result = f->text[cp->scan_index++];
        /* Look ahead & detect DOS/WIN CR-LF EOL;
         * skip the CR & return the LF
         */
        if (('\r' == result) && ('\n' == f->text[cp->scan_index]))
        {
            result = f->text[cp->scan_index++];
        }

        break;
    case scan_macro_expansion:
        m = cp->scan.expansion.expand_macro;
        if (cp->scan_index >= m->macro_body_len) {
            /* We're done with macro; pop to next source */
            next = cp->scan_next;
            finished_with(cp);
            cp = curpos = next;
            goto top; /* return next_char(); */
        }
        result = m->macro_body[cp->scan_index++];
        break;
    default:
        assert(kind == scan_text);
        result = cp->scan.text[cp->scan_index++];
        if (result == 0) {
            /* Done with string; pop to next source */
            next = cp->scan_next;
            finished_with(cp);
            cp = curpos = next;
            goto top; /* return next_char */
        }
        break;
    }

    return result;
}


static void
incline(sync)
    int sync;
{
    if (sync) {
        control_state.position++;
    }
    else {
        /* force file position directive to be added */
        control_state.position = 0;
    }
    switch (curpos->scan_kind) {
      case scan_file:
        curpos->scan_pos++;
        break;
      default:
        break;
    }
}

static void
check_position(buf)
    buffer_t *buf;
{
    if (parsing()) return;
    switch (curpos->scan_kind) {
      case scan_file:
        if (control_state.position != curpos->scan_pos) {
            control_state.position = curpos->scan_pos;
            add_position_directive(buf, 0);
        }
        break;
      default:
        assert(0);
        break;
    }
}

static void
comment_start(buf)
    buffer_t *buf;
{
    if (translate_comments && buf != NULL) {
        add(buf, '/');
        add(buf, '*');
    }
}

static void
cpp_comment_start(buf)
    buffer_t *buf;
{
    if (translate_comments && buf != NULL) {
        add(buf, '/');
        add(buf, '/');
    }
}

static int
scan_white(buf, c)
    buffer_t *buf;
    int c;
{
    while (is_white(c)) {
        cond_add(buf, c);
        c = next_char();
    }
    return c;
}

static int
skip_white(c)
    int c;
{
    return scan_white(NULL, c);
}

/* When called for a #define, this fails almost all the time
 * (see E_CIFAIL in error.h, the only one which worked)
 * even though the plain C version below this worked just fine.
 */

static int
scan_cpp_comment(buffer_t * buf, boolean want_delim)
{
    int c = next_char();

    for (;;) {
        switch (classof(c)) {
          case END_INPUT:
            return c;
          case DIGIT | XDIGIT:
          case ALPHA:
          case ALPHA | XDIGIT:
          case WHITE:
          case MSTART:
          case PARAM_START:
          case PUNCT:
            cond_add(buf,c);
            c = next_char();
            break;
          case END_OF_LINE:
            incline(SYNC_RST_CSP);
            cond_add(buf,c);
            /* There shouldn't be another char to get; don't get it! */
            /*c = next_char();*/

            /* Added presumptively from scan_c_comment as a bread crumb */
		    /*if (want_delim) { ... } */
            return c;
          default:
            UNHANDLED();
            break;
        }
    }
}


static int
scan_c_comment(buffer_t * buf, boolean want_delim)
{
    int      c = next_char();

    for (;;) {
        switch (classof(c)) {
          case END_INPUT:
            return c;
          case DIGIT | XDIGIT:
          case ALPHA:
          case ALPHA | XDIGIT:
          case WHITE:
          case MSTART:
            cond_add(buf,c);
            c = next_char();
            break;
          case END_OF_LINE:
            incline(SYNC_RST_CSP);
            cond_add(buf,c);
            c = next_char();
            break;
          case PARAM_START:
            c = next_char();
            c = next_char();
            break;
          case PUNCT:
            switch (c) {
              case '*':
                c = next_char();
                if (c == '/') {
		    if (want_delim) {
			cond_add(buf,'*');
			cond_add(buf,'/');
		    }
                    return next_char();
		} else {
		    cond_add(buf,'*');
                }
                break;
              case '\\':
                cond_add(buf,c);
                c = next_char();
                cond_add(buf,c);
                if (is_eol(c)) {
                    incline(SYNC_RST_CSP);  /* value added */
                }
                else if (is_eof(c)) {
                    return c;
                }
                c = next_char();
                break;
              default:
                cond_add(buf,c);
                c = next_char();
                break;
            }
            break;
          default:
            UNHANDLED();
            break;
        }
    }
}


static int
scan_to_end(buffer_t *buf, buffer_t *cbuf, int c)
    /* Scan to the end of the current line
     * <buf> is the buffer to copy non-comment part of line to;
     * <cbuf> the buffer to copy comments to.
     * <buf> and <cbuf> may be the same.
     * If <cbuf> is null then no comments are copied.
     */
{
    while (! (is_eol(c) || is_eof(c))) {
        switch (c) {
          case '/':
            c = next_char();
            if (c == '*') {
		if (buf==cbuf) comment_start(cbuf);
                c = scan_c_comment(cbuf, buf==cbuf);
            } else if (c == '/') {
                if (buf==cbuf) cpp_comment_start(cbuf);
                c = scan_cpp_comment(cbuf, buf==cbuf);
            } else {
                cond_add(buf,'/');
            }
            break;
          case '\\':
            c = next_char();
            if (is_eol(c)) {
                incline(SYNC_RST_CSP);
                c = next_char();
            }
            else if (is_eof(c)) {
                if (buf != NULL) {
                    add(buf, '\\');
                }
                return c;
            }
            else if (c == '\\') {
                if (buf != NULL) {
                    add(buf, '\\');
                    add(buf, '\\');
                }
                c = next_char();
            }
            else if (buf != NULL) {
                add(buf, '\\');
            }
            break;
          default:
            if (buf != NULL) {
                add(buf, c);
            }
            c = next_char();
            break;
        }
    }

    return c;
}


static int
skip_to_end(int c)
    /* skip to the end of the current line */
{
    return scan_to_end(NULL, NULL, c);
}

static int
finish_num(buf, c, mask)
    buffer_t *buf;
    int c, mask;
{
    while (! is_eof(c)) {
        if ((cpp_char_class[c] & mask) == 0) break;
        add(buf, c);
        c = next_char();
    }

    return c;
}

static int
maybe_magnitude(buf, c)
    buffer_t *buf;
    int c;
{
    if (c == 'e' || c == 'E') {
        add(buf, c);
        c = next_char();
        if (c == '+' || c == '-') {
            add(buf, c);
            c = next_char();
        }
        c = finish_num(buf, c, DIGIT);
    }

    return c;
}


static int
scan_number(buf, c)
    buffer_t *buf;
    int c;
{
    add(buf, c);

    if (c == '0') {
        c = next_char();
        if (c == 'x' || c == 'X') {
            add(buf, c);
            c = finish_num(buf, next_char(), XDIGIT);
        }
        else {
            c = finish_num(buf, c, DIGIT);
        }
    }
    else {
        c = finish_num(buf, next_char(), DIGIT);
    }

    if (int_modifier(c)) {
        do {
            add(buf, c);
            c = next_char();
        } while (int_modifier(c));
        return c;
    }

    if (c == '.') {
        add(buf, c);
        c = finish_num(buf, next_char(), DIGIT);
        c = maybe_magnitude(buf, c);
        if (float_modifier(c)) {
            add(buf, c);
            c = next_char();
        }
        return c;
    }

    c = maybe_magnitude(buf, c);

    if (int_modifier(c)) {
        do {
            add(buf, c);
            c = next_char();
        } while (int_modifier(c));
    }

    return c;
}


static int
scan_ident(buffer_t *buf, int c)
    /* Scan an identifier into buffer buf.
     * Argument c is first character of identifier.
     * Returns first character following identifier.
     */
{
    for ( ; is_alpha_numeric(c) ; c = next_char() ) {
        add(buf, c);
    }
    return c;
}

static int
scan_to_del(buffer_t *buf, int c, int del)
    /* Scan until the delimiter del is encountered.
     * del must be of class PUNCT.
     * Return value is del, unless end of line or input encountered first.
     */
{
    for (;;) {
        switch (classof(c)) {
          case END_INPUT:
            return c;
          case MSTART:
          case ALPHA:
          case ALPHA | XDIGIT:
          case DIGIT | XDIGIT:
          case WHITE:
            add(buf, c);
            c = next_char();
            break;
          case END_OF_LINE:
            return c;
          case PARAM_START:
            c = grok_actual_param(next_char());
            break;
          case PUNCT:
            switch (c) {
              case '\\':
                c = next_char();
                if (is_eol(c)) {
                    incline(SYNC_RST_CSP);
                    c = next_char();
                }
                else if (c == '\'' || c == '"' || c == '\\') {
                    add(buf, '\\');
                    add(buf, c);
                    c = next_char();
                }
                else {
                    add(buf, '\\');
                }
                break;
              default:
                if (c == del) {
                    return c;
                }
                add(buf, c);
                c = next_char();
                break;
            }
            break;
          default:
            UNHANDLED();
            break;
        }
    }
}


static int
scan_string( buffer_t * buf, int c )
    /* Scan a string literal. */
{
    c = scan_to_del(buf, c, '"');
    if (c != '"') {
        expected("'\"'", c);
        return c;
    } else {
        add(buf, c);
        return next_char();
    }
}

static int
scan_char_const(buf, c)
    buffer_t *buf;
    int c;
{
    c = scan_to_del(buf, c, '\'');
    if (c != '\'') {
        expected("single quote", c);
        return c;
    } else {
        add(buf, c);
        return next_char();
    }
}


static int
grok_formals(formals, nformals, buf, c)
    char ***formals;
    int *nformals;
    buffer_t *buf;
    int c;
{
    char *f[256];
    char **fp;
    int nf = 0;
    int i;

    for (;;) {
        c = skip_white(c);
        if (! is_alpha(c)) break;
        c = scan_ident(buf, c);
        range_check(nf, 256, "Too many formal parameters");
        f[nf++] = buf_get_str(buf);
        c = skip_white(c);
        if (c != ',') break;
        c = next_char();
    }

    *nformals = nf;

    if (nf == 0) {
        *formals = NULL;
    }
    else {
        fp = (char**) malloc(sizeof(char*) * nf);
        for (i = 0; i < nf; i++) {
            fp[i] = f[i];
        }
        *formals = fp;
    }

    return c;
}

static char*
local_copy(buffer_t *buf, char*str, int size)
{
    int len = buf_count(buf);

    if (len < size) {
        buf_move_to(buf, str);
        return str;
    }

    return buf_get_str(buf);
}

static void
grok_param(outbuf, idbuf, formals, nformals, xpect)
    buffer_t *outbuf, *idbuf;
    char *formals[];
    int nformals, xpect;
    /* See if the name in idbuf is one of the formal parameters
     * of the macro whose body it occurred in.
     * If it is, place a special parameter ID sequence in outbuf;
     * otherwise, unless we're expecting a parameter name (xpect),
     * place in the name in outbuf.
     */
{
    char ident[128];
    char *name;
    int i;

    name = local_copy(idbuf, ident, sizeof(ident));

    for (i = 0; i < nformals; i++) {
        if (! strcmp(name, formals[i])) {
            add(outbuf, PARAM_START);
            add(outbuf, i+1);
            goto end_subp;
        }
    }

    if (xpect) {
        error(FN, LN, "Expected macro formal got %s", name);
    }

    buf_add_str(outbuf, name);

  end_subp:
    if (name != ident) {
        free(name);
    }
}


static int push_string(char * string);

static int
grok_define( buffer_t * buf, int c )
    /* grok_define is called when #define has been found at the start of
     * a line.
     */
{
    buffer_t lbuf;
    buffer_t cbuf;  /* buffer for comment */
    char *macro_name;
    char *macro_body;
    char **formals;
    file_pos_t defpos;
    int nformals;
    int body_len;
    macro_t *found;
    char * eol_comment;

    /* scan identifier to be defined */
    c = skip_white(c);

    if (!is_alpha(c)) {
        expected("identifier", c);
        return skip_to_end(c);
    }

    defpos = curpos->scan_pos;

    buf_init(buf);
    buf_init(&cbuf);
    c = scan_ident(buf, c);
    macro_name = buf_get_str(buf);

    /* scan formal parameter list, if any */
    if (c == '(') {
        c = grok_formals(&formals, &nformals, buf, next_char());
        if (c == ')') {
            c = next_char();
        } else {
            expected("')'", c);
        }
    } else {
        nformals = -1;
    }

    c = skip_white(c);

    /* scan macro definition */
    buf_init(buf);
    if (nformals == -1) {
        c = scan_to_end(buf, &cbuf, c);
    } else {

        while (!is_eol(c) && !is_eof(c)) {
            switch (classof(c)) {
              case MSTART:
                c = next_char();
                if (c == '#') {
                    c = next_char();
                    break;
                }
                c = skip_white(c);
                if (! is_alpha(c)) {
                    expected("macro formal parameter name", c);
                } else {
                    /* we've encountered "stringizing" operator */
                    /* Place the quoted parameter in buf */
                    add(buf, '"');
                    buf_init(&lbuf);
                    c = scan_ident(&lbuf, c);
                    grok_param(buf, &lbuf, formals, nformals, TRUE);
                    add(buf, '"');
                }
                break;
              case WHITE:
                add(buf, c);
                c = next_char();
                break;
              case DIGIT | XDIGIT:
                c = scan_number(buf, c);
                break;
              case ALPHA:
              case ALPHA | XDIGIT:
                buf_init(&lbuf);
                c = scan_ident(&lbuf, c);
                grok_param(buf, &lbuf, formals, nformals, 0);
                break;
              case PUNCT:
                switch (c) {
                  case '"':
                    add(buf, c);
                    c = scan_string(buf, next_char());
                    break;
                  case '\'':
                    add(buf, c);
                    c = scan_char_const(buf, next_char());
                    break;
                  case '/':
                    c = next_char();
                    if (c == '*') {
                        c = scan_c_comment(&cbuf, FALSE);
                    } else if (c == '/') {
                        c = scan_cpp_comment(&cbuf, FALSE);
                    } else {
                        add(buf,'/');
                    }
                    break;
                  case '\\':
                    c = next_char();
                    switch (c) {
                      case '\n':
                        incline(SYNC_RST_CSP);   /* value added */
                        c = next_char();
                        break;
                      case '\\':
                        add(buf, '\\');
                        add(buf, '\\');
                        c = next_char();
                        break;
                      default:
                        add(buf, '\\');
                        break;
                    }
                    break;
                  default:
                    add(buf, c);
                    c = next_char();
                    break;
                }
                break;
              default:
                UNHANDLED();
                break;
            }
        }

    }
    body_len    = buf_count(buf);
    macro_body  = buf_get_str(buf);
    eol_comment = buf_get_str(&cbuf);

    /* See if there's any configuration information about the macro */
    {
	char * replace =
	    configured_macro_replacement(pos_file(defpos),
					 macro_name,
					 macro_body, body_len,
					 nformals, formals,
					 eol_comment);
	if (replace) {
	    unget_char();
	    return push_string(replace);
	}

    }

    /* define the macro */
    found = macro_find(macro_name);
    if (!found || found->macro_definition != defpos) {

        macro_def(macro_name, macro_body, body_len,
		  nformals, formals, defpos, eol_comment);
	found = macro_find(macro_name);
	found->macro_evald = is_const_macro(found);
    }
    return c;
} /* grok_define() */


static int
grok_if(buf, c)
    buffer_t *buf;
    int c;
{
    char ident[256];
    char *name;
    cpp_eval_result_t result;

    assert(control_state.cur_scope >= 0);
    assert(control_state.gen_scope >= 0);

    buf_init(buf);
    c = scan_to_end(buf, want_comments(buf), c);

    name = local_copy(buf, ident, sizeof(ident));

    result = cpp_eval(name);

    ++control_state.cur_scope;
    control_state.skip_else = 0;

    if (EVAL_FAILED(result) || !IS_EVAL_INT(result)) {
        ;
    }
    else {
        if (EVAL_INT(result) != 0) {
            ++control_state.gen_scope;
        }
    }


    if (name != ident) {
        free(name);
    }

    return c;
}


static int
grok_elif(buf, c)
    buffer_t *buf;
    int c;
{
    char ident[256];
    char *name;
    cpp_eval_result_t result;

    assert(control_state.cur_scope >= 0);
    assert(control_state.gen_scope >= 0);

    if (control_state.cur_scope == 0) {
        error(FN,LN,"elif directive found without a matching if directive");
        return skip_to_end(c);
    }

    if (control_state.skip_else) {
        return skip_to_end(c);
    }

    if (scanning()) {
        control_state.skip_else = 1;
        control_state.gen_scope--;
        return skip_to_end(c);
    }

    if (control_state.gen_scope != control_state.cur_scope - 1) {
        return skip_to_end(c);
    }

    buf_init(buf);
    c = scan_to_end(buf, want_comments(buf), c);

    name = local_copy(buf, ident, sizeof(ident));

    result = cpp_eval(name);

/*
    ++control_state.cur_scope;
*/
/*
    control_state.skip_else = 0;
*/

    if (EVAL_FAILED(result) || !IS_EVAL_INT(result)) {
        ;
    }
    else {
        if (EVAL_INT(result) != 0) {
            ++control_state.gen_scope;
        }
    }


    if (name != ident) {
        free(name);
    }

    return c;
}


static int
grok_ifdef(buf, c, sense)
    buffer_t *buf;
    int c;
    int sense;
{
    char ident[128];
    char *name;
    macro_t *m;

    control_state.skip_else = 0;

    c = skip_white(c);

    if (! is_alpha(c)) {
        expected("identifier", c);
        return skip_to_end(c);
    }

    c = scan_ident(buf, c);
    name = local_copy(buf, ident, sizeof(ident));

    ++control_state.cur_scope;

    m = macro_find(name);
    if ((m != NULL && !sense) || (m == NULL && sense)) {
        ++control_state.gen_scope;
    }

    if (name != ident) {
        free(name);
    }

    return skip_to_end(c);
}

static int
grok_else(buf, c)
    buffer_t *buf;
    int c;
{
    int was_skipping = skipping();

    assert(control_state.cur_scope >= 0);
    assert(control_state.gen_scope >= 0);

    if (control_state.cur_scope == 0) {
        error(FN, LN, "Unmatched else directive");
        return skip_to_end(c);
    }
    else {
        if (! control_state.skip_else) {
            control_state.skip_else = 0;
            if (control_state.gen_scope == control_state.cur_scope) {
                --control_state.gen_scope;
            }
            else if (control_state.gen_scope == control_state.cur_scope -1 ) {
                ++control_state.gen_scope;
            }
        }
    }

    if (scanning() && was_skipping) {
        add_position_directive(buf, 0);
    }

    return skip_to_end(c);
}


static int
grok_endif(buf, c)
    buffer_t *buf;
    int c;
{
    int was_skipping = skipping();

    assert(control_state.cur_scope >= 0);
    assert(control_state.gen_scope >= 0);

    if (control_state.cur_scope == 0) {
        error(FN, LN, "Unmatched endif directive");
        return skip_to_end(c);
    }
    else {
        control_state.skip_else = 0;
        if (control_state.gen_scope == control_state.cur_scope) {
            --control_state.gen_scope;
        }
        --control_state.cur_scope;
    }

    if (scanning() && was_skipping) {
        add_position_directive(buf, 0);
    }

    return skip_to_end(c);
}

static void
pathname_head(buffer_t *buf, char * path)
    /* Copy the directory component(s) of <path>, if any,
     * to <buf>.
     */
{
    char *last = NULL;
    char *p;

    for (p = path; *p; p++) {
        if (*p == '/') {
            last = p+1;
        }
    }

    if (last != NULL) {
        for (p = path; p != last; p++) {
            add(buf, *p);
        }
    }
}



static int
search_for_file(buffer_t * buf, buffer_t * lbuf, char * name, boolean stdinc)
    /*
     * Attempt to open file with name <name>. If <stdinc> is true,
     * only search the "system" standard directories.
     * <buf> is the source code input buffer,
     * <lbuf> is a scratch buffer.
     * Returns the next character in the input stream, which
     * is normally the first character of the just-opened file.
     */
{

    cpp_file_t *f;


    if (name[0] == '/') {
        /* An absolute pathname. */
        f = attempt_open(name);

        if (f != NULL) {
            push_file(buf, f);
            return next_char();
        }

        goto failed;
    }

    if (!stdinc) {
        /* An ordinary include file */

        /* First search in the current working directory. */
        f = attempt_open(name);

        if (f != NULL) {
            push_file(buf, f);
            return next_char();
        }

        /* Next search relative to the directory containing
         * the currently open file.
         */
        buf_init(lbuf);
        pathname_head(lbuf, FN);

        if (buf_count(lbuf)) {
            buf_add_str(lbuf, name);

            f = attempt_open_buf(lbuf);

            if (f != NULL) {
                push_file(buf, f);
                return next_char();
            }
        }
    }

    /* Search in any include directories. */
    f = attempt_open_w_searchpaths( name, lbuf, search_paths, search_index );
    if (f) {
        push_file(buf,f);
        return next_char();
    }

    /* Search in the standard directories. */
    f = attempt_open_w_searchpaths( name, lbuf, system_search_paths,
                                                system_search_index);

    if (f != NULL) {
        push_file(buf,f);
        return next_char();
    }

 failed:
    error(FN,LN,"Couldn't open %s", name);
    return -1;  /* never reached: this just shuts up a warning */
}



static int
grok_include(buf, lbuf, c)
    buffer_t *buf, *lbuf;
    int c;
{
    char ident[64];
    char *name;
    int del;

    del = skip_white(c);

    switch (del) {
      case '<':
        del = '>';
        /* fall through */
      case '"':
        buf_init(lbuf);
        c = scan_to_del(lbuf, next_char(), del);
        if (c != del) {
            goto bad_input;
        }
        c = skip_to_end(c);
        unget_char();
        break;
      default:
      bad_input:
        bad_include();
        return skip_to_end(c);
    }

    name = local_copy(lbuf, ident, sizeof(ident));

    c = search_for_file(buf, lbuf, name, del == '>');

    if (name != ident) {
        free(name);
    }

    return c;
}


static int
grok_error(buf, c)
    buffer_t *buf;
    int c;
{
    char ident[128];
    char *msg;

    buf_init(buf);
    c = skip_white(c);

    c = scan_to_end(buf, want_comments(buf), c);
    msg = local_copy(buf, ident, sizeof(ident));

    error(FN,LN,msg);

    if (msg != ident) {
        free(msg);
    }

    return c;
}

static int
grok_undef(buf, c)
    buffer_t *buf;
    int c;
{
    char ident[128];
    char *name;

    c = skip_white(c);

    if (! is_alpha(c)) {
        expected("macro identifier", c);
        return skip_to_end(c);
    }

    buf_init(buf);
    c = scan_ident(buf, c);
    name = local_copy(buf, ident, sizeof(ident));

    macro_undef(name);

    if (name != ident) {
        free(name);
    }

    return skip_to_end(c);
}


/* from cpp_perf.c */
struct resword {char *name; int token;};
extern struct resword *cpp_keyword(char *str, int len);

static int
scan_directive(buf, c)
    buffer_t *buf;
    int c;
{
    buffer_t lbuf;
    char ident[32];
    int len;
    struct resword *rsvd;

    c = skip_white(c);

    if (is_eof(c) || is_eol(c)) {
        return c;
    }

    if (!is_alpha(c)) {
        bad_directive();
        return skip_to_end(c);
    }

    buf_init(&lbuf);

    c = scan_ident(&lbuf, c);
    len = buf_count(&lbuf);

    if (len > sizeof(ident)) {
        buf_destroy(&lbuf);
        bad_directive();
        return skip_to_end(c);
    }

    buf_move_to(&lbuf, ident);
    buf_destroy(&lbuf);

    if ((rsvd = cpp_keyword(ident, len)) == NULL) {
        bad_directive();
        return skip_to_end(c);
    }

    if (scanning()) {
        switch (rsvd->token) {
          case Define:          return grok_define(&lbuf, c);
          case Elif:            return grok_elif(&lbuf, c);
          case Else:            return grok_else(buf, c);
          case Endif:           return grok_endif(buf, c);
          case Error:           return grok_error(&lbuf, c);
          case If:              return grok_if(&lbuf, c);
          case Ifdef:           return grok_ifdef(&lbuf, c, 0);
          case Ifndef:          return grok_ifdef(&lbuf, c, 1);
          case Include:         return grok_include(buf, &lbuf, c);
          case Line:
          case Pragma:
          case Ident:           return skip_to_end(c);
          case Undef:           return grok_undef(&lbuf, c);
        }
    }
    else {
        switch (rsvd->token) {
          case Define:
          case Include:
          case Pragma:
          case Undef:
          case Error:
          case Ident:
          case Line:
            return skip_to_end(c);
          case Elif:
            return grok_elif(&lbuf,c);
          case Else:
            return grok_else(buf, c);
          case Endif:
            return grok_endif(buf, c);
          case If:
          case Ifdef:
          case Ifndef:
            ++control_state.cur_scope;
            return skip_to_end(c);
        }
    }

    return c;
}


static int
scan_actual(buf,c, level)
    buffer_t *buf;
    int c;
    int level;
    /* Read an argument to a macro invocation. */
{
    for (;;) {
        switch (classof(c)) {
          case END_INPUT:
            unexpected_eof("in macro call");
            return c;
          case END_OF_LINE:
            add(buf, ' ');
            c = next_char();
            break;
          case MSTART:
          case WHITE:
            add(buf, c);
            c = next_char();
            break;
          case DIGIT | XDIGIT:
            c = scan_number(buf, c);
            break;
          case ALPHA:
          case ALPHA | XDIGIT:
            c = scan_ident(buf, c);
            break;
          case PARAM_START:
            c = grok_actual_param(next_char());
            break;
          case PUNCT:
            switch (c) {
              case '(':
                add(buf,c);
                c = scan_actual(buf,next_char(),level+1);
                if (c != ')') {
                    expected("')'", c);
                }
                else {
                    add(buf, c);
                    c = next_char();
                }
                break;
              case ')':
                return c;
              case ',':
                if (level == 0) {
                    return c;
                }
                add(buf, c);
                c = next_char();
                break;
              case '"':
                add(buf, c);
                c = scan_string(buf, next_char());
                break;
              case '\'':
                add(buf, c);
                c = scan_char_const(buf, next_char());
                break;
              case '/':
                c = next_char();
                if (is_eof(c)) {
                    add(buf,'/');
                    return c;
                } else if (c=='*') {
                    /* replace comment by single whitespace */
                    while (c!='/') {
                        c = next_char();
                        while (!is_eof(c) && c!='*') {
                            c = next_char();
                        }
                        if (is_eof(c)) {
                            /* TBD: eof inside comment */
                            return c;
                        }
                        /* just saw '*' */
                        c = next_char();
                        if (is_eof(c)) {
                            /* TBD: eof inside comment */
                            return c;
                        }
                    }
                    add(buf,' ');
                    c = next_char();
                    break;
                } else {
                    add(buf,'/');
                    break;
                }
              case '\\':
                c = next_char();
                if (is_eof(c)) {
                    add(buf,'\\');
                    return c;
                }
                switch (c) {
                  case '\n':
                    incline(SYNC_RST_CSP);   /* value added */
                    c = next_char();
                    break;
                  default:
                    add(buf,'\\');
                    add(buf,c);
                    c = next_char();
                    break;
                }
                break;
              default:
                add(buf, c);
                c = next_char();
                break;
            }
            break;
          default:
            UNHANDLED();
            break;
        }
    }
}


static int
grok_actuals(actuals, nactuals, buf, c)
    char ***actuals;
    int *nactuals;
    buffer_t *buf;
    int c;
    /* Called from grok_macro_instance to scan the arguments
     * to a macro invocation.
     */
{
    char * a[256];
    char **ap;
    int    na = 0;
    int    i;

    for (;;) {
        c = skip_white(c);
        if (is_eof(c)) {
            unexpected_eof("in macro call");
            return c;
        }
        if (c == ')') break;
        buf_init(buf);
        c = scan_actual(buf,c,0);
        range_check(na, 256, "Too many actual parameters");
        a[na++] = buf_get_str(buf);
        c = skip_white(c);
        if (c != ',') break;
        c = next_char();
    }

    *nactuals = na;

    if (na == 0) {
        *actuals = NULL;
    }
    else {
        ap = (char**) malloc(sizeof(char*) * na);
        for (i = 0; i < na; i++) {
            ap[i] = a[i];
        }
        *actuals = ap;
    }

    return c;
}

static int
push_expansion(mac, actuals, nactuals)
    macro_t *mac;
    char **actuals;
    int nactuals;
    /* Make a macro and its arguments the current text source. */
{
    scan_position_t *npos;

    npos = NEW(scan_position_t);

    npos->scan_kind = scan_macro_expansion;
    npos->scan.expansion.expand_macro = mac;
    npos->scan.expansion.expand_actuals = actuals;
    npos->scan.expansion.expand_nactuals = nactuals;
    if (curpos != NULL) {
        npos->scan_pos = curpos->scan_pos;
    }
    npos->scan_index = 0;

    /* Push npos on to the stack of scanners. */
    npos->scan_next = curpos;
    curpos = npos;

    return next_char();
}

static void
push_string0(str)
    char *str;
{
    scan_position_t *npos;

    npos = NEW(scan_position_t);

    npos->scan_kind = scan_text;
    npos->scan.text = str;
    if (curpos) npos->scan_pos = curpos->scan_pos;
    npos->scan_index = 0;
    npos->scan_next = curpos;
    curpos = npos;
}

static int
push_string(char * str)
{
    push_string0(str);
    return next_char();
}

static void
grok_builtin_macro(buf, mac)
    buffer_t *buf;
    macro_t *mac;
{
    char buffer[32];

    switch (mac->macro_params) {
      case BUILTIN_FILE:
        add(buf, '"');
        buf_add_str(buf, FN);
        add(buf, '"');
        break;
      case BUILTIN_LINE:
        sprintf(buffer, "%d", LN);
        buf_add_str(buf, buffer);
        break;
      default:
        assert(0);
        break;
    }
}

static int
grok_macro_instance(buffer_t *buf, buffer_t* lbuf, int c, macro_t *mac)
    /* Called from grok_ident when a macro name has been
     * encountered. {c} is the following character.
     */
{
    char **actuals;
    int nactuals;

    if (mac->macro_params < -1) {

        grok_builtin_macro(buf, mac);
        return c;

    } else if (mac->macro_params != -1) {

        buf_init(lbuf);
        c = scan_white(lbuf, c);
        if (c != '(') {
            buf_add_str(buf, mac->macro_name);
            buf_concat(buf, lbuf);
            return c;
        }
        buf_init(lbuf);
        c = grok_actuals(&actuals, &nactuals, lbuf, next_char());
        if (c != ')') {
            expected("')'", c);
        }

    } else {
        /* a constant-like (0-param) macro */
        unget_char();

    }

    return push_expansion(mac, actuals, nactuals);
}

static int
parenthesized_ident(buf, c)
    buffer_t *buf;
    int c;
{
    for (;;) {
        c = skip_white(c);
        if (c == '(') {
            c = parenthesized_ident(buf, next_char());
            if (c != ')') {
                return 0;
            }
            c = next_char();
        }
        else if (is_alpha(c)) {
            do {
                add(buf, c);
                c = next_char();
            } while (is_alpha_numeric(c));
        }
        else {
            return c;
        }
    }
}

static int
grok_defined(buf, lbuf, c)
    buffer_t *buf, *lbuf;
    int c;
{
    char ident[128];
    char *name;

    c = skip_white(c);

    buf_init(lbuf);

    if (c == '(') {
        c = parenthesized_ident(lbuf, next_char());
        if (c != ')') {
            add(buf, BAD_INPUT);
            return c;
        }
        c = next_char();
    }
    else {
        c = scan_ident(lbuf, c);
    }

    name = local_copy(lbuf, ident, sizeof(ident));

    add(buf, ' ');

    if (macro_find(name)) {
        add(buf, '1');
    }
    else {
        add(buf, '0');
    }

    add(buf, ' ');

    if (name != ident) {
        free(name);
    }

    return c;
}

static boolean
is_const_macro(macro_t * mac)
    /* Test whether macro represents a constant value */
{

    if (!do_const_macros) return FALSE;
    if (mac->macro_params != -1) return FALSE;
    if (!mac->macro_eval_tried) {
        /* try to evaluate the macro as a constant */
        /* TBD: see gen_mconst for more general evaluation */
        mac->macro_eval_tried = TRUE;
	if (mac->macro_body) {
	    mac->const_value = cpp_eval(mac->macro_body);
	    mac->macro_evald = !EVAL_FAILED(mac->const_value);
	} else {
	    mac->const_value.eval_result_kind = eval_failed;
	    mac->macro_evald = FALSE;
	}

    }
    return mac->macro_evald;
}

static int
grok_ident( buffer_t * buf, int c )
    /* Called when the alphanumeric {c} was encountered when an
     * identifier is expected.
     * Scan the identifier; copy to {buf} or expand as macro, as
     * appropriate.
     */
{
    buffer_t lbuf;
    char     ident[128];
    char *   name;
    macro_t *mac;

    /* Scan the identifier. */
    buf_init(&lbuf);
    c = scan_ident(&lbuf, c);
    name = local_copy(&lbuf, ident, sizeof(ident));

    /* Do the right thing with the identifier. */
    if (parsing() && !strcmp(name,"defined")) {

        return grok_defined(buf, &lbuf, c);

    } else if ((mac = macro_find(name)) && !is_const_macro(mac)) {

        c = grok_macro_instance(buf, &lbuf, c, mac);

    } else {

        buf_add_str(buf, name);
    }

    if (name != ident) {
        free(name);
    }

    return c;
}

static int
grok_actual_param(int param_ord)
    /* Replace a parameter marker by the actual argument when
     * expanding a macro.  This is accomplished by pushing the
     * actual argument on the source stack.
     */
{
    char **actuals;
    char *param;
    int nactuals;

    assert(curpos->scan_kind == scan_macro_expansion);

    actuals = curpos->scan.expansion.expand_actuals;
    nactuals = curpos->scan.expansion.expand_nactuals;

    if (actuals == NULL || param_ord > nactuals) {
        return next_char();
    }

    param = actuals[param_ord - 1];

    if (param == NULL) {
        return next_char();
    }

    return push_string(param);
}


static int
skip(buf, c)
    buffer_t *buf;
    int c;
{
    buffer_t lbuf;

    assert(!parsing());

    for (;;) {
        if (scanning()) {
            return c;
        }

        switch (classof(c)) {
          case END_INPUT:
            return c;
          case DIGIT | XDIGIT:
          case ALPHA:
          case ALPHA | XDIGIT:
          case WHITE:
            c = next_char();
            break;
          case END_OF_LINE:
            incline(SYNC_RST_CSP);
            c = next_char();
            break;
          case PARAM_START:
            c = next_char();
            c = next_char();
            break;
          case MSTART:
            c = scan_directive(buf, next_char());
            break;
          case PUNCT:
            switch (c) {
              case '"':
                buf_init(&lbuf);
                c = scan_string(&lbuf, next_char());
                buf_destroy(&lbuf);
                break;
              case '\'':
                buf_init(&lbuf);
                c = scan_char_const(&lbuf, next_char());
                buf_destroy(&lbuf);
                break;
              case '/':
                c = next_char();
                if (c == '*') {
                    c = scan_c_comment(NULL,FALSE);
                } else if (c == '/')
                    c = scan_cpp_comment(NULL,FALSE);
                break;
              case '\\':
                UNHANDLED();
                break;
              default:
                c = next_char();
                break;
            }
            break;
          default:
            UNHANDLED();
            break;
        }
    }
}


static int
scan(buf)
    buffer_t *buf;
{
    int c;

    c = next_char();

    for (;;) {
        if (skipping()) {
            c = skip(buf, c);
        }

        assert(scanning());

        switch (classof(c)) {
          case END_INPUT:
            return -1;
          case WHITE:
            add(buf, c);
            return 0;
          case END_OF_LINE:
            incline(SYNC_INC_CSP);
            check_position(buf);
            add(buf, c);
            return 0;
          case PARAM_START:
            c = grok_actual_param(next_char());
            break;
          case DIGIT | XDIGIT:
            c = scan_number(buf, c);
            break;
          case ALPHA:
          case ALPHA | XDIGIT:
            c = grok_ident(buf, c);
            break;
          case MSTART:
            if (parsing()) {
                add(buf, BAD_INPUT);
                return 0;
            }
            c = scan_directive(buf, next_char());
            break;
          case PUNCT:
            switch (c) {
              case '"':
                add(buf, c);
                c = scan_string(buf, next_char());
                break;
              case '\'':
                add(buf, c);
                c = scan_char_const(buf, next_char());
                break;
              case '/':
                c = next_char();
                if (c == '*') {
                    comment_start(buf);
                    c = scan_c_comment(want_comments(buf), TRUE);
                } else if (c == '/') {
                    cpp_comment_start(buf);
                    c = scan_cpp_comment(want_comments(buf), TRUE);
                } else {
                    add(buf,'/');
                }
                break;
              case '\\':
                UNHANDLED();
                break;
              default:
                add(buf, c);
                return 0;
            }
            break;
          default:
            UNHANDLED();
            break;
        }
    }
}


int
cpp_getc_from(buf)
    buffer_t *buf;
{
    while (buf_empty(buf)) {
        if (scan(buf) == -1) break;
    }
    return buf_get(buf);
}

int
cpp_getc()
{
    return cpp_getc_from(&cppbuf);
}


void
cpp_init_contents( char * str )
{
    buf_init(&cppbuf);

    control_state.skip_else = 0;
    control_state.cur_scope = 0;
    control_state.gen_scope = 0;
    control_state._parsing = 0;

    push_string0(str);

    /* TBD: maybe curpos should be initialized with the
     * position of the macro definition.
     */
    curpos->scan_pos = 0;
}

