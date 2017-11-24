/* $Source: /home/CVSROOT/c2ada/package.c,v $ */
/* $Revision: 1.2 $  $Date: 1999/02/03 19:45:04 $ */

#include <assert.h>

#include "il.h"
#include "types.h"
#include "anonymous.h"
#include "allocate.h"
#include "stab.h"
#include "ada_name.h"
#include "package.h"
#include "format.h"
#include "errors.h"
#include "gen.h"
#include "units.h"

/* Generic package types & routines */

#if 0
/* For when we want to get really "class'ic" */
typedef struct {
    void   (*gen_pkg_def_func)(pkg_def_pt, symbol_pt sym, int indent);
} pkg_class_t, *pkg_class_pt;


typedef struct pkg_def_t {
    pkg_class_pt class;
} pkg_def_t;
pkg_class_t Pointers_pkg = {gen_ptrs_pkg_def};
#endif

typedef enum {
    Pointers_pkg
} pkg_kind_t;

typedef struct pkg_def_t {
    pkg_kind_t  kind;
    pkg_def_pt  next;
    int         unit;
    symbol_pt   sym;
} pkg_def_t;

struct {
   pkg_def_pt  head;
   pkg_def_pt  tail;
} pkg_list;

void remember( pkg_def_pt pkg )
{


    if (pkg_list.tail) {
	pkg_list.tail->next = pkg;
    } else {
	pkg_list.head = pkg;
    }
    pkg_list.tail = pkg;
}



/* An extension of pkgdef for an instantiation of the
 * Pointers generic package.
 * (We'll synthesize the 0 type).
 */

char *
generic_ptrs_pkg_name(boolean const_ptr)
{
    static char * all_name;   /* for "access all T" pointers */
    static char * const_name; /* for "access const T" pointers */
    if (!all_name) {
	all_name = predef_name_copy("Pointers");
	const_name = predef_name_copy("Const_Pointers");
    }
    return const_ptr? const_name : all_name;
}



typedef struct {
    pkg_def_t     pkg;

    /* The types used in the generic instantiation */
    typeinfo_pt	  element;
    typeinfo_pt	  pointer;

} ptrs_pkg_def_t, *ptrs_pkg_def_pt;

static ptrs_pkg_def_pt
new_ptrs_pkg_def(void)
{
    ptrs_pkg_def_pt pkg = allocate(sizeof(ptrs_pkg_def_t));
    pkg->pkg.kind = Pointers_pkg;
    return pkg;
}

typeinfo_pt
new_ptrs_type( typeinfo_pt element,
	       typeinfo_pt pointer,
	       ctxt_pt     ctxt,
	       file_pos_t  pos)
{
    int unit = current_unit();
    ptrs_pkg_def_pt p = new_ptrs_pkg_def();
    symbol_pt pkg_sym, sym;
    char * pkg_name0 = new_strf("%s_pointers",
			       tail(element->type_base->sym_ada_name));

    p->element = element;
    p->pointer = pointer;

    /* Create symbol for package */
    /* TBD: this is generic to all package defs */
    set_current_scope( 0 );
    pkg_sym = sym = new_sym();
    sym->sym_def  = pos;
    sym->sym_kind = pkg_symbol;
    sym->sym_ada_name = ada_name( pkg_name0, pos_unit(pos) );
    sym->sym_value.pkg_def = &p->pkg;

    p->pkg.sym = sym;
    p->pkg.unit = unit;
    remember(&p->pkg);

    if (pointer->type_next->_constant) {
	with_c_const_pointers(unit);
    } else {
	with_c_pointers(unit);
    }

    deallocate(pkg_name0);

    return p->pointer;
}

typeinfo_pt
ptrs_type_for( typeinfo_pt pointer,
	       ctxt_pt     ctxt,
	       file_pos_t  pos)
{
    typeinfo_pt element = pointer->type_next;
    pkg_def_pt  pkg;
    int         unit = current_unit();

    /* check if there's already an appropriate package in scope */
    for (pkg = pkg_list.head; pkg; pkg = pkg->next) {
	if (pkg->unit == unit && pkg->kind == Pointers_pkg) {
	    ptrs_pkg_def_pt p = (ptrs_pkg_def_pt) pkg;
	    if (equal_types(pointer, p->pointer)) {
		return p->pointer;
	    }
	}
    }

    return new_ptrs_type(element, pointer, ctxt, pos);
}


void
gen_ptrs_pkg_def( ptrs_pkg_def_pt pkg, symbol_pt sym, int indent )
{
    char * element_type_name =
	new_string(type_nameof(pkg->element, FALSE, FALSE));
    char * pointer_type_name = type_nameof(pkg->pointer, FALSE, FALSE);
    boolean is_const_ptr = pkg->pointer->type_next->_constant;


    putf( "%>package %s is\n", indent, sym->sym_ada_name );
    putf( "%>new %s(\n", indent+4, generic_ptrs_pkg_name(is_const_ptr) );
    putf( "%>Element => %s,\n", indent+8, element_type_name );
    putf( "%>Pointer => %s);\n", indent+8, pointer_type_name );

    deallocate(element_type_name);

    /* TBD: maybe we should use sym_ada_name of Pointers directly */
    putf("%>use %s;\n", indent, sym->sym_ada_name);
    putf("\n");
}

void
gen_pkg_def( symbol_pt sym, int indent )
{
    pkg_def_pt pkg_def;
    assert(sym->sym_kind==pkg_symbol);
    pkg_def = sym->sym_value.pkg_def;
    switch (pkg_def->kind) {
    case Pointers_pkg:
	gen_ptrs_pkg_def( (ptrs_pkg_def_pt)pkg_def, sym, indent);
    default: break;
    }
}

void
gen_unit_pkg_defs( int unit, int indent )
{
    pkg_def_pt pkg;
    for (pkg = pkg_list.head; pkg; pkg = pkg->next) {
	if (pkg->unit == unit) {
	    gen_pkg_def(pkg->sym, indent);
	}
    }
}
    

    

    
	 
	 
