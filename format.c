#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

#include "boolean.h"
#include "errors.h"
#include "format.h"

extern FILE *cur_unit_fd;

#undef NULL
#define NULL	0

static int indentation = START_INDENT;
static int spec_indentation = START_INDENT;
static int body_indentation = START_INDENT;

/*
 * This flag is true if we're currently putting a string or char literal.
 * It's used to decide how to handle line breaks.
 */
static boolean in_text_literal = FALSE;

/* This is a stronger form of literal quotation: unlike strings,
 * we'll NEVER want to split in one of these literals (floating
 * point literals are the motivating case.)
 */
static boolean in_literal = FALSE;

/*
 * This flag is set if we're outputting a comment.
 */
static boolean in_comment = FALSE;

/*
 * The maximum allowable width of an output line.  GNAT, for one, has
 * a hard limit on this, aside from legibility concerns.
 */
static const int max_line_width = 100;

static int line_num = 0;
static int spec_line_num = 0;
static int body_line_num = 0;

void format_to_spec()
{
      body_indentation = indentation;
      body_line_num = line_num;
      indentation = spec_indentation;
      line_num = spec_line_num;
}

void format_to_body()
{
      spec_indentation = indentation;
      spec_line_num = line_num;
      indentation = body_indentation;
      line_num = body_line_num;
}

int
output_line()
{
	return line_num;
}

void
reset_output_line()
{
	spec_line_num = body_line_num = line_num = 1;
}

static boolean
allow_break_after(char c)
{
    if (isalnum(c)) return FALSE;
    switch (c) {
    case '_':    /* constituent of identifiers */
    case ':':    /*  :=         */
    case '=':    /*  =>         */
    case '.':    /*  ..         */
    case '/':    /*  /=         */
    case '>':    /*  >=, >>     */
    case '<':    /*  <=, <<, <> */
    case '*':    /*  **         */
	return FALSE;
    default:
	return TRUE;
    }
}


static void
put(char c)
{
    fputc(c, cur_unit_fd);

    if (c == '\n') {
	line_num++;
	indentation = START_INDENT;
	in_comment  = FALSE;
    } else {
	indentation++;
	if (indentation > max_line_width) {
	    /* For now, we're not strictly enforcing line length.
	     * We may have to break long text literals if this
	     * simpler scheme doesn't work
	     */
	    if ( !(in_text_literal || in_literal) && allow_break_after(c) ) {
		if (in_comment) {
		    put('\n');
		    putf("%--");
		} else {
		    put('\n');
		}
	    }

	}
    }
}

void
reset_indent()
{
    spec_indentation = body_indentation = indentation = START_INDENT;
}

void
put_char(int c)
{
    put(c);
}

void
new_line()
{
    put('\n');
}

int
cur_indent()
{
    return indentation;
}

void
indent_to(int n)
{
    if (n<indentation) {
	put('\n');
    }
    while(indentation < n) {
	put(' ');
    }
}

void
put_string(s)
	char *s;
{
	if (s == NULL) s = " <NULL> ";
	while (*s) {
		put(*s++);
	}
}

void
putf(char *s, ...)
{
    va_list ap;
    char    c;
    char *  sarg;
    int     iarg;

    va_start(ap, s);

    while ( (c = *s++) ) {
	if (c=='%') {
	    c = *s++;
	    switch (c) {
	    case 's':
		sarg = va_arg(ap,char*);
		if (sarg && *sarg) {
		    put_string(sarg);
		}
		break;
	    case '>':
		iarg = va_arg(ap,int);
		indent_to(iarg);
		break;
	    case '%':
		put('%');
		break;
	    case '{':   /* start of string or char literal */
		in_text_literal = TRUE;
		break;
	    case '}':   /* end of string or char literal */
		in_text_literal = FALSE;
		break;
	    case '-':   /* start of comment; the format string */
		        /* will look like ...%--... */
		if (indentation>max_line_width) put('\n');
		in_comment = TRUE;
		put('-');
		break;
	    case '[':  /* start of unbreakable literal */
		in_literal = TRUE;
		break;
	    case ']':  /* end of unbreakable literal */
		in_literal = FALSE;
		break;
	    case 0:
	    default:
		fatal(__FILE__,__LINE__,"bad format in putf()");
	    }
	} else {
	    put(c);
	}
    }
    va_end(ap);
}

