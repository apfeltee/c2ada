/* $Source: /home/CVSROOT/c2ada/order.c,v $ */
/* $Revision: 1.1.1.1 $  $Date: 1999/02/02 12:01:51 $ */

#include "boolean.h"
#include "il.h"
#include "units.h"
#include "order.h"
#include "files.h"


/* TBD: we're commandeering an unused bit, which should be renamed */
#define DONE_MARK(sym) ((sym)->emitted)



static void undone_type_requisites(symbols_t   syms,
				   typeinfo_pt type,
				   unit_n      unit );

static void undone_node_requisites(symbols_t   syms,
				   node_pt     node,
				   unit_n      unit );

boolean 
sym_done(symbol_pt sym)
{
    if (DONE_MARK(sym)) return TRUE;
    if (sym->intrinsic) return TRUE;
    switch (sym->sym_kind) {
    case enum_literal:
	return sym_done(sym->sym_type->type_base);
    case param_symbol:
	return TRUE;
    default:
	return FALSE;
    }
}


static void
undone_sym( symbols_t syms, symbol_pt sym, unit_n unit )
{
    if (pos_unit(sym->sym_def) != unit) return;
    if (sym_done(sym))                  return;
    symset_add(syms, sym);
}

static void
undone_sym_requisites( symbols_t syms, symbol_pt sym, unit_n unit )
{
    switch (sym->sym_kind) {

    case type_symbol:
	/* type requisites */
	undone_type_requisites( syms, sym->sym_type, unit);
	break;

    case func_symbol:
	/* return type */
	undone_type_requisites( syms, sym->sym_type, unit);
	/* parameter types */
	/* (TBD) ? body dependencies */
	break;

    case param_symbol:
	/* type requisites */
	undone_type_requisites( syms, sym->sym_type, unit);

    case var_symbol:
	/* type requisites */
	undone_type_requisites( syms, sym->sym_type, unit);

	/* initializer value requisites */
	if (sym->has_initializer) {
	    undone_node_requisites( syms,
				       sym->sym_value.initializer,
				       unit );
	}
	break;

    case enum_literal:
	/* (no additional requisites) */
	break;

    case pkg_symbol:
	/* argument (type sym) requisites */
	/* TBD */
	break;
    }
}

static void
undone_type_requisites( symbols_t syms, typeinfo_pt type, unit_n unit)
{
    if (!type) return;

    /* TBD what about requisites of type->type_base symbol? */
    if (type->type_base) {
	undone_sym(syms, type->type_base, unit);
	return;
    }

    switch (type->type_kind) {

    case pointer_to:
	/* type_next requisites */
	undone_type_requisites( syms, type->type_next, unit );
	break;

    case array_of:
	/* type_next requisites */
	undone_type_requisites( syms, type->type_next, unit );
	break;

    case struct_of:
    case union_of:
	/* field type requisites */
	{
	    /* n
	    symbol_pt tag;
	    for (tag=type->typesym->sym_tags; tag; tag=tag->sym_parse_list) {
		undone_type_requisites( syms, tag->sym_type. unit );
	    }
	    */
	}
	break;

    case field_type:
	/* (no additional requisites?) */
	break;
    case int_type:
	/* (no additional requisties?) */
	break;
    case float_type:
	/* (no additional requisites?) */
	break;
    case void_type:
	/* (no additioanl requisites?) */
	break;

    case function_type:
	/* return type */
	undone_type_requisites(syms, type->type_next, unit);

	/* argument types */
	{
	    symbol_pt arg;
	    for (arg=type->type_info.formals; arg; arg=arg->sym_parse_list) {
		undone_type_requisites(syms, arg->sym_type, unit);
	    }
	    break;
	}

    case enum_type:
	/* (no additional requisites) */
	break;

    case typemodifier:
	/* (case shouldn't occur?) */
	/* type_next requisites */
	undone_type_requisites(syms, type->type_next, unit);
	break;
    }
    return;
}

void
undone_node_requisites( symbols_t syms, node_pt node, unit_n unit )
{
    if (!node) return;
    switch (node->node_kind) {
    case _Error:
	/* raise exception */
    case _Ellipsis:
    case _FP_Number:         
    case _Int_Number:        
    case _Char_Lit:
	return;

    case _Type:
	/* undone_type ( node->node.typ) */
	undone_type_requisites(syms, node->node.typ, unit);
	break;

    case _Sym:
	/* node->node.sym */
	undone_sym(syms, node->node.sym, unit);
	break;

    case _Ident:
	break;

    case _Macro_ID:
	/* TBD */
	break;

    case _String:
	break;
    
    /* binary operators */
    case _List:
    case _Comma:
    case _Bit_Field:
    case _Dot_Selected:
    case _Arrow_Selected:
    case _Array_Index:
    case _Func_Call:
    case _Type_Cast:
    case _Assign:
    case _Mul_Assign:
    case _Div_Assign:
    case _Mod_Assign:
    case _Add_Assign:
    case _Sub_Assign:
    case _Shl_Assign:
    case _Shr_Assign:
    case _Band_Assign:
    case _Xor_Assign:
    case _Bor_Assign:
    case _Eq:
    case _Ne:
    case _Lt:
    case _Le:
    case _Gt:
    case _Ge:
    case _Land:
    case _Lor:
    case _Band:
    case _Bor:
    case _Xor:
    case _Add:
    case _Sub:
    case _Mul:
    case _Div:
    case _Rem:
    case _Shl:
    case _Shr:						/* last binary */
    case _Exp:      /* exponentiation, Ada only: binary */
	undone_node_requisites(syms, node->node.binary.l, unit);
	undone_node_requisites(syms, node->node.binary.r, unit);
	break;

    /* unary operators */
    case _Sizeof:					/* first unary */
    case _Pre_Inc:
    case _Pre_Dec:
    case _Post_Inc:
    case _Post_Dec:
    case _Addrof:
    case _Unary_Plus:
    case _Unary_Minus:
    case _Ones_Complement:
    case _Not:
    case _Aggregate:
    case _Indirect:	
    case bool:
    case _UnBool:
    case _Char_to_Int:
    case _Int_to_Char:
	/* node->node.unary */
	undone_node_requisites(syms, node->node.unary, unit);
	break;

    case _Cond:
	/* node->cond.{bool,tru,fals} */
	undone_node_requisites(syms, node->node.cond.bool, unit);
	undone_node_requisites(syms, node->node.cond.tru,  unit);
	undone_node_requisites(syms, node->node.cond.fals, unit);
	break;

    default: break;
	/* error */
    }
}

/* TBD: can we assume that fix_stmt has enqueued all the necessary
 * symbols (types)?  After all, no forward refs in C.
 */

symbols_t
undone_requisites( symbol_pt sym )
{
    symbols_t syms;
    unit_n   unit  = pos_unit(sym->sym_def);

    syms = get_undone_requisites(sym);
    if (syms) {
	/* update the current requisites */
	symset_filter_undone(syms);
    } else {
	syms = new_symbols_set();
	undone_sym_requisites(syms, sym, unit);
    }
    set_undone_requisites(sym, syms);
    return syms;
}

boolean
has_undone_requisites(symbol_pt sym)
{
    symbols_t syms = undone_requisites(sym);
    boolean   result = symset_size(syms)>0;

    /* TBD: free reference to syms */
    return result;
}


void
set_symbol_done( symbol_pt sym )
{
    DONE_MARK(sym) = TRUE;
}


void
postpone_doing( symbol_pt sym )
{
    /* TBD: just a placeholder at the moment */
}
