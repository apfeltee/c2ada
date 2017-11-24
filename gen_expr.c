/* $Source: /home/CVSROOT/c2ada/gen_expr.c,v $ */
/* $Revision: 1.1.1.1 $  $Date: 1999/02/02 12:01:51 $ */

#include <assert.h>
#include <stdio.h>

#include "errors.h"
#include "gen_expr.h"
#include "format.h"
#include "gen.h"
#include "anonymous.h"
#include "macro.h"
#include "stmt.h"
#include "print.h"
#include "nodeop.h"
#include "types.h"
#include "units.h"


#define streq(a,b) (!strcmp(a,b))

extern char * packaged_name( char*, file_pos_t, file_pos_t );

/*
 * This module implements gen_expr, a function to print an expression
 * tree as Ada source code.  It is assumed that the expression is one
 * directly representable in Ada: an earlier phase [TBD] has the 
 * responsibility to transform an arbitrary C expression into one
 * tractable here.
 */


/* FUNCTION DISPATCHING
 * The type gen_expr_func_t is a function type for gen_expr_* functions;
 * an instance is defined for each kind of expression. The function
 * gen_expr_func (defined at the end of this module) selects the
 * appropriate function based on its argument. gen_expr then simply
 * calls gen_expr_func to find the right function, and invokes it.
 */

typedef void (*gen_expr_func_t)(node_pt e, boolean in_parens);
static gen_expr_func_t gen_expr_func( node_pt e );

static boolean dynamic_referent(node_pt e);

void
gen_expr( node_pt e, boolean in_parens )
{
    (*gen_expr_func(e))(e, in_parens);
}

/* expr_op_string:
 * return the string to be printed as the operator for an expression.
 */
static char *
expr_op_string( node_pt e)
{
    switch (e->node_kind) {

    /* binary */
    case _Assign: return ":=";
    case _Eq:   return "=";
    case _Ne:   return "/=";
    case _Lt:   return "<";
    case _Gt:   return ">";
    case _Le:   return "<=";
    case _Ge:   return ">=";
    case _Land: return "and then";
    case _Lor:  return "or else";
    case _Band: return "and";
    case _Bor:  return "or";
    case _Xor:  return "xor";
    case _Add:  return "+";
    case _Sub:  return "-";
    case _Mul:  return "*";
    case _Div:  return "/";
    case _Rem:  return "rem";  /* TBD: or mod? */
    case _Exp:  return "**";   /* Ada only, of course */
    case _Concat:return "&";   /* Ada only */
	

    /* binary functions */
    case _Shl: 
	{
	    static char * shl_funcname;
	    if (!shl_funcname) {
		shl_funcname = "Shift_left";
	    }
	    return shl_funcname;
	}
   case _Shr: 
	{
	    static char * shr_funcname;
	    if (!shr_funcname) {
		shr_funcname = "Shift_right";
	    }
	    return shr_funcname;
	}

    /* unary functions */
    case _Char_to_Int:
	{
	    static char * char_pos_funcname;
	    if (!char_pos_funcname) {
		char_pos_funcname = predef_name_copy("Char'Pos");
	    }
	    return char_pos_funcname;
	}
    case _Int_to_Char:
	{
	    static char * char_val_funcname;
	    if (!char_val_funcname) {
		char_val_funcname = predef_name_copy("Char'Val");
	    }
	    return char_val_funcname;
	}
    case bool:
	{
	    static char * toBool_funcname;
	    if (!toBool_funcname) {
		toBool_funcname = predef_name_copy("Not_Zero");
	    }
	    return toBool_funcname;
	}
    case _UnBool:
	{
	    static char * fromBool_funcname;
	    if (!fromBool_funcname) {
		fromBool_funcname = predef_name_copy("Bool_to_Int");
	    }
	    return fromBool_funcname;
	}
  
 
	
	
    /* prefix unary */
    case _Unary_Plus:  return "+";
    case _Unary_Minus: return "-";
    case _Ones_Complement: return "not ";

    /* postfix unary */
    case _Addrof:
	{
	    node_pt e1 = e->node.unary;
	    if (dynamic_referent(e1)) {

		/* Any nonstatic symbol requires special treatment */
		/* TBD: This is a solution specific to GNAT */
		return "'unrestricted_access";

	    } else {
		return "'access";
	    }
	}
    case _Indirect: return ".all";

    /* not directly mappable: */
    case _Sizeof:
    case _Not: /* right translation is type-dependent */
    case _Cond:
    default:
	return "<OPERATOR>" ;  /* TBD: return (char*)0 ? */
    }
}


static void
lparen(boolean in_parens)
{
    if (in_parens) put_char('(');
}

static void
rparen(boolean in_parens)
{
    if (in_parens) put_char(')');
}


/* Ada operator precedence enumeration. */

typedef enum {
    ap_logical,
    ap_relational,
    ap_binary_adding,
    ap_unary_adding,
    ap_multiplying,
    ap_highest
} ada_prec_t;

ada_prec_t
ada_prec( node_kind_t op )
{
    switch (op) {
    case _Mul:
    case _Div:
    case _Rem:
 /* case _Mod:  -- this & other commented are possible additional node kinds
                   to be added to represent Ada better */
	return ap_multiplying;
    case _Add:
    case _Sub:
    case _Concat:
	return ap_binary_adding;
    case _Eq:
    case _Ne:
    case _Lt:
    case _Le:
    case _Ge:
    case _Gt:
	return ap_relational;
    case _Land:
    case _Lor:
    case _Band:
    case _Bor:
    case _Xor:
	return ap_logical;
    case _Unary_Plus:
    case _Unary_Minus:
	return ap_unary_adding;
    case _Exp:
    default:
	return ap_highest;
    }
}


boolean
paren_sub( boolean right, node_pt x, node_pt xsub )
{
   /*
    * Parenthesize a binary subexpression if
    * (1) its operator is lower precedence =>
    *     (a+b)*c  vs  a*b+c
    * (2) different logical operators =>
    *     (a or b) and c  vs  a or b or c
    * (3) same precedence, but not logical & on right =>
    *     a-(b-c)  vs  a-b - c
    */

    ada_prec_t prec     = ada_prec(x->node_kind);
    ada_prec_t prec_sub = ada_prec(xsub->node_kind);

    if (prec_sub < prec) return 1;
    if (prec_sub == prec) {
	if (prec == ap_logical) {
	    return x->node_kind != xsub->node_kind;
	} else {
	    return right;
	}
    }
    return 0;
}


/* gen_expr_Binop: gen routine for infix binary operator expressions */
void
gen_expr_Binop( node_pt e, boolean in_parens)
{
    boolean bracket_left  = paren_sub(0, e, e->node.binary.l);
    boolean bracket_right = paren_sub(1, e, e->node.binary.r);

    lparen(in_parens);
    gen_expr(e->node.binary.l, bracket_left);
    put_char(' ');
    put_string( expr_op_string(e) );
    put_char(' ');
    gen_expr(e->node.binary.r, bracket_right);
    rparen(in_parens);

}

void
gen_expr_Binop_Assign( node_pt e, boolean in_parens)
    /* for binary assignment expressions */
{
    node_pt     el   = e->node.binary.l;
    node_kind_t kind = non_assign_op(e->node_kind);
    node_pt     er   = new_node(kind, el, e->node.binary.r);

    gen_expr(el, FALSE);
    put_string( " := " );
    gen_expr(er, FALSE);
}


void
gen_expr_Binop_func( node_pt e, boolean in_parens)
{
    put_string( expr_op_string(e) );  /* print name of function */
    put_string("( ");
    gen_expr( e->node.binary.l, 0 );
    put_string(", ");
    gen_expr( e->node.binary.r, 0 );
    put_string(" )");
}

void
gen_expr_Unop( node_pt e, boolean in_parens)
{
    boolean bracket_sub = ada_prec(e->node.unary->node_kind) < ap_highest;

    lparen(in_parens);
    put_string( expr_op_string(e) );
    gen_expr(e->node.unary, bracket_sub);
    rparen(in_parens);
}

void
gen_expr_Unop_postfix( node_pt e, boolean in_parens)
{
    boolean bracket_sub = ada_prec(e->node.unary->node_kind)<ap_highest;

    lparen(in_parens);
    gen_expr(e->node.unary, bracket_sub);
    put_string( expr_op_string(e) );
    rparen(in_parens);
}

void
gen_expr_Unop_func( node_pt e, boolean in_parens)
{
    put_string( expr_op_string(e) );  /* print name of function */
    put_string("( ");
    gen_expr( e->node.unary, 0);
    put_string(" )");
}

void
gen_expr_Sizeof( node_pt e, boolean in_parens)
{
    static char * sizeof_funcname;

    if (!sizeof_funcname) {
	sizeof_funcname = predef_name_copy("Sizeof");
    }

    lparen(in_parens);
    put_string( sizeof_funcname );
    lparen(TRUE);
    gen_expr(e->node.unary, FALSE);
    put_string("'Size");
    rparen(TRUE);
    rparen(in_parens);
}

void
gen_expr_Type( node_pt e, boolean in_parens )
{
    char * name;
    name = type_nameof(e->node.typ, 0, 0);

    lparen(in_parens);
    put_string( name );
    rparen(in_parens);
}


void
gen_expr_Sym( node_pt e, boolean in_parens )
{
    /* Qualify the symbol's name with its package name if it's not
     * in the current package.
     */
    symbol_pt sym = e->node.sym;
    char * qname = sym->sym_ada_name;
    extern int yypos;
    file_pos_t sym_pos = e->node.sym->sym_def;

    /* NB: The test of the symbol's pos against yypos is a way
     * of detecting symbols that were created after parsing: i.e.,
     * in the "fixup" phase.
     */
    
    if (!(sym->intrinsic || sym->_struct_or_union_member || sym_pos==yypos)) {
	qname = packaged_name(e->node.sym->sym_ada_name,
			      e->node_def,
			      sym_pos);
    }
    lparen(in_parens);
    put_string(qname);
    rparen(in_parens);
}

void
gen_expr_Ident( node_pt e, boolean in_parens )
{
    /* Identifiers should be resolved to symbols by now */
    /* TBD: set an error indicator? issue message? */
    char buffer[256];
    sprintf(buffer, "{UNRESOLVED IDENTIFIER %s}", e->node.id.name);
    put_string( buffer );
}
    

char *
null_pointer_value_name( typeinfo_pt type )
{
    symbol_pt tsym = type->type_base;

    assert(type->type_kind == pointer_to);
    if (type->type_next->type_kind == void_type) {

	return "System.Null_Address";

    } else if ( tsym && tsym->private) {

	symbol_pt sym = private_type_null( tsym );
	
	if (pos_in_current_unit(sym->sym_def)) {
	    return sym->sym_ada_name;
	} else {
	    static char buf[1024];
	    sprintf(buf, "%s.%s", 
		    unit_name(pos_unit(sym->sym_def)),
		    sym->sym_ada_name);
	    return buf;
	}
    } else {

	return "null";
    }
}
	

void
gen_expr_Macro_ID( node_pt e, boolean in_parens )
{
    char * name = e->node.macro->macro_ada_name;
    if (is_null_ptr_value(e)) {

	if (e->type->type_kind==pointer_to) {
	    name = null_pointer_value_name(e->type);
	} else {
	    /*
	     * This case could show up, e.g., where NULL
	     * is defined as 0, then used in a varargs
	     * arg list, where there's no type checking
	     */
	    name = "null";
	}

    } else {

	/* qualify the name by its source "package" */
	name = packaged_name(name,
			     e->node_def,
			     e->node.macro->macro_definition);
    }
    assert(name!=0);
    lparen(in_parens);
    put_string(name);
    rparen(in_parens);
}

    

void
gen_expr_FP_Number( node_pt e, boolean in_parens )
{
    lparen(in_parens);
    print_fp_value( e->node.fval );
    rparen(in_parens);
}

void
gen_expr_Int_Number( node_pt e, boolean in_parens )
{
    lparen(in_parens);
    if (e->type==type_boolean()) {
	put_string(e->node.ival? "true" : "false");
    } else if (e->type && e->type->type_kind==pointer_to) {
	put_string( null_pointer_value_name(e->type) );
    } else if (e->char_lit) {
	print_char_value( e->node.ival );
    } else {
	print_value( e->node.ival, e->baseval );
    }
    rparen(in_parens);
}

void
gen_expr_String( node_pt e, boolean in_parens )
{
    lparen(in_parens);
    if (e->node.str.len) {
	print_string_value( e->node.str.form, -1, !e->no_nul );
    } else {
	put_string("\"\"");
    }
    rparen(in_parens);
}

void
gen_expr_List( node_pt e, boolean in_parens )
{
    /* TBD: assuming that _List nodes used only for arg lists;
     * i.e., that comma expressions don't make it this far.
     */
    gen_expr( e->node.binary.l, 0);
    put_string(", ");
    gen_expr( e->node.binary.r, 0);
}

void
gen_expr_Selected( node_pt e, boolean in_parens )
{
    lparen(in_parens);
    gen_expr(e->node.binary.l, 0 /*tbd*/ );
    put_string(".");
    gen_expr(e->node.binary.r, 0);
    rparen(in_parens);
}

void
gen_expr_Array_Index( node_pt e, boolean in_parens )
{
    lparen(in_parens);
    gen_expr(e->node.binary.l, 0 /*tbd*/ );
    put_string("(");
    gen_expr(e->node.binary.r, 0 );
    put_string(")");
    rparen(in_parens);
}

void
gen_expr_Func_Call( node_pt e, boolean in_parens )
{
    lparen( in_parens );
    gen_expr( e->node.binary.l, 0 /* tbd */);
    if (e->node.binary.r) {
	put_string("( ");
	gen_expr( e->node.binary.r, 0 );
	put_string(" )");
    }
    rparen( in_parens );
}

static void
gen_forced_type(typeinfo_pt type, boolean to_unsigned, node_pt e)
    /* e must be converted to/from unsigned.
     * Then, if the target type is not a builtin type, or
     * not the same size as the default conversion
     * we emit a type coercion around that.
     */
{
    static char * to_unsigned_name;
    static char * to_signed_name;
    boolean coerce_after;
    boolean coerce_before;
    typeinfo_pt etype = e->type;
    char * btype_name;

    if (!to_unsigned_name) {
	to_unsigned_name = "To_unsigned";
	to_signed_name   = "To_signed";
    }
    
    coerce_after  = !type->_builtin || type->_sizeof != etype->_sizeof;

    coerce_before = etype && !etype->_builtin;
    if (coerce_before) {
	btype_name = int_type_builtin_name(etype);
    }

    if (coerce_after) putf("%s(", type_nameof(type, 0, 0));
    {
	putf("%s(", to_unsigned? to_unsigned_name : to_signed_name );
	{
	    if (coerce_before) putf("%s(", btype_name);
	    gen_expr( e, 0 );
	    if (coerce_before) putf(")");
	}
	put_string(")");
    }
    if (coerce_after) put_string(")");
}

static boolean
dynamic_referent(node_pt e)
    /* Returns TRUE unless e is known to be statically allocated */
{
    switch (e->node_kind) {
    case _Sym:
	return e->node.sym->sym_kind==param_symbol ||
	       e->node.sym->sym_scope > 1;
    case _Dot_Selected:
	return dynamic_referent(e->node.binary.l);
    case _Macro_ID:
	/* all macro values are statically allocated */
	return FALSE;
    case _Array_Index:
	return dynamic_referent(e->node.binary.l);
    default:
	/* TBD other cases can be resolved */
	return TRUE;
    }
}

void
gen_expr_Type_Cast( node_pt e, boolean in_parens )
{
    node_pt el = e->node.binary.l;
    node_pt er = e->node.binary.r;
    typeinfo_pt type = el->node.typ;
    typeinfo_pt from_type = er->type;
    boolean is_ptr_cast = type->type_kind==pointer_to;

    if (type->type_kind==array_of && er->node_kind==_String) {
	/* special-case construct used in initializing a
	 * definite-length char array with a string literal
	 */
	print_string_value(er->node.str.form,
			   type->type_info.array.elements,
			   TRUE);
	return;
    }

    assert(from_type);

    lparen(in_parens);
    if (is_ptr_cast && is_null_ptr_value(er)) {

	put_string( null_pointer_value_name(type) );

    } else if (is_ptr_cast &&
	       from_type->type_kind==array_of) {

	/*  A =>  A(A'first)'access */
	gen_expr( er, 0 );
	put_string("( ");
	gen_expr( er, 0 );
	put_string("'first )");

	if (dynamic_referent(er)) {
	    put_string("'unrestricted_access");
	} else {
	    put_string("'access");
	}

    } else if (type->type_kind==int_type &&
	       from_type->type_kind==int_type &&
	       e->baseval /* special signal for type force */ ) {

	gen_forced_type(type, type->_unsigned, er);

    } else  {

	boolean qualified = (is_ptr_cast && er->node_kind==_Addrof);
	    
	gen_expr( el, 0 );
	put_string( qualified? "'(" : "(" );
	gen_expr( er, 0 );
	put_string(")");

    }
    rparen(in_parens);
}

void
gen_expr_Assign( node_pt e, boolean in_parens )
{
    gen_expr( e->node.binary.l, 0 );
    put_string(" := ");
    gen_expr( e->node.binary.r, 0 );
}

static char *
bool_to_int_funcname(void)
{
    static char * funcname;
    if (!funcname) {
	funcname = predef_name_copy("To_Int");
    }
    return funcname;
}

void
gen_expr_Not( node_pt e, boolean in_parens )
    /* The _Not operator returns a boolean */
{
    static char * eq0_funcname;
    node_pt eSub = e->node.unary;
    typeinfo_pt subtype = eSub->type;
    if (!eq0_funcname) {
	eq0_funcname = predef_name_copy("Is_Zero");
    }

    lparen(in_parens);
    if (eSub->node_kind==bool) {
	put_string("not ");
	gen_expr(e->node.unary, FALSE);
    } else {
	assert(subtype!=NULL);
	if (subtype->type_kind==pointer_to) {
	    gen_expr(e->node.unary, FALSE);
	    put_string(" = null");
	} else {
	    put_string(eq0_funcname);
	    put_string("(");
	    gen_expr(e->node.unary, FALSE);
	    put_string(")");
	}
    }
}


void
gen_expr_Unimplemented( node_pt e, boolean in_parens )
{
    put_string("{UNIMPLEMENTED_EXPRESSION_FORM}");
    warning(file_name(e->node_def), line_number(e->node_def),
	    "unimplemented expression form (%s) in gen_expr",
	    nameof_node_kind(e->node_kind));
}



static gen_expr_func_t
gen_expr_func( node_pt e )
{
    switch (e->node_kind) {
    case _Add:
    case _Sub:
    case _Mul:
    case _Div:
    case _Rem:
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
    case _Exp:
    case _Concat:
	return gen_expr_Binop;

    case _Unary_Plus:
    case _Unary_Minus:
    case _Ones_Complement:
	return gen_expr_Unop;

    case _Not:
	return gen_expr_Not;

    case bool:
    case _UnBool:
    case _Char_to_Int:
    case _Int_to_Char:
	return gen_expr_Unop_func;

    case _Addrof:
    case _Indirect:
	return gen_expr_Unop_postfix;

    case _Sizeof:
	return gen_expr_Sizeof;

    case _FP_Number:
	return gen_expr_FP_Number;

    case _Int_Number:
	return gen_expr_Int_Number;

    case _Type:
	return gen_expr_Type;

    case _String:
	return gen_expr_String;

    case _Sym:
	return gen_expr_Sym;

    case _Ident:
	return gen_expr_Ident;

    case _Macro_ID:
	return gen_expr_Macro_ID;

    case _Dot_Selected:
    case _Arrow_Selected:
	return gen_expr_Selected;

    case _Array_Index:
	return gen_expr_Array_Index;

    case _Func_Call:
	return gen_expr_Func_Call;

    case _List:
	return gen_expr_List;

    case _Type_Cast:
	return gen_expr_Type_Cast;

    case _Assign:
	return gen_expr_Assign;

    case _Shl:
    case _Shr:
	return gen_expr_Binop_func;

    case _Bit_Field:
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
	return gen_expr_Binop_Assign;

    default:
	return gen_expr_Unimplemented;
	
    }
}
