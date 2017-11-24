#include <assert.h>
#include <stdio.h>
#include <memory.h>
#include <sys/types.h>

#include "errors.h"
#include "host.h"
#include "files.h"
#include "hash.h"
#include "buffer.h"
#include "cpp.h"
#include "cpp_hide.h"
#include "allocate.h"
#include "il.h"
#include "nodeop.h"
#include "types.h"
#include "stab.h"
#include "stmt.h"
#include "units.h"
#include "y.tab.h"
#include "comment.h"

#undef NULL
#define NULL		0

#define FN		file_name(yypos)
#define LN		line_number(yypos)

#define TRACE()		printf("\n%s:%d",__FILE__,__LINE__)


/* This is an otherwise unused character that's been hijacked
 * to represent the start of a macro body in a special call
 * to the parser.
 */
#define MARKER_CHAR '\033'

file_pos_t yypos;
static int yyc;
static int skipping_compound_statements;
static boolean at_line_start = TRUE;
static node_t *last_ident;

/* When this flag is TRUE, a typedef-name token will be returned
 * as an IDENTIFIER; otherwise as a TYPEDEF_NAME
 */
static boolean typedef_name_as_id;

void
yield_typedef(boolean flag)
{
    typedef_name_as_id = !flag;
}

void
yyerror(msg)
    char *msg;
{
    error(FN,LN,msg);
}

void
td(void)
{
    yield_typedef(TRUE);
}

void
yylex_init()
{
    yyc = cpp_getc();
}

static void
end_line()
{
    yypos++;
    last_ident = NULL;
    at_line_start = TRUE;
}

static int
magnitude(c, val)
    int c;
    host_int_t *val;
{
    host_int_t m;
    int sign;

    if (! is_magnitude(c)) {
	*val = 1;
	return c;
    }

    c = cpp_getc();
    sign = 1;

    if (c == '-') {
	sign = -1;
	c = cpp_getc();
    }
    else if (c == '+') {
	c = cpp_getc();
    }

    for (m = 0; is_digit(c); ) {
	m = m * 10 + (c - '0');
	c = cpp_getc();
    }

    *val = m * sign;
    return c;
}

static int
grok_number(boolean fraction)
    /* fraction is TRUE if the character previous to yyc was '.' */
{
    host_int_t val;
    int base;

    if (fraction) {
	val = 0;
	base = 10;
    } else {
	/* scan integer part of number */
	if (yyc == '0') {
	    val = 0;
	    yyc = cpp_getc();
	    if (yyc == 'x' || yyc == 'X') {
		base = 16;
		for (;;) {
		    yyc = cpp_getc();
		    if (is_digit(yyc)) {
			val = (val<<4) + (yyc - '0');
		    }
		    else if (yyc >= 'A' && yyc <= 'F') {
			val = (val<<4) + 10 + (yyc - 'A');
		    }
		    else if (yyc >= 'a' && yyc <= 'f') {
			val = (val<<4) + 10 + (yyc - 'a');
		    }
		    else {
			break;
		    }
		}
	    } else {
		base = is_octal_digit(yyc)? 8: 10;
		while (is_octal_digit(yyc)) {
		    val = (val<<3) + (yyc - '0');
		    yyc = cpp_getc();
		}
	    }
	} else {
	    val = yyc - '0';
	    base = 10;
	    for (;;) {
		yyc = cpp_getc();
		if (! is_digit(yyc)) break;
		val = val * 10 + (yyc - '0');
	    }
	}
	if (int_modifier(yyc)) {
	    yyc = cpp_getc();
	}
    }


    if (fraction || yyc == '.') {
	host_float_t dval, d;

	dval = (host_float_t) val;
	d = 0.1;

	/* If fraction, yyc is already the first following digit */
	if (!fraction) yyc = cpp_getc();

	while (is_digit(yyc)) {
	    int tmp;
	    tmp = yyc - '0';
	    dval += (d * (host_float_t)tmp);
	    d    *=  0.1;
	    yyc = cpp_getc();
	}


	/* handle magnitude (i.e, exponent) */
	if (is_magnitude(yyc)) {
	    host_int_t vm;
	    host_float_t m;

	    yyc = magnitude(yyc, &vm);

	    if (vm > 0) {
		for (m = 1.0; vm; vm--) {
		    m *= 10.0;
		}
	    }
	    else {
		for (m = 1.0; vm; vm++) {
		    m /= 10.0;
		}
	    }

	    dval *= m;
	}

	if (float_modifier(yyc)) {
	    yyc = cpp_getc();
	}

	yylval.nod = new_node(_FP_Number, dval);
	return FLOATING_CONSTANT;
    }

    while (int_modifier(yyc)) {
	yyc = cpp_getc();
    }

    yylval.nod = new_node(_Int_Number, val, base);
    return INTEGER_CONSTANT;
}


struct resword {char *name; short token;};
extern struct resword *in_word_set(char*, int);

static int
grok_ident(void)
{

    char id[1024], *p;
    struct resword *r;
    symbol_t *sym;



    for (p = id; is_alpha_numeric(yyc); yyc = cpp_getc()) {
	*p++ = yyc;
    }
    *p = 0;

    r = in_word_set(id, (int)(p-id));
    if (r != NULL) {
	return r->token;
    }


    if (!macro_find(id) && (sym = find_sym(id))) {
	if (!typedef_name_as_id && is_typedef(sym)) {
	    yylval.typ = copy_type(sym->sym_type);
	    yylval.typ->type_base = sym;
	    yylval.typ->_typedef = 0;
	    return TYPEDEF_NAME;
	}
    }

    yylval.nod = new_node(_Ident, new_string(id));
    last_ident = yylval.nod;
    return IDENTIFIER;
}

static int
escaped_char(int c)
    /*
     * Interpret an escape sequence.  {c} is the first character
     * following the backslash, and the global {yyc} holds the
     * first character after that.  escaped_char will consume
     * more characters (with calls to cpp_getc) if {c} is '0' or 'x',
     * indicating the start of a numeric sequence.
     * On exit, yyc holds the first character following the
     * escape sequence.
     */
{
    int val, i;

    switch (c) {
      case 'n':	    return '\n';
      case 't':	    return '\t';
      case 'v':	    return '\v';
      case 'b':	    return '\b';
      case 'r':	    return '\r';
      case 'f':	    return '\f';
      case 'a':	    return '\a';
      case '?':	    return '\?';
      case '\'':    return '\'';
      case '\"':    return '\"';
      case '\\':    return '\\';

      case 0:
	for (i = 0, val = 0; is_octal_digit(yyc) && i < 3; i++, yyc = cpp_getc()) {
	    val = val * 8 + (yyc - '0');
	}
	return val;

      case 'x':
	for (i = 0, val = 0; is_hex_digit(yyc) && i < 2; i++, yyc = cpp_getc()) {
	    val *= 16;
	    if (is_digit(yyc)) {
		val += (yyc - '0');
	    }
	    else if (yyc <= 'F') {
		val += (yyc - ('A' - 10));
	    }
	    else {
		val += (yyc - ('a' - 10));
	    }
	}
	return val;

      default:
	break;
    }

    return c;
}

static int
scan_char_const()
{
    host_int_t cval = 0;
    int c;

    for (;;) {
	if (is_eof(yyc)) {
	    error(FN,LN,"End of file while scanning char constant");
	    goto end_char_const;
	}
	if (is_eol(yyc)) {
	    error(FN,LN,"End of line while scanning char constant");
	    goto end_char_const;
	}
	switch (yyc) {
	  case '\'':
	    yyc = cpp_getc();
	    goto end_char_const;
	  case '\\':
	    yyc = cpp_getc();
	    switch (yyc) {
	      case '\n':
		yypos++;
		yyc = cpp_getc();
		break;
	      case '0':
	      case 'x':
	      case 'n':
	      case 't':
	      case 'v':
	      case 'b':
	      case 'r':
	      case 'f':
	      case 'a':
	      case '?':
	      case '\\':
	      case '\'':
	      case '\"':
		c = yyc;
		yyc = cpp_getc();
		cval <<= 8;
		cval |= escaped_char(c);
		break;
	      default:
		cval <<= 8;
		cval |= yyc;
		yyc = cpp_getc();
		break;
	    }
	    break;
	  default:
	    cval <<= 8;
	    cval |= yyc;
	    yyc = cpp_getc();
	    break;
	}
    }

  end_char_const:
    yylval.nod = new_node(_Int_Number, cval, 10);
    yylval.nod->char_lit = TRUE;
    return CHARACTER_CONSTANT;
}

static int
scan_string()
{
    buffer_t buf;
    int c, len;
    char *s;

    buf_init(&buf);

    for (;;) {
	if (is_eof(yyc)) {
	    error(FN,LN,"End of file while scanning string constant");
	    goto end_of_string;
	}
	if (is_eol(yyc)) {
	    error(FN,LN,"End of line while scanning string constant");
	    goto end_of_string;
	}
	switch (yyc) {
	  case '"':
	    yyc = cpp_getc();
	    goto end_of_string;
	  case '\\':
	    yyc = cpp_getc();
	    switch (yyc) {
	      case '\n':
		yypos++;
		yyc = cpp_getc();
		break;
	      case '0':
	      case 'x':
	      case 'n':
	      case 't':
	      case 'v':
	      case 'b':
	      case 'r':
	      case 'f':
	      case 'a':
	      case '?':
	      case '\\':
	      case '\'':
	      case '\"':
		c = yyc;
		yyc = cpp_getc();
		buf_add(&buf, escaped_char(c));
		break;
	      default:
		buf_add(&buf, yyc);
		yyc = cpp_getc();
		break;
	    }
	    break;
	  default:
	    buf_add(&buf, yyc);
	    yyc = cpp_getc();
	    break;
	}
    }

  end_of_string:
    len = buf_count(&buf);
    s = buf_get_str(&buf);

    yylval.nod = new_node(_String, s, len);
    set_cur_unit_has_const_string();
    return STRING;
}

static int
skip_to_end(c)
    int c;
{
    for (;;) {
	if (is_eof(c) || is_eol(c)) break;
	c = cpp_getc();
    }
    return c;
}

int
skip_white(c)
    int c;
{
    while (is_white(c)) {
	c = cpp_getc();
    }
    return c;
}


static int
skip_c_comment(c)
    int c;
{
    int tmp;

    for (;;) {
	switch (classof(c)) {
	  case END_INPUT:
	    return 0;
	  case END_OF_LINE:
	    yypos++;
	    c = cpp_getc();
	    break;
	  case DIGIT | XDIGIT:
	  case ALPHA:
	  case ALPHA | XDIGIT:
	  case MSTART:
	  case WHITE:
	    c = cpp_getc();
	    break;
	  case PUNCT:
	    tmp = c;
	    c = cpp_getc();
	    switch (tmp)  {
	      case '*':
		if (c == '/') {
		    return cpp_getc();
		}
		break;
	    }
	    break;
	  default:
	    assert(0);
	    break;
	}
    }
}

static int
skip_cpp_comment(c)
    int c;
{

    for (;;) {
	switch (classof(c)) {
	  case END_INPUT:
	    return 0;
	  case END_OF_LINE:
	    yypos++;
	    return cpp_getc();
	  case DIGIT | XDIGIT:
	  case ALPHA:
	  case ALPHA | XDIGIT:
	  case MSTART:
	  case WHITE:
	  case PUNCT:
	    c = cpp_getc();
	    break;
	  default:
	    assert(0);
	    break;
	}
    }
}

/******************** saving comments **************/

static struct comment_block * comment_accum;

static void
save_comment_line( char * str )
{
    if (!comment_accum) comment_accum = new_comment_block();
    add_comment_line( comment_accum, str );
}

struct comment_block *
fetch_comment_block()
{
    struct comment_block * tmp;
    tmp = comment_accum;
    comment_accum = 0;
    return tmp;
}


/**************************************************/

static int
save_c_comment(c)
    int c;
{
    int tmp, result;
    buffer_t buf;

    buf_init(&buf);

    for (;;) {
	switch (classof(c)) {
	  case END_INPUT:
	    result = 0;
	    goto end_of_subp;
	  case END_OF_LINE:
	    buf_add(&buf, c);
	    yypos++;
	    c = cpp_getc();
	    break;
	  case DIGIT | XDIGIT:
	  case ALPHA:
	  case ALPHA | XDIGIT:
	  case MSTART:
	  case WHITE:
	    buf_add(&buf, c);
	    c = cpp_getc();
	    break;
	  case PUNCT:
	    tmp = c;
	    c = cpp_getc();
	    switch (tmp)  {
	      case '*':
		if (c == '/') {
		    result = cpp_getc();
		    goto end_of_subp;
		}
		break;
	    }
	    buf_add(&buf, tmp);
	    break;
	  default:
	    assert(0);
	    break;
	}
    }

  end_of_subp:
    if (buf_count(&buf) > 0) {
	if (at_line_start) {
	    save_comment_line( buf_get_str(&buf) );
	} else {
	    assert(last_ident->node_kind == _Ident);
	    last_ident->node.id.cmnt = buf_get_str(&buf);
	}
    }

    last_ident = NULL;
    return result;
}

int
save_cpp_comment(c)
    int c;
{
    int result;
    buffer_t buf;

    buf_init(&buf);

    for (;;) {
	switch (classof(c)) {
	  case END_INPUT:
	    result = 0;
	    goto end_of_subp;
	  case END_OF_LINE:
	    buf_add(&buf, c);
	    yypos++;
	    result = cpp_getc();
	    goto end_of_subp;
	  case DIGIT | XDIGIT:
	  case ALPHA:
	  case ALPHA | XDIGIT:
	  case MSTART:
	  case WHITE:
	  case PUNCT:
	    buf_add(&buf, c);
	    c = cpp_getc();
	    break;
	  default:
	    assert(0);
	    break;
	}
    }

  end_of_subp:
    if (buf_count(&buf) > 0) {
	assert(last_ident->node_kind == _Ident);
	last_ident->node.id.cmnt = buf_get_str(&buf);
    }

    last_ident = NULL;
    return result;
}

static int
scan_c_comment(c)
    int c;
{
    if (last_ident || at_line_start) {
	return save_c_comment(c);
    }
    return skip_c_comment(c);
}

static int
scan_cpp_comment(c)
    int c;
{
    if (last_ident) {
	return save_cpp_comment(c);
    }
    return skip_cpp_comment(c);
}

static void
grok_directive(void)
    /*
     * The only directive expected at this point (post-preprocessing)
     * is of the form
     *	 # <linenumber> <filename> <nesting level>
     * where <nesting level> is an extension to the standard C directive,
     * showing how many include levels deep this file is.
     */
{
    char fname[256];
    int i, line, nest;
    file_pos_t old_pos;

    /* scan the line number */
    yyc = skip_white(cpp_getc());
    if (! is_digit(yyc)) {
	yyc = skip_to_end(yyc);
	return;
    }

    for (line = 0; is_digit(yyc); yyc = cpp_getc()) {
	line = line * 10 + (yyc - '0');
    }

    /* scan the file name */
    yyc = skip_white(yyc);
    if (yyc != '"') {
	yyc = skip_to_end(yyc);
	return;
    }
    for (i = 0; ; i++) {
	yyc = cpp_getc();
	if (yyc == '"' || is_eof(yyc) || is_eol(yyc)) break;
	fname[i] = yyc;
    }
    fname[i] = 0;
    if (i == 0) {
	yyc = skip_to_end(yyc);
	return;
    }

    /* change the file position */
    old_pos = yypos;
    yypos = set_file_pos(fname, line);

    if (at_file_start && pos_file(old_pos)!=pos_file(yypos)) {
	/* Grab initial comments in current file before
	 * opening up next one.
	 */
	if (!cur_unit_header_comment_set()) {
	set_cur_unit_header_comment( fetch_comment_block() );
	}
    }

    init_unit(yypos);

    if (yyc != '"') {
	yyc = skip_to_end(yyc);
	return;
    }

    /* scan nesting level number */
    yyc = skip_white(cpp_getc());
    if (! is_digit(yyc)) {
	yyc = skip_to_end(yyc);
	return;
    }

    for (nest = 0; is_digit(yyc); yyc = cpp_getc()) {
	nest = nest * 10 + (yyc - '0');
    }

    unit_included(yypos, nest);

    yyc = skip_to_end(yyc);
    if (is_eol(yyc)) {
	/* DON'T increment yypos */
	yyc = cpp_getc();
    }
}

static int
skip()
{
    int token;
    int yylex();

    token = yylex();

    for (;;) {
	switch (token) {
	  case '}':
	  case 0:
	    goto done;
	  case '{':
	    token = skip();
	    if (token == '}') {
		token = yylex();
	    }
	    break;
	  default:
	    token = yylex();
	    break;
	}
    }

  done:
    return token;
}

static int next_token(void);  /* forward ref */

int
yylex(void)
{
    int result = next_token();
    if (at_file_start) {
	set_cur_unit_header_comment( fetch_comment_block() );
    }
    at_file_start = FALSE;
    return result;
}

static int
next_token(void)
{
    int token;

    if (skipping_compound_statements) {
	skipping_compound_statements = 0;
	token = skip();
	return token;
    }

    for (;;) {
	switch (classof(yyc)) {
	  case END_INPUT:
	    return 0;
	  case WHITE:
	    do {
		yyc = cpp_getc();
	    } while (is_white(yyc));
	    break;
	  case END_OF_LINE:
	    end_line();
	    yyc = cpp_getc();
	    break;
	  case DIGIT | XDIGIT:
	    at_line_start = FALSE;
	    return grok_number(FALSE);
	  case ALPHA:
	  case ALPHA | XDIGIT:
	    at_line_start = FALSE;
	    return grok_ident();
	  case MSTART:
	    grok_directive();
	    break;
	  case PUNCT:
	    token = yyc;
	    yyc = cpp_getc();
	    switch (token)  {
	      case '.':
		if (yyc == '.') {
		    yyc = cpp_getc();
		    if (yyc == '.') {
			yyc = cpp_getc();
			at_line_start = FALSE;
			return ELLIPSIS;
		    }
		    at_line_start = FALSE;
		    return DOTDOT;
		} else if (is_digit(yyc)) {
		    /* This is a floating point number starting with '.' */
		    return grok_number(TRUE);
		}
		break;
	      case '\"':
		at_line_start = FALSE;
		return scan_string();
	      case '\'':
		at_line_start = FALSE;
		return scan_char_const();
	      case '&':
		switch (yyc) {
		  case '&':
		    yyc = cpp_getc();
		    at_line_start = FALSE;
		    return AND_OP;
		  case '=':
		    yyc = cpp_getc();
		    at_line_start = FALSE;
		    return AND_ASSIGN;
		}
		break;
	      case '^':
		if (yyc == '=') {
		    yyc = cpp_getc();
		    at_line_start = FALSE;
		    return XOR_ASSIGN;
		}
		break;
	      case '|':
		switch (yyc) {
		  case '|':
		    yyc = cpp_getc();
		    at_line_start = FALSE;
		    return OR_OP;
		  case '=':
		    yyc = cpp_getc();
		    at_line_start = FALSE;
		    return OR_ASSIGN;
		}
		break;
	      case '*':
		if (yyc == '=') {
		    yyc = cpp_getc();
		    at_line_start = FALSE;
		    return MUL_ASSIGN;
		}
		break;
	      case '/':
		switch (yyc) {
		  case '=':
		    yyc = cpp_getc();
		    at_line_start = FALSE;
		    return DIV_ASSIGN;
		  case '*':
		    yyc = scan_c_comment(cpp_getc());
		    continue;
		  case '/':
		    yyc = scan_cpp_comment(cpp_getc());
		    continue;
		}
		break;
	      case '%':
		if (yyc == '=') {
		    yyc = cpp_getc();
		    at_line_start = FALSE;
		    return MOD_ASSIGN;
		}
		break;
	      case '<':
		switch(yyc) {
		  case '<':
		    yyc = cpp_getc();
		    if (yyc == '=') {
			yyc = cpp_getc();
			at_line_start = FALSE;
			return LEFT_ASSIGN;
		    }
		    at_line_start = FALSE;
		    return LEFT_OP;
		  case '=':
		    yyc = cpp_getc();
		    at_line_start = FALSE;
		    return LE_OP;
		}
		break;
	      case '>':
		switch(yyc) {
		  case '>':
		    yyc = cpp_getc();
		    if (yyc == '=') {
			yyc = cpp_getc();
			at_line_start = FALSE;
			return RIGHT_ASSIGN;
		    }
		    at_line_start = FALSE;
		    return RIGHT_OP;
		  case '=':
		    yyc = cpp_getc();
		    at_line_start = FALSE;
		    return GE_OP;
		}
		break;
	      case '=':
		if (yyc == '=') {
		    yyc = cpp_getc();
		    at_line_start = FALSE;
		    return EQ_OP;
		}
		break;
	      case '!':
		if (yyc == '=') {
		    yyc = cpp_getc();
		    at_line_start = FALSE;
		    return NE_OP;
		}
		break;
	      case '+':
		switch (yyc) {
		  case '+':
		    yyc = cpp_getc();
		    at_line_start = FALSE;
		    return INC_OP;
		  case '=':
		    yyc = cpp_getc();
		    at_line_start = FALSE;
		    return ADD_ASSIGN;
		}
		break;
	      case '-':
		switch(yyc) {
		  case '-':
		    yyc = cpp_getc();
		    at_line_start = FALSE;
		    return DEC_OP;
		  case '=':
		    yyc = cpp_getc();
		    at_line_start = FALSE;
		    return SUB_ASSIGN;
		  case '>':
		    yyc = cpp_getc();
		    at_line_start = FALSE;
		    return PTR_OP;
		}
		break;
		case MARKER_CHAR:
		at_line_start = FALSE;
		return MARKER;
	    }
	    at_line_start = FALSE;
	    return token;
	  default:
	    assert(0);
	    break;
	}
    }
}

void
yyskip()
{
    skipping_compound_statements = 1;
}


void cpp_init_contents( char * str );

void
scan_string_init(char * str)
{
    char * newstr = allocate(strlen(str)+2);
    sprintf(newstr, "%s%c", str, MARKER_CHAR);
    cpp_init_contents(newstr);
    yyc = MARKER_CHAR;
}
