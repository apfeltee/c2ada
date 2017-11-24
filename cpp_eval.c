#include <assert.h>
#include <sys/types.h>
#include <ctype.h>
#include <setjmp.h>

#include "errors.h"
#include "host.h"
#include "files.h"
#include "hash.h"
#include "buffer.h"
#include "cpp.h"
#include "cpp_hide.h"
#include "cpp_eval.h"
#include "allocate.h"

#undef NULL
#define NULL	0

#undef getc
#define getc(s)		    cpp_getc_from(&s->evalbuf)

/* #define MAKE_FAIL(x)	    (x).eval_result_kind = eval_failed */
#define MAKE_FAIL(x) make_fail(&x)
static void
make_fail(cpp_eval_result_t *m)
{
    m->eval_result_kind = eval_failed;
}
#define MAKE_INT(x,v,b)	    {(x).eval_result_kind = eval_int;	\
		 (x).eval_result.ival = (v);	    \
		 (x).base = (b);}
#define MAKE_FLOAT(x,v)	    {(x).eval_result_kind = eval_float; (x).eval_result.fval = (v);}
#define MAKE_STRING(x,v)    {(x).eval_result_kind = eval_string; (x).eval_result.sval = (v);}


typedef cpp_eval_result_t tokval_t;


typedef enum  {
    tok_error,
    tok_eof,
    tok_discrete,
    tok_float,
    tok_string,
    tok_eq,
    tok_neq,
    tok_gt,
    tok_geq,
    tok_lt,
    tok_leq,
    tok_shift_left,
    tok_shift_right,
    tok_or,
    tok_and,
    tok_type
} tokclass_t;

typedef struct {
    cpp_eval_result_t  result;
    buffer_t           evalbuf;
    int                c;
    int                recover;
    jmp_buf	       exception;

    tokval_t           curval;
    tokval_t           nextval;
    tokclass_t         curtok;
    tokclass_t         nexttok;
} cpp_eval_state_t, *cpp_eval_state_pt;



static int
promote(l,r)
    tokval_t *l, *r;
{
    switch(l->eval_result_kind)
    {
      case eval_int:

	switch(r->eval_result_kind)
        {
	  case eval_int:
	    return eval_int;
	  case eval_float:
	    l->eval_result_kind = eval_float;
	    l->eval_result.fval = (host_float_t) l->eval_result.ival;
	    return eval_float;
	  default:
            break;
	}
        break;

      case eval_float:

	switch(r->eval_result_kind)
        {
	  case eval_int:
	    r->eval_result_kind = eval_float;
	    r->eval_result.fval = (host_float_t) r->eval_result.ival;
	    /* fall through */
	  case eval_float:
	    return eval_float;
	  default:
            break;
	}
        break;

      default:
        break;
    }

    return eval_failed;
}

static tokval_t
failed(cpp_eval_state_pt s)
{
    tokval_t tmp;
    s->recover = 1;
    MAKE_FAIL(tmp);
    longjmp(s->exception,1);
    return tmp;
}

static int
escaped_char(int c, cpp_eval_state_pt s)
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
	for (i = 0, val = 0; is_octal_digit(c) && i < 3; i++, c = getc(s)) {
	    val = val * 8 + (c - '0');
	}
	return val;

      case 'x':
	for (i = 0, val = 0; is_hex_digit(c) && i < 2; i++, c = getc(s)) {
	    val *= 16;
	    if (is_digit(c)) {
		val += (c - '0');
	    }
	    else if (c <= 'F') {
		val += (c - ('A' - 10));
	    }
	    else {
		val += (c - ('a' - 10));
	    }
	}
	return val;

      default:
	break;
    }

    return c;
}

static int
scan_string(int c, cpp_eval_state_pt s)
{
    buffer_t buf;

    buf_init(&buf);

    for (;;) {
	if (is_eof(c)) {
	    goto end_of_string;
	}
	if (is_eol(c)) {
	    goto end_of_string;
	}
	switch (c) {
	  case '"':
	    c = getc(s);
	    goto end_of_string;
	  case '\\':
	    c = getc(s);
	    switch (c) {
	      case '\n':
		c = getc(s);
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
		buf_add(&buf, escaped_char(c,s));
		c = getc(s);
		break;
	      default:
		buf_add(&buf, c);
		c = getc(s);
		break;
	    }
	    break;
	  default:
	    buf_add(&buf, c);
	    c = getc(s);
	    break;
	}
    }

  end_of_string:
    {
	char * str = buf_get_str(&buf);
	MAKE_STRING(s->nextval, str);
    }
    return c;
}

static int
scan_char_const(int c, cpp_eval_state_pt s)
{
    host_int_t cval = 0;

    for (;;) {
	if (is_eof(c)) {
	    goto end_char_const;
	}
	if (is_eol(c)) {
	    goto end_char_const;
	}
	switch (c) {
	  case '\'':
	    c = getc(s);
	    goto end_char_const;
	  case '\\':
	    c = getc(s);
	    switch (c) {
	      case '\n':
		c = getc(s);
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
		cval <<= 8;
		cval |= escaped_char(c,s);
		c = getc(s);
		break;
	      default:
		cval <<= 8;
		cval |= c;
		c = getc(s);
		break;
	    }
	    break;
	  default:
	    cval <<= 8;
	    cval |= c;
	    c = getc(s);
	    break;
	}
    }

  end_char_const:
    MAKE_INT(s->nextval, cval, 10);
    return c;
}


static int
magnitude(int c, host_int_t * val, cpp_eval_state_pt s)
{
    host_int_t m;
    int sign;

    if (! is_magnitude(c)) {
	*val = 1;
	return c;
    }


    c = getc(s);
    sign = 1;

    if (c == '-') {
	sign = -1;
	c = getc(s);
    }
    else if (c == '+') {
	c = getc(s);
    }

    for (m = 0; is_digit(c); ) {
	m = m * 10 + (c - '0');
	c = getc(s);
    }

    *val = m * sign;
    return c;
}

static int
scan_digit(int c, cpp_eval_state_pt s)
{
    host_int_t val;
    int base;

    if ( c != '.') {
	if (c == '0') {
	    val = 0;
	    c = getc(s);
	    if (c == 'x' || c == 'X') {
		base = 16;
		for (;;) {
		    c = getc(s);
		    if (is_digit(c)) {
			val = (val<<4) + (c - '0');
		    }
		    else if (c >= 'A' && c <= 'F') {
			val = (val<<4) + 10 + (c - 'A');
		    }
		    else if (c >= 'a' && c <= 'f') {
			val = (val<<4) + 10 + (c - 'a');
		    }
		    else {
			break;
		    }
		}
	    }
	    else {
		base = is_octal_digit(c)? 8: 10;
		for (;;) {
		    if (!is_octal_digit(c)) {
			break;
		    }
		    val = (val<<3) + (c - '0');
		    c = getc(s);
		}
	    }
	}
	else {
	    base = 10;
	    val = c - '0';
	    for (;;) {
		c = getc(s);
		if (! is_digit(c)) break;
		val = val * 10 + (c - '0');
	    }
	}
    }

    if (int_modifier(c)) {
	c = getc(s);
    }

    if (c == '.') {
	host_float_t dval, d;
	int tmp;

	dval = (host_float_t) val;
	d = 0.1;

	for (;;) {
	    c = getc(s);
	    if (! is_digit(c)) break;
	    tmp = c - '0';
	    dval = dval + (d * (host_float_t)tmp);
	    d = d * 0.1;
	}


	/* handle magnitude */
	if (is_magnitude(c)) {
	    host_int_t vm;
	    host_float_t m;

	    c = magnitude(c, &vm, s);

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

	if (float_modifier(c)) {
	    c = getc(s);
	}

	MAKE_FLOAT(s->nextval, dval);
	s->nexttok = tok_float;
	return c;
    }

    while (int_modifier(c)) {
	c = getc(s);
    }

    MAKE_INT(s->nextval, val, base);
    s->nexttok = tok_discrete;
    return c;
}

extern typeinfo_pt grok_coercion(char *);  /* TBD: put in gen_macros.h */

static int
scan_id(int c, cpp_eval_state_pt s)
{
    char id[100];
    char * cp = id;
    macro_t * m ;
    typeinfo_pt type;


    while (isalnum(c) || c=='_') {
	*cp++ = c;
	c = getc(s);
    }
    *cp = '\0';

    m = macro_find(id);

    if (m && m->macro_evald) {
	s->nextval = m->const_value;
	switch (s->nextval.eval_result_kind) {
	case eval_int:
	    s->nexttok = tok_discrete;
	    break;
	case eval_float:
	    s->nexttok = tok_float;
	    break;
	case eval_string:
	    s->nexttok = tok_string;
	    break;
	default:
	case eval_failed:
	    s->nexttok = tok_error;
	    break;
	}
    } else if ( (type = grok_coercion(id)) ) {

	s->nexttok = tok_type;
	s->nextval.eval_result_kind = eval_type;
	s->nextval.eval_result.tval = type;

    } else {
	s->nexttok = tok_error;
    }
    return c;
}

static int
skip_c_comment(int c, cpp_eval_state_pt s)
{
    for (;;) {
	if (c == BAD_INPUT) {
	    return BAD_INPUT;
	}
	if (c == 0 || is_eof(c) || is_eol(c)) {
	    return c;
	}
	switch (c) {
	  case '\\':
	    c = getc(s);
	    c = getc(s);
	    break;
	  case '*':
	    c = getc(s);
	    if (c == '/') {
		return getc(s);
	    }
	    break;
	  default:
	    c = getc(s);
	    break;
	}
    }
}

int
skip_cpp_comment(int c, cpp_eval_state_pt s)
{
    for (;;) {
	if (c == BAD_INPUT) {
	    return BAD_INPUT;
	}
	if ((c == 0) || is_eof(c))
	    return c;
	else if (is_eol(c))
	    return getc(s);
	else
	    c = getc(s);
    }
}

static int
advance(cpp_eval_state_pt s)
{
    s->curtok = s->nexttok;
    s->curval = s->nextval;

    for (;;) {
	if (s->c == BAD_INPUT) {
	    s->nexttok = tok_error;
	    break;
	}
	if (s->c == 0 || is_eof(s->c) || is_eol(s->c)) {
	    s->nexttok = tok_eof;
	    break;
	}
	if (is_white(s->c)) {
	    s->c = getc(s);
	    continue;
	}
	if (is_digit(s->c) || (s->c == '.')) {
	    s->c = scan_digit(s->c, s);
	    break;
	}
	if (is_alpha(s->c)) {
	    s->c = scan_id(s->c,s);
	    break;
	}

	s->nexttok = s->c;
	s->c = getc(s);

	switch (s->nexttok) {
	  case '/':
	    if (s->c == '*') {
		s->c = skip_c_comment(getc(s),s);
		continue;
	    } else if (s->c == '/') {
		s->c = skip_cpp_comment(getc(s),s);
		continue;
	    }
	    break;
	  case '\"':
	    s->c = scan_string(s->c,s);
	    s->nexttok = tok_string;
	    break;
	  case '\'':
	    s->c = scan_char_const(s->c,s);
	    s->nexttok = tok_discrete;
	    break;
	  case '<':
	    if (s->c == '<') {
		s->c = getc(s);
		s->nexttok = tok_shift_left;
	    }
	    else if (s->c == '=') {
		s->c = getc(s);
		s->nexttok = tok_leq;
	    }
	    break;
	  case '>':
	    if (s->c == '>') {
		s->c = getc(s);
		s->nexttok = tok_shift_right;
	    }
	    else if (s->c == '=') {
		s->c = getc(s);
		s->nexttok = tok_geq;
	    }
	    break;
	  case '=':
	    if (s->c == '=') {
		s->c = getc(s);
		s->nexttok = tok_eq;
	    }
	    break;
	  case '!':
	    if (s->c == '=') {
		s->c = getc(s);
		s->nexttok = tok_neq;
	    }
	    break;
	  case '|':
	    if (s->c == '|') {
		s->c = getc(s);
		s->nexttok = tok_or;
	    }
	    break;
	  case '&':
	    if (s->c == '&') {
		s->c = getc(s);
		s->nexttok = tok_and;
	    }
	    break;
	  default:
            break;
	}

	break;
    }


    return s->curtok;
}

static int
expect(int tok, cpp_eval_state_pt s)
{
    if (s->curtok != tok) {
	longjmp(s->exception, 1);
	return 0;
    }
    return 1;
}

static tokval_t eval();

static typeinfo_pt
ctype(cpp_eval_state_pt s)
{
    if (s->curtok==tok_type) {
	return s->nextval.eval_result.tval;
    } else {
	return 0;
    }
}

static tokval_t
cast( tokval_t val, typeinfo_pt type)
{
    /* TBD: convert if necessary */
    val.explicit_type = type;
    return val;
}

static tokval_t
term(cpp_eval_state_pt s)
    /* unary expression */
{
    tokval_t val;

    switch (s->curtok) {
      case tok_discrete:
      case tok_float:
      case tok_string:
	val = s->curval;
	advance(s);
	return val;
      case '-':
	advance(s);
	val = term(s);
	if (IS_EVAL_INT(val)) {
	    EVAL_INT(val) = -EVAL_INT(val);
	    return val;
	}
	if (IS_EVAL_FLOAT(val)) {
	    EVAL_FLOAT(val) = -EVAL_FLOAT(val);
	    return val;
	}
	MAKE_FAIL(val);
	return val;
      case '+':
	advance(s);
	val = term(s);
	MAKE_FAIL(val);
	return val;
      case '!':
	advance(s);
	val = term(s);
	if (IS_EVAL_INT(val)) {
	    EVAL_INT(val) = !EVAL_INT(val);
	    return val;
	}
	MAKE_FAIL(val);
	return val;
      case '~':
	advance(s);
	val = term(s);
	if (IS_EVAL_INT(val)) {
	    EVAL_INT(val) = ~EVAL_INT(val);
	    return val;
	}
	MAKE_FAIL(val);
	return val;
      case '(':
	advance(s);
	/* Here's the place to look for a typecast */
	{
	    typeinfo_pt type = ctype(s);
	    if (type) {
		advance(s);
		if (expect(')', s)) {
		    advance(s);
		} else {
		    MAKE_FAIL(val);
		    return val;
		}
		val = eval(s);
		val = cast(val, type);
		return val;
	    }
	}

	val = eval(s);

	if (expect(')',s)) {
	    advance(s);
	} else {
	    MAKE_FAIL(val);
	}
	return val;
      default:
	return failed(s);
    }
}

static tokval_t
f10(cpp_eval_state_pt s)
    /* multiplicative expression */
{
    tokval_t l,r;
    int op;

    l = term(s);

    for (;;) {
	switch (s->curtok) {
	  case '*':
	  case '/':
	  case '%':
	    op = s->curtok;
	    break;
	  default:
	    return l;
	}

	advance(s);

	r = term(s);

	switch (promote(&l,&r)) {
	  case eval_int:
	    switch (op) {
	      case '*':
		EVAL_INT(l) = EVAL_INT(l) * EVAL_INT(r);
		break;
	      case '/':
		if (EVAL_INT(r) == 0) {
		    return failed(s);
		}
		EVAL_INT(l) = EVAL_INT(l) / EVAL_INT(r);
		break;
	      case '%':
		if (EVAL_INT(r) == 0) {
		    return failed(s);
		}
		EVAL_INT(l) = EVAL_INT(l) % EVAL_INT(r);
		break;
	    }
	    break;
	  case eval_float:
	    switch (op) {
	      case '*':
		EVAL_FLOAT(l) = EVAL_FLOAT(l) * EVAL_FLOAT(r);
		break;
	      case '/':
		if (EVAL_FLOAT(r) == 0.0) {
		    return failed(s);
		}
		EVAL_FLOAT(l) = EVAL_FLOAT(l) / EVAL_FLOAT(r);
		break;
	      case '%':
		if (EVAL_FLOAT(r) == 0.0) {
		    return failed(s);
		}
		EVAL_FLOAT(l) = EVAL_FLOAT(l) / EVAL_FLOAT(r);
		break;
	    }
	    break;
	  default:
	    return failed(s);
	}
    }
}

static tokval_t
f9(cpp_eval_state_pt s)
    /* additive expression */
{
    tokval_t l,r;
    int op;


    l = f10(s);

    for (;;) {
	switch (s->curtok) {
	  case '+':
	  case '-':
	    op = s->curtok;
	    break;
	  default:
	    return l;
	}

	advance(s);

	r = f10(s);

	switch (promote(&l,&r)) {
	  case eval_int:
	    switch (op) {
	      case '+':
		EVAL_INT(l) = EVAL_INT(l) + EVAL_INT(r);
		break;
	      case '-':
		EVAL_INT(l) = EVAL_INT(l) - EVAL_INT(r);
		break;
	    }
	    break;
	  case eval_float:
	    switch (op) {
	      case '+':
		EVAL_FLOAT(l) = EVAL_FLOAT(l) + EVAL_FLOAT(r);
		break;
	      case '-':
		EVAL_FLOAT(l) = EVAL_FLOAT(l) - EVAL_FLOAT(r);
		break;
	    }
	    break;
	  default:
	    return failed(s);
	}
    }
}

static tokval_t
f8(cpp_eval_state_pt s)
    /* shift expression */
{
    tokval_t l,r;
    int op;


    l = f9(s);

    for (;;) {
	switch (s->curtok) {
	  case tok_shift_left:
	  case tok_shift_right:
	    op = s->curtok;
	    break;
	  default:
	    return l;
	}

	advance(s);

	r = f9(s);

	if (IS_EVAL_INT(l) & IS_EVAL_INT(r)) {
	    switch (op) {
	      case tok_shift_left:
		EVAL_INT(l) = EVAL_INT(l) << EVAL_INT(r);
		break;
	      case tok_shift_right:
		EVAL_INT(l) = EVAL_INT(l) >> EVAL_INT(r);
		break;
	    }
	}
	else {
	    return failed(s);
	}
    }
}

static tokval_t
f7(cpp_eval_state_pt s)
    /* relational expression */
{
    tokval_t l,r;
    int op, tmp;


    l = f8(s);

    for (;;) {
	switch (s->curtok) {
	  case tok_gt:
	  case tok_geq:
	  case tok_lt:
	  case tok_leq:
	    op = s->curtok;
	    break;
	  default:
	    return l;
	}

	advance(s);

	r = f8(s);


	switch (promote(&l,&r)) {
	  case eval_int:
	    switch (op) {
	      case tok_gt:
		EVAL_INT(l) = EVAL_INT(l) > EVAL_INT(r);
		break;
	      case tok_geq:
		EVAL_INT(l) = EVAL_INT(l) >= EVAL_INT(r);
		break;
	      case tok_lt:
		EVAL_INT(l) = EVAL_INT(l) < EVAL_INT(r);
		break;
	      case tok_leq:
		EVAL_INT(l) = EVAL_INT(l) <= EVAL_INT(r);
		break;
	    }
	    break;
	  case eval_float:
	    switch (op) {
	      case tok_gt:
		tmp = EVAL_FLOAT(l) > EVAL_FLOAT(r);
		break;
	      case tok_geq:
		tmp = EVAL_FLOAT(l) >= EVAL_FLOAT(r);
		break;
	      case tok_lt:
		tmp = EVAL_FLOAT(l) < EVAL_FLOAT(r);
		break;
	      case tok_leq:
		tmp = EVAL_FLOAT(l) <= EVAL_FLOAT(r);
		break;
	    }
	    MAKE_INT(l, tmp, 10);
	    break;
	  default:
	    return failed(s);
	}
    }
}

static tokval_t
f6(cpp_eval_state_pt s)
    /* equality expression */
{
    tokval_t l,r;
    int op, tmp;

    l = f7(s);

    for (;;) {
	switch (s->curtok) {
	  case tok_eq:
	  case tok_neq:
	    op = s->curtok;
	    break;
	  default:
	    return l;
	}

	advance(s);

	r = f7(s);

	switch (promote(&l,&r)) {
	  case eval_int:
	    if (op == tok_eq) {
		EVAL_INT(l) = EVAL_INT(l) == EVAL_INT(r);
	    }
	    else {
		EVAL_INT(l) = EVAL_INT(l) != EVAL_INT(r);
	    }
	    break;
	  case eval_float:
	    if (op == tok_eq) {
		tmp = EVAL_FLOAT(l) == EVAL_FLOAT(r);
	    }
	    else {
		tmp = EVAL_FLOAT(l) != EVAL_FLOAT(r);
	    }
	    MAKE_INT(l, tmp, 10);
	    break;
	  default:
	    return failed(s);
	}
    }
}

static tokval_t
f5(cpp_eval_state_pt s)
    /* and (&) expression */
{
    tokval_t l,r;

    l = f6(s);

    for (;;) {
	if (s->curtok != '&') {
	    return l;
	}

	advance(s);

	r = f6(s);

	if (IS_EVAL_INT(l) & IS_EVAL_INT(r)) {
	    EVAL_INT(l) = EVAL_INT(l) & EVAL_INT(r);
	}
	else {
	    return failed(s);
	}
    }
}

static tokval_t
f4(cpp_eval_state_pt s)
    /* xor (^) expression */
{
    tokval_t l,r;

    l = f5(s);

    for (;;) {
	if (s->curtok != '^') {
	    return l;
	}

	advance(s);

	r = f5(s);

	if (IS_EVAL_INT(l) & IS_EVAL_INT(r)) {
	    EVAL_INT(l) = EVAL_INT(l) ^ EVAL_INT(r);
	}
	else {
	    return failed(s);
	}
    }
}

static tokval_t
f3(cpp_eval_state_pt s)
    /* inclusive-or (|) expression */
{
    tokval_t l,r;

    l = f4(s);

    for (;;) {
	if (s->curtok != '|') {
	    return l;
	}

	advance(s);

	r = f4(s);

	if (IS_EVAL_INT(l) && IS_EVAL_INT(r)) {
	    EVAL_INT(l) = EVAL_INT(l) | EVAL_INT(r);
	}
	else {
	    return failed(s);
	}
    }
}

static tokval_t
f2(cpp_eval_state_pt s)
    /* logical-and (&&) expression */
{
    tokval_t l,r;

    l = f3(s);
    for (;;) {
	if (s->curtok != tok_and) {
	    return l;
	}

	advance(s);

	r = f3(s);

	if (IS_EVAL_INT(l) && IS_EVAL_INT(r)) {
	    EVAL_INT(l) = EVAL_INT(l) && EVAL_INT(r);
	}
	else {
	    return failed(s);
	}
    }
}

static tokval_t
f1(cpp_eval_state_pt s)
    /* logical-or (||) expression */
{
    tokval_t l,r;

    l = f2(s);

    for (;;) {
	if (s->curtok != tok_or) {
	    return l;
	}

	advance(s);

	r = f2(s);

	if (IS_EVAL_INT(l) && IS_EVAL_INT(r)) {
	    EVAL_INT(l) = EVAL_INT(l) || EVAL_INT(r);
	}
	else {
	    return failed(s);
	}
    }
}

static tokval_t
eval(cpp_eval_state_pt s)
{
    tokval_t cond,tru,fals,tmp;

    tmp = f1(s);

    for (;;) {
	if (s->curtok != '?') {
	    return tmp;
	}

	advance(s);

	tru = eval(s);

	if (!expect(':',s)) {
	    MAKE_FAIL(tmp);
	    return tmp;
	}

	advance(s);

	fals = eval(s);

	if (IS_EVAL_INT(cond)) {
	    tmp = (EVAL_INT(cond) != 0) ? tru : fals;
	}
	else {
	    return failed(s);
	}
    }
}

cpp_eval_state_t state0;  /* all fields zeroed */

cpp_eval_result_t
cpp_eval(char * str)
{
    scan_position_t *   newpos;
    scan_position_t *   savepos;
    cpp_control_state_t save_state;
    cpp_control_state_t new_state;
    cpp_eval_state_t    state = state0;
    cpp_eval_state_pt   s = &state;


    assert(str != NULL);

    buf_init(&s->evalbuf);

    s->result.eval_result_kind = eval_int;
    s->result.eval_result.ival = 0;
    s->result.explicit_type = 0;

    new_state.skip_else = 0;
    new_state.cur_scope = 0;
    new_state.gen_scope = 0;
    new_state._parsing = 1;

    newpos = NEW(scan_position_t);

    newpos->scan_kind = scan_text;
    newpos->scan.text = str;

    cpp_set_state(newpos, &new_state, &savepos, &save_state);

    if (savepos != NULL) {
	newpos->scan_pos = savepos->scan_pos;
    }

    s->c = ' ';
    s->recover = 0;

    if (!setjmp(s->exception)) {
	advance(s);
	advance(s);
	s->result = eval(s);
    } else {
	MAKE_FAIL(s->result);
    }

    cpp_set_state(savepos, &save_state, &newpos, &new_state);

    if (s->curtok != tok_eof) {
	MAKE_FAIL(s->result);
    }

    return s->result;
}
