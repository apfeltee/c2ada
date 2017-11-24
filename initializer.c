/* $Source: /home/CVSROOT/c2ada/initializer.c,v $ */
/* $Revision: 1.2 $  $Date: 1999/02/03 19:45:04 $ */

/* Routines concerned with fixing and generating variable initializers */

#include <assert.h>

#include "il.h"
#include "errors.h"
#include "stmt.h"
#include "gen.h"
#include "types.h"
#include "gen_expr.h"
#include "fix_stmt.h"
#include "format.h"
#include "anonymous.h"
#include "initializer.h"

void
gen_zero(typeinfo_pt t)
    /* generate a 0 literal of the appropriate type */
{

    static char *predef_nul;
    if (!predef_nul) predef_nul = predef_name_copy("Nul");

    switch (t->type_kind) {
    case pointer_to:
	put_string( null_pointer_value_name(t) );
	break;
    case int_type:
	put_string(equal_types(t,type_char())? predef_nul : "0"); break;
    case float_type:
	put_string("0.0"); break;
    case array_of:
	put_string("(others => ");
	gen_zero(t->type_next);
	put_string(")");
	break;
    case struct_of:
	{
	    symbol_pt tsym = t->type_base;
	    symbol_pt fields = tsym->sym_tags;
	    typeinfo_pt t1;
	    boolean first = TRUE;
	    put_string("(");
	    for (fields = tsym->sym_tags;
		 fields;
		 fields=fields->sym_parse_list) {

		if (first && !fields->sym_parse_list) {
		    /* 1-field structs require named component association */
		    putf("%s => ", fields->sym_ada_name);
		}
		first = FALSE;
		
		t1 = fields->sym_type;
		gen_zero(t1);
		if (fields->sym_parse_list) put_string(", ");
	    }
	    put_string(")");
	}
	break;

    case enum_type:
	/* TBD: requires unchecked conversion */
    case union_of:
    case field_type:
    default:
	put_string("{MISSING 0 INITIALIZER}"); break;
    }
} /* gen_zero */

static  boolean
is_string_lit(node_pt e)
{
    if (e->node_kind==_String) return TRUE;
    if (e->node_kind==_Type_Cast &&
	e->node.binary.r->node_kind==_String &&
	e->node.binary.l->node.typ->type_kind==array_of) return TRUE;
    return FALSE;
}

static node_pt
gen_agg_item( typeinfo_pt t, node_pt e)
{
    if (e->node_kind==_List) {
	switch (t->type_kind) {
	case array_of:
	    if (is_string_lit(e->node.binary.l) &&
		equal_types( t->type_next, type_char() )) {
		    
		gen_initializer( t, e->node.binary.l );
		return e->node.binary.r;
	    }
	    /* else FALL THROUGH */
	case struct_of:
	case union_of:
	    if (e->node.binary.l->node_kind==_Aggregate) {
		gen_initializer( t, e->node.binary.l );
		return e->node.binary.r;
	    } else {
		/* aggregate types consume the head of the list */
		return gen_initializer(t, e);
	    } 
	default:
	    gen_initializer( t, e->node.binary.l );
	    return e->node.binary.r;
	}
    } else {
	gen_initializer( t, e );
	return 0;
    }
}


node_pt
gen_initializer(typeinfo_pt t, node_pt e)
    /* Output an explicit initializer */
{
    switch (t->type_kind) {
    case int_type:
    case float_type:
    case pointer_to:
	/* TBD: check type? */
	/* TBD: ignore braces */
	gen_expr( e, FALSE );
	return 0;

    case array_of: {
	typeinfo_pt tsub = t->type_next;   /* component type */
	node_pt enext;
	int n;
	int size = t->type_info.array.elements;
	int list_indent;
 	boolean singleton;

	/* 
	 * An array of character type may be initialized by a
	 * character string literal, optionally enclosed in braces.
	 * TBD: more general test for "character type".
	 */
	if (equal_types(tsub,type_char())) {

	    if (is_string_lit(e)) {
		gen_expr( e, FALSE );
		return 0;
	    }
	    if (e->node_kind==_Aggregate &&
		is_string_lit(e->node.unary)) {

		gen_expr( e->node.unary, FALSE );
		return 0;
	    }
	}

	if (e->node_kind==_Aggregate) {
	    enext = e->node.unary;
	    singleton = !(enext->node_kind==_List && enext->node.binary.r);
	} else {
	    enext = e;
	    assert(size >= 0);
	    singleton = FALSE;
	}

	put_string("(");
	list_indent = cur_indent();
	if (singleton && size==-1) putf("0 => ");
	for (n=0; (size==-1 || n<size) && enext; n++) {
	    if (n>0) {
		assert( !singleton );
		putf(",\n%>", list_indent);
	    }
	    enext = gen_agg_item( tsub, enext );
	}
	    
	if (n<size) {
	    putf(",\n%>others => ", list_indent);
	    gen_zero(tsub);
	}
	put_string(")");
	return enext;

    } /* case array_of */

    case struct_of: {
	node_pt enext;
	symbol_pt fnext ;  /* next field */
	boolean first = TRUE;

	if (e->node_kind==_Aggregate) {
	    enext = e->node.unary;
	} else {
	    enext = e;
	}

	put_string("( ");

	for (fnext=t->type_base->sym_tags;
	     fnext;
	     fnext =fnext->sym_parse_list){

	    if (first) {
		first=FALSE;
		if (!fnext->sym_parse_list) {
		    /* We must use "named component association"
		     * to initialize 1-element structures
		     */
		    putf("%s => ", fnext->sym_ada_name);
		}
	    } else {
		put_string(", ");
	    }		

	    if (enext) {
		enext = gen_agg_item( fnext->sym_type, enext );
	    } else {
		gen_zero( fnext->sym_type );
	    }
	}

	put_string(" )");

	if (e->node_kind==_Aggregate) {
	    return 0;
	} else {
	    return enext;
	}

    } /* case struct_of */

    default:
	put_string("{UNIMPLEMENTED INITIALIZATION}");
	return NULL;

    } /* switch */

} /* gen_initializer() */



static node_pt * fix_initializer(typeinfo_pt t, node_pt *ep);

static node_pt *
fix_agg_item( typeinfo_pt t, node_pt *ep)
{
    node_pt e = *ep;
    if (e->node_kind==_List) {
	switch (t->type_kind) {
	case array_of:
	    if (e->node.binary.l->node_kind==_String &&
		equal_types( t->type_next, type_char() )) {

		fix_initializer( t, &e->node.binary.l );
		return &e->node.binary.r;
	    }
	    /* else FALL THROUGH */
	case struct_of:
	case union_of:
	    if (e->node.binary.l->node_kind==_Aggregate) {
		fix_initializer( t, &e->node.binary.l );
		return &e->node.binary.r;
	    } else {
		/* aggregate types consume the head of the list */
		return fix_initializer(t,ep);
	    } 
	default:
	    fix_initializer( t, &e->node.binary.l );
	    return &e->node.binary.r;
	}
    } else {
	fix_initializer( t, ep );
	return 0;
    }
}


static node_pt *
fix_initializer( typeinfo_pt t, node_pt *ep )
{
    node_pt e = *ep;

    switch (t->type_kind) {
    case int_type:
    case float_type:
    case pointer_to:
	*ep = fix_initializer_expr(e, t);
	return 0;

    case array_of: {
	typeinfo_pt tsub = t->type_next;   /* component type */
	node_pt enext, *ep_next;
	int n;
	int size = t->type_info.array.elements;

	/* 
	 * An array of character type may be initialized by a
	 * character string literal, optionally enclosed in braces.
	 * TBD: more general test for "character type".
	 */
	if (equal_types(tsub,type_char())) {

	    if (e->node_kind==_String) {
		*ep = fix_initializer_expr(e, t);
		return 0;
	    }

	    if (e->node_kind == _Aggregate &&
		e->node.unary->node_kind==_String) {

		*ep = fix_initializer_expr(e->node.unary, t);
		return 0;
	    }
	}
	    
	if (e->node_kind==_Aggregate) {
	    ep_next = &e->node.unary;
	} else {
	    ep_next = ep;
	    assert(size >= 0);
	}
	enext = *ep_next;

	for (n=0; (size==-1 || n<size) && ep_next; n++) {
	    ep_next = fix_agg_item( tsub, ep_next );
	}
	    
	return ep_next;

    } /* case array_of */

    case struct_of: {
	node_pt *ep_next;
	symbol_pt fnext ;  /* next field */


	if (e->node_kind==_Aggregate) {
	    ep_next = &e->node.unary;
	} else {
	    ep_next = ep;
	}

	for (fnext=t->type_base->sym_tags;
	     fnext;
	     fnext =fnext->sym_parse_list){

	    if (ep_next) {
		ep_next = fix_agg_item( fnext->sym_type, ep_next );
	    
	    }

	}

	if (e->node_kind==_Aggregate) {
	    return 0;
	} else {
	    return ep_next;
	}
    } /* case struct_of */

    default:

	put_string("{UNIMPLEMENTED INITIALIZATION}");
	return NULL;
    } /* switch */
}    



void
fix_sym_initializer( symbol_pt sym )
{
    typeinfo_pt type = sym->sym_type;

    fix_initializer( type, &sym->sym_value.initializer );

} /* fix_sym_initializer() */
