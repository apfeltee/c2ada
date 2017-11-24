/*
 * Functions to generate anonymous Ada types
 */

#include <assert.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "errors.h"
#include "host.h"
#include "files.h"
#include "hash.h"
#include "il.h"
#include "nodeop.h"
#include "allocate.h"
#include "types.h"
#include "stab.h"
#include "ada_name.h"
#include "anonymous.h"
#include "gen.h"
#include "units.h"
#include "ada_types.h"

extern int auto_package;
extern file_pos_t yypos;

#undef TYPE_HASH_MAX
#define TYPE_HASH_MAX	256		/* Anonymous type hash table size */

#undef NULL
#define NULL			0

static typeinfo_t *anonymous_types[TYPE_HASH_MAX];
static int anonymous_ord[MAX_UNIQ_FNAMES];

symbol_t *anonymous_function_pointer;

/*
 * Use ordinal for anonymous type names
 */
int
next_anonymous_ord()
{
    unit_n unit = pos_unit(yypos);
    return anonymous_ord[unit]++;
}

char *
predef_name(char * s)
{
    static char buf[100];

    sprintf(buf, "%s.%s", predef_pkg, s);
    return buf;
}

char *
predef_name_copy(char * s)
    /* like predef_name() but allocates a new copy of the string */
{
    return new_string(predef_name(s));
}

/*
 * don't generate names ending in _t_t
 */
static int
ends_in_t(str)
	char *str;
{
	for (; *str; str++) {
		if (str[0] == '_' && str[1] == 't' && str[2] == 0) {
			return 1;
		}
	}
	return 0;
}

/*
 * look for type in Anonymous type hash table
 */
typeinfo_pt
find_anonymous_type(typeinfo_pt typ)
{
    typeinfo_pt t;

    assert(typ != NULL);
    assert(typ->type_hash != 0);

    for (t = anonymous_types[typ->type_hash & (TYPE_HASH_MAX-1)];
	 t; t = t->type_anonymous_list) {

	if (same_ada_type(t, typ)) {

	    symbol_pt tsym = t->type_base;
	    symbol_pt rsym;

	    assert(tsym);
	    if (t->type_kind != pointer_to) return t;
	    if (tsym->intrinsic) return t;
	    if (pos_in_current_unit(tsym->sym_def)) return t;

	    /* t->type_kind == pointer_to */
	    rsym = t->type_next->type_base;
	    if (rsym && pos_unit(rsym->sym_def)==pos_unit(tsym->sym_def)) {
		return t;
	    }
	}
    }

    return NULL;
}

/*
 * add type to Anonymous type hash table
 */
void
store_anonymous_type(typ)
	typeinfo_t *typ;
{
	int index;
	index = typ->type_hash & (TYPE_HASH_MAX-1);
	typ->type_anonymous_list = anonymous_types[index];
	anonymous_types[index] = typ;
}

static void
define_anon_type( typeinfo_pt type,
		  char *      id,
		  char *      ada_name,
		  boolean     gened )
{
    symbol_pt     basetype;

    basetype = new_sym();
    basetype->intrinsic = TRUE;
    basetype->sym_kind = type_symbol;
    basetype->sym_type = type;
    basetype->sym_ident = new_node(_Ident, id);
    basetype->sym_ada_name = ada_name;
    basetype->gened = gened;

    type->_anonymous = TRUE;
    type->type_base = basetype;
    store_anonymous_type(type);

}

static typeinfo_pt
add_const_pointer_type(typeinfo_pt typ)
{
    typ = concat_types( typeof_typemod(TYPEMOD_CONST), typ );
    typ = typeof_typespec( typ );
    typ = add_pointer_type(typ);
    typ = typeof_typespec( typ );
    return typ;
}


/*
 * Define some builtin/intrinsic anonymous types
 */
void
init_anonymous_types(void)
{
    static boolean initialized = FALSE;
    typeinfo_t *typ;


    if (initialized) return;
    initialized = TRUE;

    /* (char *) */
    typ = add_pointer_type(typeof_char());
    define_anon_type(typ,
		     "%% builtin (char*) %%",
		     predef_name_copy("charp"),
		     TRUE);

    /* (const char *) */
    typ = add_const_pointer_type( typeof_char() );
    define_anon_type(typ,
		     "%% (const char *) %%",
		     predef_name_copy("const_charp"),
		     TRUE);

    /* (void *) */
    define_anon_type(add_pointer_type(typeof_void()),
		     "%% builtin (void*) %%",
		     "System.Address",
		     TRUE);

    /* (const void *) */
    define_anon_type(add_const_pointer_type(typeof_void()),
		     "%% builtin (const void*) %%",
		     "System.Address",
		     TRUE);


    /* void (*)(void) */
    typ = add_pointer_type(add_function_type(typeof_void()));
    define_anon_type(typ,
		     "%% void (*)() %%",
		     predef_name_copy("function_pointer"),
		     TRUE);
    anonymous_function_pointer = typ->type_base;

    /* char[] */
    typ = typeof_char_array();
    define_anon_type(typ,
		     "%% char[] *%%",
		     predef_name_copy("char_array"),
		     TRUE);

}

/*
 * Here we go.  We may need to generate an anonymous type
 * for the input type.  An example is when "int *a;" is
 * encountered we'll need to gen "type a_int_t is ...".
 */
symbol_t*
get_anonymous_type(typeinfo_pt typ)
{
    typeinfo_pt  anonymous_type;
    symbol_pt    basetype;
    ident_case_t icase;
    char         buf[512];
    boolean      type_is_func_ptr = is_function_pointer(typ);
    boolean      private = FALSE;

    assert(typ != NULL);

    if (typ->type_next && !typ->type_next->type_base) {

	if (!type_is_func_ptr) {
            exit(1);
	    /*fatal(file_name(yypos), line_number(yypos),
		  "typedef ** ?", __FILE__, __LINE__);
	    */
	}
    }

    assert(typ->type_hash != 0);
    assert(typ->type_anonymous_list == NULL);

    anonymous_type = find_anonymous_type(typ);

    if (anonymous_type) {


	basetype = anonymous_type->type_base;
	assert(basetype != NULL);
	typ->type_base = basetype;
	return basetype;

#if 0
	if ( typ->type_kind != pointer_to ||
	     basetype->intrinsic ||
	     in_current_unti(basetype->sym_def) ) {

	    typ->type_base = basetype;
	    return basetype;
	} else {
	    /* type->type_kind == pointer_to */
	    symbol_pt rsym = typ->type_next->type_base;
	    if (rsym && FILE_ORD(rsym->sym_def)==FILE_ORD(basetype->sym_def)) {
		return typ->type_base = basetype;
	    }
	}
#endif


    }

    if (type_is_func_ptr) {

	sprintf(buf, "anon%d_func_access", next_anonymous_ord());

    } else if (typ->type_kind == pointer_to) {

	typeinfo_pt rtyp;
	symbol_pt   rsym;


	rtyp = typ->type_next;
	assert(rtyp != NULL);

	rsym = rtyp->type_base;
	assert(rsym != NULL);
	private = rsym->private;

	/* assert(rsym->gened); */
	/* might not be true any more if incomplete struct */

	if (rsym->sym_ada_name == NULL) {

	    assert(rsym->sym_ident != NULL);
	    assert(rsym->sym_ident->node_kind == _Ident);
	    assert(rsym->sym_ident->node.id.name != NULL);
	    rsym->sym_ada_name =
		ada_name(rsym->sym_ident->node.id.name,
			 pos_unit(rsym->sym_def));
	}

	/* synthesize type name in <buf> */
	if (equal_types(rtyp, rsym->sym_type)) {
	    strcpy(buf, tail(rsym->sym_ada_name));
	} else {
	    strcpy(buf, tail(type_nameof(rtyp, FALSE, FALSE)));
	}

	strcat(buf, rtyp->_constant? "_const_access" : "_access");

	icase = id_case(rsym->sym_ada_name);
	/* if (! ends_in_t(rsym->sym_ada_name)) { */
	/* 	strcat(buf, "_t"); */
	/* } */
	id_format(buf, icase);

    } else if (typ->type_kind == array_of) {

	char *p;
	int ndim = num_dimensions(typ);

	assert(ndim >= 1);
	if (ndim == 1) {
	    sprintf(buf, "%s_array", type_nameof(typ->type_next, 0, 0));
	} else {
	    typeinfo_t *basetyp;
	    int i;

	    for(i=0, basetyp=typ; i<ndim; i++) {
		assert(basetyp->type_kind == array_of);
		basetyp = basetyp->type_next;
	    }
	    assert(basetyp->type_kind != array_of);
	    sprintf(buf, "%s_%dd_array",
		    type_nameof(basetyp, 0, 0), ndim);
	}
	p = strrchr(buf, '.');
	if(p != NULL)
	    strcpy(buf, p+1);
    } else {
	sprintf(buf, "anonymous%d_t", next_anonymous_ord());
    }

    basetype = new_sym();
    typ->type_base = basetype;

    typ = copy_type(typ);
    typ->_anonymous = 1;

    store_anonymous_type(typ);

    basetype->sym_kind = type_symbol;
    basetype->sym_type = typ;
    basetype->sym_def = yypos;

    if (type_is_func_ptr) {
	basetype->sym_tags = typ->type_info.formals;
    }

    basetype->sym_ident = new_node(_Ident, new_string(buf));
    basetype->sym_ada_name =
	ada_name(basetype->sym_ident->node.id.name, pos_unit(yypos));
    basetype->private = private;

    return basetype;
}
