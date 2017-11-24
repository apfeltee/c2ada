#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>

#include "allocate.h"
#include "errors.h"
#include "il.h"
#include "anonymous.h"
#include "buffer.h"
#include "cpp.h"
#include "files.h"
#include "gen.h"
#include "gen_macros.h"
#include "macro.h"
#include "ada_name.h"
#include "cpp_hide.h"
#include "cpp_eval.h"
#include "format.h"
#include "units.h"
#include "stab.h"
#include "stmt.h"
#include "types.h"
#include "comment.h"

macro_t *unit_macros[MAX_UNIQ_FNAMES];
static macro_t *unknown_macro_list = NULL;

static char * no_empty_params(char *);

static macro_function_t * grok_macro_function(char * rhs);
struct typeinfo_t * grok_coercion(char *type_name);

static int match_param_name(char *formal_name, char *body_name);

extern int auto_package;
extern int ada_version;

static void
macro_enq(m)
    macro_t *m;
{
    macro_t *t, *last;
    unit_n   unit;

    assert(m != NULL);

    unit = pos_unit(m->macro_definition);

    assert(unit < MAX_UNIQ_FNAMES);

    for (last = NULL, t = unit_macros[unit]; t; t = t->macro_next) {
	last = t;
    }

    if (last) {
	last->macro_next = m;
    } else {
	assert(unit_macros[unit] == NULL);
	unit_macros[unit] = m;
    }
}

static void
dump_macros(list, max)
    macro_t *list;
    int max;
{
    macro_t *m;
    int i = 0;

    fprintf(stderr, "----------- macro dump --------\n");
    for (m = list; m; m = m->macro_next) {
	fprintf(stderr, "name <%s>, ada_name <%s>, body <%s>, ",
	    m->macro_name, m->macro_ada_name, m->macro_body);
	fprintf(stderr, "body_len %d, params %d\n",
	    m->macro_body_len, m->macro_params);
	if(i++ >= max)
	    break;
    }
    fprintf(stderr, "----------- end macro dump --------\n");
}

void
gen_macro_warnings()
{
    extern int macro_warnings;
    macro_t *m;

    if(macro_warnings) {
	for(m=unknown_macro_list; m; m = m->macro_next) {
	    printf("%s untranslated, %s line %d\n",
		   m->macro_name,
		   file_name(m->macro_definition),
		   (int) line_number(m->macro_definition));
	}
    }
    unknown_macro_list = NULL;
}

static int
could_be_ada_ident(s)
    register char *s;
{
    if (!s || !is_alpha((int) s[0])) 	return 0;
    while(is_alpha_numeric( (int) *s++)) { }
    return is_alpha_numeric( (int) s[-2]);
}

static void
add_to_unknown_list(unknown)
    macro_t *unknown;
{
    macro_t *m;

    if(unknown_macro_list == NULL) {
	unknown_macro_list = unknown;
    } else {
	for(m = unknown_macro_list; m->macro_next != NULL; m = m->macro_next)
	    if(m == unknown)
		return;
	m->macro_next = unknown;
    }
    unknown->macro_next = NULL;
}

static void
gen_const_char(name, val)
    char *name, val;
{
    char *buf;

    indent_to(4);
    put_string(name);
    MAX_INDENT(max_const_name_indent);
    indent_to(max_const_name_indent);
    putf(": constant %s", ada_version<1995? "Character" : "Char");
    put_string(" := ");

    buf = char_to_string(val,TRUE);
    if ((strlen(buf) == 1) || !strcmp(buf, "\"\"")) {
	put_char('\'');
	put_string(buf);
	put_char('\'');
    } else {
	put_string(buf);
    }
    put_char(';');
}

static void
gen_const_int(name, tname, val, base)
    char *name, *tname;
    host_int_t val;
    int base;
{
    indent_to(4);
    put_string(name);
    MAX_INDENT(max_const_name_indent);
    indent_to(max_const_name_indent);
    put_string(": constant ");
    if(tname != NULL)
	put_string(tname);
    put_string(" := ");
    print_value(val, base);
    put_char(';');
}

static void
gen_const_synonym( char*name, char* tname, char * value )
{
    indent_to(4);
    put_string(name);
    MAX_INDENT(max_const_name_indent);
    indent_to(max_const_name_indent);
    put_string(": constant ");
    if (tname != NULL) put_string(tname);
    put_string( " := ");
    put_string(value);
    put_string(";");
}

static void
gen_const_ref(name, tname, val)
    char *name, *tname;
    host_int_t val;
{
    /* This routine is only called to initialize null pointers */
    assert(val==0);
    indent_to(4);
    put_string(name);
    MAX_INDENT(max_const_name_indent);
    indent_to(max_const_name_indent);
    put_string(": constant ");
    if (tname != NULL) put_string(tname);
    put_string(" := null;");
}

static void
gen_const_float(name, val)
    char *name;
    char *val;
{
    char c, *p;

    indent_to(4);
    put_string(name);
    MAX_INDENT(max_const_name_indent);
    indent_to(max_const_name_indent);
    put_string(": constant := ");
    /* Ada floats must be 0.1, can't be .1 */
    for(p = val; *p != '\0'; p++) {
	c = *p;
	if ((c == '.') && ((p == val) || !isdigit(p[-1])))
	    put_char('0');
	put_char(c);
	if ((c == '.') && (!isdigit(p[1])))
	    put_char('0');
    }
    put_char(';');
}

static void
gen_const_rename(name, unit, typ)
    char *name, *typ;
    int unit;
{
    char *p;

    indent_to(4);
    put_string(name);
    MAX_INDENT(max_const_name_indent);
    indent_to(max_const_name_indent);
    put_string(": constant");
    put_string(typ);
    put_string(" := ");

    p = unit_name(unit);
    assert(p != NULL);

    put_string(p);
    put_char('.');
    put_string(name);
    put_char(';');
}

static void
gen_const_bool(name)
    char *name;
{

    indent_to(4);
    put_string(name);
    MAX_INDENT(max_const_name_indent);
    indent_to(max_const_name_indent);
    put_string(": constant Boolean := True;");
}

static void
gen_comment(macro_t * m)
{
    if (m->eol_comment) {
	print_comment(m->eol_comment);
    } else {
	print_position(m->macro_definition);
    }
}

#if 0
parse_macro( char * body )
{
    boolean parsed;
    scan_string_init(body);
    parsed = !yyparse();
}

process_macro_expression( node_pt expr )
{

}
#endif


char *
combined_name(name, ord)
    char *name;
    int ord;
{
    static char buf[200];

    if(ord == -1)
	return name;
    sprintf(buf, "%s.%s", unit_name(ord), name);
    return buf;
}

char *
packaged_name(char *name, file_pos_t macro_pos, file_pos_t subp_pos)
{
    if (pos_unit(macro_pos) == pos_unit(subp_pos)) {
	return name;
    } else {
	return combined_name(name, pos_unit(subp_pos));
    }
}

static int
gen_mconst(m, import)
    macro_t *m;
    int import;
{
    cpp_eval_result_t result;
    int is_wide_string = 0;
    char * tname = NULL;
    typeinfo_t * t = NULL;
    macro_t * mbody;

    if ((m->macro_params != -1) || !strcmp(m->macro_name, "NULL")) {
	return 0;
    }

    output_to(m->macro_declared_in_header);
    put_comment_block( m->comment, 4);

    if ((m->macro_body == NULL) || (m->macro_body_len < 1)) {
	gen_const_bool(m->macro_ada_name);
	gen_comment(m);
	return TRUE;
    }


    if (strstr(m->macro_name, "NULL") != NULL) {
	warning(file_name(m->macro_definition),
		line_number(m->macro_definition),
		"macro %s contains NULL", m->macro_name);
    }

    /* TBD:    parse_macro(m->macro_body);    */

    mbody = macro_find(m->macro_body);
    if (mbody && m->macro_evald) {
	/* Duplicates an existing constant */
	typeinfo_pt type = mbody->const_value.explicit_type;
	if (type) {
	    tname = type_nameof(type, 0, 0);
	} else if (IS_EVAL_STRING(mbody->const_value)) {
	    tname = string_name(0);
	} else {
	    tname = 0;
	}
	gen_const_synonym(m->macro_ada_name, tname,
	    packaged_name(mbody->macro_ada_name,
			  m->macro_definition,
			  mbody->macro_definition));
	gen_comment(m);
	return TRUE;
    }

    result = cpp_eval(m->macro_body);
    if ( (t=result.explicit_type) ) {
	tname = type_nameof(t, 0, 0);
    }


    if (EVAL_FAILED(result) &&
	(ada_version >= 1995) &&
	(m->macro_body[0] == 'L') && (m->macro_body[1] == '"')) {
	    is_wide_string = 1;
	    result = cpp_eval(&m->macro_body[1]);
    }


    if (EVAL_FAILED(result)) {
	/* make a try at finding ones coercions like #define x (int) 123 */
	char *leftparen, *leftparen2, *rightparen;

	rightparen = strchr(m->macro_body, ')');
	if(rightparen != NULL) {
	    for(leftparen2 = m->macro_body, leftparen = NULL;
		(leftparen2 != NULL) && (leftparen2 < rightparen); ) {

		leftparen = leftparen2;
		leftparen2 = strchr(leftparen+1,'(');
	    }
	    if(leftparen != NULL) {
		char buf[2048], *p1, *p2;
		int len;

		p1 = leftparen;
		while(is_white( (int) *++p1))
		    ;
		p2 = rightparen;
		while(is_white( (int) *--p2))
		    ;
		len = p2 - p1 + 1;
		strncpy(buf, p1, len);
		buf[len] = '\0';
		t = grok_coercion(buf);
		if (t != NULL) {
		    strcpy(buf, m->macro_body);
		    memset(&buf[leftparen-m->macro_body],
			   ' ', rightparen-leftparen+1);
		    result = cpp_eval(buf);
		    tname = type_nameof(t, 0, 0);
		    goto after_coercion;
		}
	    }
	}
	return FALSE;
    }

after_coercion:
    if (IS_EVAL_INT(result)) {
	char *first = strchr(m->macro_body, '\'');

	if (first != NULL) {
	    gen_const_char(m->macro_ada_name, EVAL_INT(result));
	} else if (import == -1) {
	    int res = EVAL_INT(result);
	    if ((t != NULL) && (t->type_kind == pointer_to) && (res == 0)) {
		gen_const_ref(m->macro_ada_name, tname, res);

	    } else {
		gen_const_int(m->macro_ada_name, tname, res, result.base);
	    }
	} else {
	    gen_const_rename(m->macro_ada_name, tname, import, "");
	}
	gen_comment(m);
	return TRUE;
    }
    if (IS_EVAL_FLOAT(result)) {
	if (import == -1) {
	    gen_const_float(m->macro_ada_name, m->macro_body);
	} else {
	    gen_const_rename(m->macro_ada_name, import, "");
	}
	gen_comment(m);
	return TRUE;
    }
    if (IS_EVAL_STRING(result)) {
	if (import == -1) {
	    gen_char_array(m->macro_ada_name, EVAL_STRING(result),
			     is_wide_string, TRUE);
	} else {
	    char buf[20];

	    sprintf(buf, " %s", string_name(0));
	    gen_const_rename(m->macro_ada_name, import, buf);
	}
	gen_comment(m);
	return TRUE;
    }
    return FALSE;
}

static void
do_macro_body(buf, ret, coercion, fname, all, params)
    char *buf, *ret, *coercion, *fname, *all, *params;
{
    params = no_empty_params(params);
    if(coercion == NULL)
	sprintf(buf, "%s %s%s%s;", ret, fname, all, params);
    else
	sprintf(buf, "%s %s(%s%s%s);", ret, coercion, fname, all, params);
}

static char *
coercion_name(has_coercion, coercion)
    int has_coercion;
    typeinfo_t *coercion;
{
    char *s, *dot;

    if(!has_coercion)
	return NULL;
    s = type_nameof(coercion, 0, 0);
    dot = strrchr(s, '.');
    if((dot != NULL) && !strcmp(dot, ".void"))
	return NULL;
    return s;
}

static symbol_t *
bogus_param(new_param_name)
    char *new_param_name;
{
    symbol_t *param;

    param = new_sym();
    param->sym_ada_name = new_param_name;
    param->sym_type = bogus_type;
    return param;
}

static symbol_t *
copy_nth_param(func_sym, n, new_param_name)
    symbol_t *func_sym;
    int n;
    char *new_param_name;
{
    symbol_t *param;
    int i;

    for (param = func_sym->sym_tags, i = 0;
	 param != NULL;
	 param = param->sym_parse_list, i++) {
	if(i == n) {
	    param = copy_sym(param);
	    param->sym_ada_name = new_param_name;
	    return param;
	}
    }
    return bogus_param(new_param_name);
}

static void
gen_macro_func(func_sym, m, import)
    symbol_t *func_sym;
    int import;
    macro_t *m;
{
    symbol_t *macro_sym, *mparam, *last_mparam;
    typeinfo_t *coercion;
    int i, j, is_pointer, has_coercion;
    char *name1, *name2, *rhsname;
    char body_buf[MAX_OUTBUF_LEN];

    assert(m != NULL);
    assert(m->macro_func != NULL);
    macro_sym = copy_sym(func_sym);
    is_pointer = m->macro_func->mf_is_pointer;
    if(is_pointer) {
	assert((func_sym->sym_type != NULL) &&
	       (func_sym->sym_type->type_kind == pointer_to) &&
	       (func_sym->sym_type->type_next != NULL) &&
	       (func_sym->sym_type->type_next->type_kind == function_type) &&
	       (func_sym->sym_type->type_base != NULL));
	macro_sym->sym_type = func_sym->sym_type->type_next;
    }
    macro_sym->sym_ada_name =
	(m->macro_ada_name) ?
	    m->macro_ada_name :
	    ada_name(m->macro_name, pos_unit(m->macro_definition));

    macro_sym->sym_def = m->macro_definition;
    macro_sym->sym_hash = 0;
    macro_sym->sym_hash_list = NULL;
    macro_sym->comment  = NULL;
    has_coercion = (m->macro_func->mf_coercion != NULL);
    if(has_coercion) {
	macro_sym->sym_type = copy_type(func_sym->sym_type);
	coercion = grok_coercion(m->macro_func->mf_coercion);
	if(coercion == NULL) {
	    error(__FILE__, __LINE__,
		  "type coercion (%s)", m->macro_func->mf_coercion);
	/* } else if (coercion->type_kind == pointer_to) { */
	    /* macro_sym->sym_type = coercion; */
	} else {
	    macro_sym->sym_type->type_next = coercion;
	}
	rhsname = strstr(m->macro_func->mf_rhs, func_sym->sym_ada_name);
	assert(rhsname != NULL);
    } else {
	rhsname = m->macro_func->mf_rhs;
    }
    if(m->macro_params == 0) {
	macro_sym->sym_tags = NULL;
    } else {
	for(i=0; i < m->macro_params; i++) {
	    mparam = NULL;
	    name1 = m->macro_param_vec[i];
	    for (j=0; j < m->macro_func->mf_nparams; j++) {
		name2 = m->macro_func->mf_params[j];
		if(match_param_name(name1, name2)) {
		    if(is_pointer)
			mparam = copy_nth_param(func_sym->sym_type->type_base,
						j, name1);
		    else
			mparam = copy_nth_param(func_sym, j, name1);
		    break;
		}
	    }
	    if(mparam == NULL)
		mparam = bogus_param(name1);
	    else
		mparam->sym_ada_name = ada_name(mparam->sym_ada_name, -1);
	    if(i == 0)
		macro_sym->sym_tags = mparam;
	    else
		last_mparam->sym_parse_list = mparam;
	    last_mparam = mparam;
	}
	if(i > 0)
	    mparam->sym_parse_list = NULL;
    }
    name1 = func_sym->sym_ident->node.str.form;
    name2 = strchr(strstr(rhsname, name1) + strlen(name1), '(');
    name1 = packaged_name(func_sym->sym_ada_name,
			  m->macro_definition, func_sym->sym_def);
    do_macro_body(body_buf, (is_function(func_sym)? "return": ""),
		  coercion_name(has_coercion, coercion),
		  name1, (is_pointer? ".all": ""), name2);
    macro_sym->has_initializer = 1;
    macro_sym->sym_value.body = new_stmt_Compound(
	m->macro_definition, NULL,
	new_stmt_MacroBody(m->macro_definition, body_buf));

    if (m->macro_declared_in_header) {
	output_to_spec();
	put_comment_block(m->comment, 4);
	gen_subp(macro_sym, NULL, 1, 1, 0);
	output_to_body();
	put_comment_block(m->comment, 4);
	gen_subp(macro_sym, NULL, 0, 1, 0);
    } else {
	output_to_body();
	put_comment_block(m->comment, 4);
	gen_subp(macro_sym, NULL, 0, 1, 0);
    }
}

static void
check_interf(m, sym)
    macro_t *m;
    symbol_t *sym;
{
    /*
     * if a macro and a function have the same name,
     * import the function before declaring the macro
     */
    if ((!strcmp(sym->sym_ada_name, m->macro_name)) && (sym->interfaced == 0)) {
	output_to(sym->_declared_in_header);
	interface_c(sym, 4);
	put_char('\n');
	sym->interfaced = 1;
    }
}

/* ***************
 * Top level stuff
 * ***************/

void
gen_macro_names()
{
    macro_t *          m;
    macro_function_t * mf;
    int                i;
    unit_n             unit;
    char c, *name, *p1, *p2, rhs_buf[MAX_OUTBUF_LEN];

    for (m = macro_list_head; m != NULL; m = m->macro_next) {
	if (could_be_ada_ident(m->macro_name)) {

	    unit = pos_unit(m->macro_definition);
	    m->macro_ada_name = ada_name(m->macro_name, unit);

	    if(m->macro_params == -1) {
		mf = grok_macro_function(m->macro_body);
		if(mf != NULL) {
		    mf->mf_rhs = m->macro_body;
		    m->macro_func = mf;
		    m->macro_params = 0;
		}
	    } else if((m->macro_body != NULL) &&
		      (m->macro_params != -1) && (m->macro_func == NULL)) {
		for(p1=m->macro_body, p2=rhs_buf; *p1; ) {
		    c = *p1++;
		    if(c == PARAM_START) {
			name = m->macro_param_vec[(*p1++) - 1];
			name = ada_name(name, -1);
			strcpy(p2, name);
			p2 += strlen(name);
			deallocate(name);
		    } else {
			*p2++ = c;
		    }
		}
		*p2 = '\0';
		m->macro_func = grok_macro_function(rhs_buf);
		if(m->macro_func != NULL)
		    m->macro_func->mf_rhs = new_string(rhs_buf);
		for(i = 0; i < m->macro_params; i++) {
		    name = m->macro_param_vec[i];
		    m->macro_param_vec[i] = ada_name(name, -1);
		    deallocate(name);
		}
	    }
	    m->macro_valid = 1;
	}
    }
}

void
rethread_macros()
{
    macro_t *m, *next;

    assert(auto_package);

    for (m = macro_list_head; m; m = next) {
	next = m->macro_next;
	m->macro_next = NULL;
	macro_enq(m);
    }
}

void
gen_macro_constants(m, import)
    macro_t *m;
    int import;
{
    while(m != NULL) {
	if(m->macro_valid && !m->macro_gened)
	    m->macro_gened = gen_mconst(m, import);
	m = m->macro_next;
    }
}

void
import_macro_constants()
{
    int i;
    int uord;

    if (!should_import())
	return;

    for (i = 0; ; i++) {
	uord = nth_direct_ref_unit_ord(i);
	if (uord == -1) break;
	gen_macro_constants(unit_macros[uord], uord);
    }
}

void gen_macro_types(m, import)
    macro_t *m;
    int import;
{
    typeinfo_t *t;

    while(m != NULL) {
	if ((m->macro_valid && !m->macro_gened) &&
	    (m->macro_func == NULL) &&
	    (m->macro_params == -1) &&
	    (t = grok_coercion(m->macro_body)) != NULL) {
	    output_to(m->macro_declared_in_header);
	    subtype_decl(m->macro_name, NULL, type_nameof(t, 0, 0),
			 4, NULL, m->macro_definition);
	    m->macro_gened = 1;
	}
	m = m->macro_next;
    }
}

void gen_macro_vars(macro_t * m, int import, int colonpos)
{
    symbol_t *sym;

    while (m != NULL) {
	if (m->macro_valid && !m->macro_gened && (m->macro_body != NULL)) {
	    /* for #define var2 var, generate "var2: integer renames var1 */
	    output_to(m->macro_declared_in_header);
	    if ((m->macro_func == NULL) &&
		((sym = find_sym(m->macro_body)) != NULL) &&
		(sym->sym_kind == var_symbol)) {

		symbol_pt msym = new_sym();
		*msym = *sym;

		msym->sym_ada_name = m->macro_name;
		msym->sym_def = m->macro_definition;
		msym->comment = m->comment;
		msym->emitted = FALSE;

		gen_var_or_field(msym, 4, colonpos, import,
				 combined_name(sym->sym_ada_name, import), 0);

		m->macro_gened = 1;
	    } else if ((m->macro_func != NULL) &&
		       ((sym = find_sym(m->macro_func->mf_fname)) != NULL) &&
		       is_function_pointer(sym->sym_type)) {
		/* #define f(x) (*pf)(x) */
		gen_macro_func(sym, m, import);
		m->macro_gened = 1;
	    }
	}
	m = m->macro_next;
    }
}

void gen_macro_funcs(m, import)
    macro_t *m;
    int import;
{
    symbol_t *sym;
    char *tmp_name, *rename;
    comment_block_pt tmp_comment;
    char buf[100];
    file_pos_t tmp_pos;

    while (m != NULL) {
	if(m->macro_valid && !m->macro_gened && (m->macro_body != NULL)) {
	    if((m->macro_func != NULL) &&
	       ((sym = find_sym(m->macro_func->mf_fname)) != NULL) &&
	       (sym->sym_kind == func_symbol) &&
	       !strcmp(sym->sym_ident->node.id.name, m->macro_func->mf_fname)) {
		/* recognize #define func1(...) func2(...), */
		check_interf(m, sym);
		gen_macro_func(sym, m, import);
		m->macro_gened = 1;

	    } else if ((m->macro_func == NULL) &&
		       ((sym = find_sym(m->macro_body)) != NULL) &&
		       (sym->sym_kind == func_symbol)) {

		/* recognize #define func1 func2 */
		tmp_name = sym->sym_ada_name;
		tmp_pos  = sym->sym_def;
		tmp_comment = sym->comment;

		if (pos_unit(sym->sym_def) == pos_unit(m->macro_definition)) {
		    rename = combined_name(tmp_name, import);
		} else {
		    rename = combined_name(tmp_name, pos_unit(sym->sym_def));
		}
		sprintf(buf, "\n               renames %s", rename);
		sym->sym_ada_name = m->macro_name;
		sym->sym_def = m->macro_definition;
		sym->comment = m->comment;

		gen_subp(sym, buf, m->macro_declared_in_header, 0, 0);
		sym->sym_ada_name = tmp_name;
		sym->sym_def = tmp_pos;
		sym->comment = tmp_comment;
		m->macro_gened = 1;
	    }
	}
	m = m->macro_next;
    }
}

void
finish_macros(m)
    macro_t *m;
{
    macro_t *next;

    while (m != NULL) {
	next = m->macro_next;
	if(!m->macro_gened)
	    add_to_unknown_list(m);
	m = next;
    }
}

/* ***************************************
 * Regular expression recognition routines
 * ***************************************/

#define INIT register char *sp = instring;

#include <regex.h>

/*
 * Implementation of regexp.h's step() in terms of regex.h.
 */

char *loc1, *loc2;

static int step_impl(const char *string, const regex_t *expbuf)
{
  regmatch_t match;
  if (regexec(expbuf, string, 1, &match, 0) == 0) {
    loc1 = (char *)string + match.rm_so;
    loc2 = (char *)string + match.rm_eo;
    return 1;
  } else {
    return 0;
  }
}

#define step(string, expbuf) step_impl(string, &expbuf)

int
regerr(err)
    int err;
{
    printf("regular expression error %d\n", err);
    exit(1);
}

/* used by grok_macro_function */
static char *ident = "[a-zA-Z_][a-zA-Z0-9_]*";
static regex_t ident_buf;
static char *in_parens = "(.*)";
static regex_t in_parens_buf;
static char *next_arg = "[,)]";
static regex_t next_arg_buf;

/* used by grok_coercion */
static char *int_name = "^[ 	]*int[ 	]*";
static regex_t int_buf;

static char *const_name = "^[ 	]*const[ 	]*";
static regex_t const_buf;

static char *char_name = "^[ 	]*char[ 	]*";
static regex_t char_buf;

static char *empty_params_name = "([ 	]*)";
static regex_t empty_params_buf;

static char *float_name = "^[ 	]*float[ 	]*";
static regex_t float_buf;

static char *double_name = "^[ 	]*double[ 	]*";
static regex_t double_buf;

static char *short_name = "^[ 	]*short[ 	]*";
static regex_t short_buf;

static char *long_name = "^[ 	]*long[ 	]*";
static regex_t long_buf;

static char *unsigned_name = "^[ 	]*unsigned[ 	]*";
static regex_t unsigned_buf;

static char *signed_name = "^[ 	]*signed[ 	]*";
static regex_t signed_buf;

static char *void_name = "^[ 	]*void[ 	]*";
static regex_t void_buf;

static char *star_name = "^[ 	]*\\*[ 	]*";
static regex_t star_buf;

static char *struct_name = "^struct[ 	]*";
static regex_t struct_buf;

static char *union_name = "^struct\\>";
static regex_t union_buf;

static char *
new_str(loc1, loc2)
    char *loc1, *loc2;
{
    int len = loc2-loc1;
    char *p;

    assert(len > 0);
    p = malloc(len+1);
    strncpy(p, loc1, len);
    p[len] = '\0';
    return p;
}

static void
init_regex()
{
    static int first_time = 1;

    if(!first_time)
	return;

#   ifdef DEBUG
    {
	extern void setbuf();
	setbuf(stdout, NULL);
    }
#   endif

#   define COMP(str, buf)			\
    regcomp(&buf, str, 0)

    COMP(ident,             ident_buf);
    COMP(in_parens,         in_parens_buf);
    COMP(next_arg,          next_arg_buf);
    COMP(empty_params_name, empty_params_buf);

    COMP(char_name,     char_buf);
    COMP(const_name,    const_buf);
    COMP(double_name,   double_buf);
    COMP(float_name,    float_buf);
    COMP(int_name,      int_buf);
    COMP(long_name,     long_buf);
    COMP(short_name,    short_buf);
    COMP(signed_name,   signed_buf);
    COMP(star_name,     star_buf);
    COMP(struct_name,   struct_buf);
    COMP(union_name,    union_buf);
    COMP(unsigned_name, unsigned_buf);
    COMP(void_name,     void_buf);
#   undef COMP

    first_time = 0;
}

static void
unbalanced(loc)
    char *loc;
{
    printf("error, unbalanced parens %s\n", loc);
}

static char *
skip_parens(loc, max)
    char *loc, *max;
{
    char c, *p;
    int paren_count = 0;

    p = loc;
    while( (c = *p) ) {
	switch(c) {
	    case ',':
		if(paren_count == 0) return p;
		break;
	    case ')':
		if (paren_count == 0) {
		    /* unbalanced(loc); return p; */
		    return NULL;
		}
		paren_count--;
		break;
	    case '(':
		paren_count++;
		break;
	    default:
		break;
	}
	if (++p >= max) {
	    if (paren_count != 0)
		/* unbalanced(loc); return p; */
		return NULL;
	    return p;
	}
    }
    return p;
}

static macro_function_t *
grok_macro_function(rhs)
    char *rhs;
{
    char *loc, *end_params, *before_func, *after_func, *star, *next;
    int siz;
    char *argv[256];
    int  argc = 0;
    macro_function_t f, *res;


    if (rhs == NULL)
	return NULL;
    if ((strchr(rhs, '{') != NULL) || (strchr(rhs, ';') != NULL))
	return NULL;
    {
	char *first_paren, *first_quote;
	first_paren = strchr(rhs, '(');
	first_quote = strchr(rhs, '"');
	if ((first_quote != NULL) &&
	    ((first_paren == NULL) || (first_paren > first_quote)))
	    return NULL;
    }
    init_regex();
    /* printf("getting parameters from %s\n", rhs); */
    f.mf_is_pointer = 0;
    f.mf_coercion   = NULL;
    if (step(rhs, ident_buf)) {	/* first identifier */
	before_func = loc1;
	after_func = loc2;
	if (step(after_func, in_parens_buf)) {	/* paren list */
	    if (strchr(after_func, ')') < loc1) {
		/* either a pointer_to_function (*f) or a coercion (int) */
		star = strchr(rhs, '*');
		if ((star == NULL) || (star > after_func)) {
		    /* coercion */
		    next = strchr(before_func, ')');
		    if (!step(next, ident_buf)) /* ident after coercion */
			return NULL;
		    f.mf_coercion = new_str(before_func, next);
		    before_func = loc1;
		    after_func = loc2;
		    if (!step(after_func-1, in_parens_buf)) /* params */
			return NULL;
		} else {
		    /* pointer_to_function */
		    f.mf_is_pointer = 1;
		}
	    }
	    f.mf_fname = new_str(before_func, after_func);
	    loc = loc1+1;
	    end_params = loc2-1;
	    while(loc < end_params) {
		next = skip_parens(loc, end_params);
		if (next == NULL)
		    return NULL;
		argv[argc++] = new_str(loc, next);
		loc = next + 1;
	    }
	    f.mf_nparams = argc;
	    siz = argc*sizeof(char *);
	    f.mf_params = (char **)malloc(siz);
	    (void)memcpy(f.mf_params, argv, siz);
	    res = (macro_function_t *)malloc(sizeof(macro_function_t));
	    memcpy(res, &f, sizeof(macro_function_t));
	    return res;
	}
    }
    return NULL;
}

struct typeinfo_t *
grok_coercion(coercion_name)
    char *coercion_name;
{
    struct typeinfo_t *typ = NULL;
    char *p = coercion_name;
    char *end_coercion_name = p + strlen(coercion_name);
    symbol_t *sym;
    char buf[100];

    init_regex();
    for(;;) {
	/* type modifiers */
#       define TM(buff, tmod)					\
	if (step(p, buff)) {					\
	    typ = concat_types(typ, typeof_typemod(tmod));	\
	    p = loc2;						\
	    if ((p >= end_coercion_name) || step(p, star_buf))	\
		break;						\
	    continue;						\
	}

	TM(const_buf,    TYPEMOD_CONST);
	TM(long_buf,     TYPEMOD_LONG);
	TM(short_buf,    TYPEMOD_SHORT);
	TM(signed_buf,   TYPEMOD_SIGNED);
	TM(unsigned_buf, TYPEMOD_UNSIGNED);
	/*
	 * don't bother with volatile, typedef, extern, static, auto,
	 * register, or inline for a type coercion
	 */
#       undef TM

	/* basic types */
#       define BT(buff, bastype)				\
	if (step(p, buff)) {					\
	    typ = concat_types(typ, bastype);			\
	    p = loc2;						\
	    if ((p >= end_coercion_name) || step(p, star_buf))	\
		break;						\
	    continue;						\
	}

	/* basic types */
	BT(char_buf,   typeof_char());
	BT(double_buf, typeof_double());
	BT(float_buf,  typeof_float());
	BT(int_buf,    typeof_int());
	BT(void_buf,   typeof_void());
#       undef BT

	/* struct and union */
#       define SU(buff, prefix)						\
	if (step(p, buff) && step(loc2, ident_buf)) {			\
	    p = loc2;							\
	    buf[0] = prefix;						\
	    strncpy(&buf[1], loc1, loc2-loc1);				\
	    buf[loc2-loc1+1] = '\0';					\
	    if ((sym = find_sym(buf)) == NULL) {			\
		return NULL;						\
	    } else {							\
		typ = concat_types(typ, sym->sym_type);			\
		if ((p < end_coercion_name) && !step(p, star_buf))	\
		    continue;						\
	    }								\
	    break;							\
	}

	/* struct and union */
	SU(struct_buf, STRUCT_PREFIX);
	SU(union_buf,  UNION_PREFIX);
#       undef SU

	/* typedefs */

	if (step(p, ident_buf)) {
	    strncpy(buf, loc1, loc2-loc1);
	    buf[loc2-loc1] = '\0';
	    if ((sym = find_sym(buf)) && is_typedef(sym)) {
		typ = concat_types(typ, typeof_specifier(sym));
	    } else {
		return NULL;
	    }
	    p = loc2;
	    if ((p >= end_coercion_name) || step(p, star_buf))
		break;
	    continue;
	}

	/* all the above matches will have done continue or break.
	 * If we're here there is nothing, probably no coercion
	 */
	return NULL;
    }

    assert(typ != NULL);
    typ = typeof_typespec(typ);
    if (p >= end_coercion_name)
	return typ;

    /* indirections */

    while(step(p, star_buf)) {
	typ = add_pointer_type(typ);
	sym = get_anonymous_type(typ);
	if (!sym->gened) {
	    store_anonymous_type(typ);
	    new_line();
	    gen_access_type(sym, FALSE);
	    sym->gened = TRUE;
	}
	typ = pointer_to_sym(sym);
	p = loc2;
	if (p >= end_coercion_name)
	    return typ;
    }

    return NULL;	/* one that gets here is "void * const" */
}

static char *
no_empty_params(params)
    char *params;
{
    if (step(params, empty_params_buf))
	return "";
    else
	return params;
}

static int
match_param_name(formal_name, body_name)
    char *formal_name, *body_name;
{
    char pattern[100];
    regex_t compiled;
    regmatch_t matches[1];

    if (body_name == NULL)
	return(0);
    sprintf(pattern, "\\<%s\\>", formal_name);
    regcomp(&compiled, pattern, 0);
    return(regexec(&compiled, body_name, 1, matches, 0));
}

#ifdef TEST_MACROS
    void
    main(argc, argv)
	int argc;
	char **argv;
    {
	macro_function_t *res;
	int i;
	char buf[100];

	init_regex();
	if (argc > 1) {
	    sprintf(buf, argv[1]);
	    for(i=2; i<argc; i++) {
		strcat(buf, " ");
		strcat(buf, argv[i]);
	    }
	    res = grok_macro_function(buf);
	    if (res != NULL)
		print(res);
	} else {
	    res = grok_macro_function(
		     " func (val1, p1, p11, 1+p1+2, p2, p22, val2)");
	    if (res != NULL)
		print(res);
	    res = grok_macro_function(" func2 (val1, p1+1+p2, p2, val2)");
	    if (res != NULL)
		print(res);
	}
    }
#endif
