/* $Source: /home/CVSROOT/c2ada/fix_stmt.c,v $ */
/* $Revision: 1.3 $  $Date: 1999/02/09 18:16:51 $ */

/*
 * This module contains code to transform statement and expression
 * trees to "Ada-compatible" forms that gen_stmt and gen_expr can
 * deal with.
 *
 * The way to handle a expression statement is a decision with consequences
 * in several individual routines.  That decision is to translate the
 * expression, taking account of the is_stmt argument, so that if an
 * expression is the body of a statement (hence its value is not required)
 * to return an expression nevertheless.  Most notably, an assignment
 * expression is returned as an assignment expression if it's a statement;
 * if not, a new (assignment) statement is emitted and the expression
 * returned is the left hand side of the assignment.  This treatment
 * requires that each expression statement be translated in a distinct
 * context, so that the sequencing of side-effects is maintained, and
 * this is what happens in fix_statement_itself.
 *
 */

#include <assert.h>
#include <stdlib.h>

#include "hostinfo.h"
#include "errors.h"
#include "context.h"
#include "fix_stmt.h"
#include "types.h"
#include "nodeop.h"
#include "stmt.h"
#include "macro.h"
#include "localfunc.h"
#include "print.h"
#include "allocate.h"
#include "stab.h"
#include "aux_decls.h"
#include "units.h"
#include "package.h"
#include "anonymous.h"
#include "ada_types.h"

#include "gen.h"

#include <string.h>

/* a string equality operator */
#define streq(a,b) (!(strcmp(a,b)))

typedef int usage_flags;
typedef enum {
   Is_bool,
   Is_stmt,
   Is_called,
   Is_derefd
} usage_flag;

#define USE(flag) (1<<flag)

#define USAGE(flag) (usage&USE(flag))

/* Set FALSE by external calls to fix_func_body,
 *     TRUE  by          calls to fix_initializer_expr */
boolean in_initializer = FALSE;       

/* forward references */
static typeinfo_pt type_of(node_pt);  
static stmt_pt fix_stmt( stmt_pt stmt, ctxt_pt ctxt );
static node_pt fix_expr( node_pt expr, ctxt_pt ctxt, usage_flags usage );
static void record_type_usage( node_pt e);

/* To make sure that created statements are properly scoped */
static stmt_pt
new_scoped_stmt_Expr(scope_id_t scope, node_pt e)
{
    set_current_scope( scope );
    return new_stmt_Expr( e );
}

static typeinfo_pt
type_univ_int( void )
    /* TBD: should go into types module */
{
    /* TBD: need distinctive universal type */
    return type_int();
}

static typeinfo_pt
type_natural( void )
    /* return a type type to represent Ada Standard.Natural */
{
    static typeinfo_pt natural;
    static symbol_pt   sym;
    
    if (!natural) {

	natural = typeof_int();

	sym = new_sym();
	sym->sym_kind = type_symbol;
	sym->intrinsic = TRUE;
	sym->sym_ada_name = "Natural";
	sym->gened = TRUE;
	sym->sym_type = natural;

	natural->type_base = sym;
	
    }
    return natural;
}

static typeinfo_pt
type_ptrdiff( void )
    /* return a type to represent C's ptrdiff_t type,
     * which is Interfaces.C.ptrdiff_t in Ada.
     */
{
    static typeinfo_pt ptrdiff;
    static symbol_pt sym;

    if (!ptrdiff) {
	
	ptrdiff = typeof_int();
	
	sym = new_sym();
	sym->sym_kind = type_symbol;
	sym->intrinsic = TRUE;
	sym->sym_ada_name = predef_name_copy("ptrdiff_t");
	sym->gened = TRUE;
	sym->sym_type = ptrdiff;

	ptrdiff->type_base = sym;
	
    }
    return ptrdiff;
}

static typeinfo_pt
type_size_t_( void )
    /* return a type to represent C's size_t type,
     * which is Interfaces.C.size_t in Ada.
     */
{
    static typeinfo_pt size_t_;
    static symbol_pt sym;

    if (!size_t_) {
	
	size_t_ = typeof_int();
	
	sym = new_sym();
	sym->sym_kind = type_symbol;
	sym->intrinsic = TRUE;
	sym->sym_ada_name = predef_name_copy("size_t");
	sym->gened = TRUE;
	sym->sym_type = size_t_;

	size_t_->type_base = sym;
	
    }
    return size_t_;
}

static symbol_pt
predef_new_string_func(void)
{
    static symbol_pt sym;   /* function symbol */
    
    if (!sym) {
	typeinfo_pt type;
	symbol_pt   parm;
	
	sym = new_sym();
	sym->sym_kind = func_symbol /*tbd*/;
	sym->intrinsic = TRUE;
	sym->sym_ada_name = predef_name_copy("New_string");
	sym->gened = TRUE;

	/* parameter symbol */
	parm = new_sym();
	parm->sym_type = type_string();

	/* TBD: add parameter types */
	sym->sym_type = type = add_function_type( type_string() );
	type->type_info.formals = parm;
	

    }
    return sym;
}


static node_pt
zero(void)
{
    return new_node(_Int_Number, 0, 10);
}

static node_pt
one(void)
{
    return new_node(_Int_Number, 1, 10);
}

static node_pt
two(void)
{
    return new_node(_Int_Number, 2, 10);
}

static node_pt
null(typeinfo_pt t)
{
    node_pt result = new_node(_Int_Number, 0, 10);
    result->type = t;
    return result;
}

static node_pt
value_boolean(boolean v)
{
    node_pt result = v? one() : zero();
    result->type = type_boolean();
    return result;
}

static symbol_pt
new_tmp_var( typeinfo_pt type, file_pos_t pos )
    /* make up a new tmp var */
{
    node_pt id =
	new_pos_node(pos, _Ident, new_string("tmp_var"));  /*TBD:generate*/
    symbol_pt sym = var_declaration(type, id);
    sym->sym_def = pos;
    gen_ada_var(sym);
    return sym;
}

static node_pt
new_tmpvar_node( typeinfo_pt type,
		 ctxt_pt     ctxt,
		 file_pos_t  pos,
		 node_pt     value,
		 boolean     emit_initializer )
{
    symbol_pt sym = new_tmp_var( type, pos );
    node_pt result;

    append_decl(ctxt, sym);
    result = new_pos_node(pos, _Sym, sym);

    if (value) {
	if (emit_initializer) {
	    sym->has_initializer       = TRUE;
	    sym->emit_initializer      = TRUE;
	    sym->sym_value.initializer = value;
	} else {
	    append_pre(ctxt,
		       new_scoped_stmt_Expr(ctxt_scope(ctxt),
					    new_pos_node(value->node_def,
							 _Assign,
							 result,
							 value)));
	}

    }
    return result;
}
   

static boolean
has_pointer_type(node_pt e)
{
    typeinfo_pt t = type_of(e);
    return t->type_kind == pointer_to;
}

static boolean
has_array_type(node_pt e)
{
    typeinfo_pt t = type_of(e);
    return t->type_kind == array_of;
}

static boolean
has_type_univ_int(node_pt e)
{
    if (e->char_lit)                return FALSE;
    if (e->type == type_char())     return FALSE;
    if (e->node_kind==_Int_Number)  return TRUE;
    if (e->node_kind==_Char_to_Int) return TRUE;
    if (e->node_kind==_Macro_ID) {
	macro_t * m = e->node.macro;
	return m->const_value.eval_result_kind==eval_int &&
	       !m->const_value.explicit_type;
    }
    /* This is a special case to detect the generated translation
     * of (a<<b) into (a * 2**b)
     */
    if (e->node_kind==_Exp) {
	return TRUE;
    }
    if ((e->node_kind==_Mul || e->node_kind==_Div) &&
	has_type_univ_int(e->node.binary.l) &&
	has_type_univ_int(e->node.binary.r)) {

	return TRUE;
    }

    return FALSE;
}

static boolean
is_char_type(typeinfo_pt etype)
{
    if (assignment_equal_types(etype,type_char())) return TRUE;
    /*
     *TBD: other char types. In Ada's Interfaces.C package,
     * Char is the only character type: Plain_char, Signed_char,
     * and Unsigned_char are all integral (or modular) types. 
     * So maybe the above test is the only case to concern us.
     */
    return FALSE;
}

static boolean
is_char_array_type(typeinfo_pt t)
{
    return t->type_kind==array_of && is_char_type(t->type_next);
}

#define SAME_TYPE(t) (t)
#define UNIMPLEMENTED(arg) \
   warning(__FILE__, __LINE__, "unimplemented case in " arg)

/* forward reference */
static node_pt
fix_control_expr( node_pt e, ctxt_pt ctxt,
		 boolean control_bool, boolean pre_ok, boolean post_ok);


static symbol_pt
find_field( node_pt lnode, node_pt field, boolean indirect)
    /*
     * Find the symbol for the field of struct/union rec with the
     * same name as the field argument. Returns null pointer if
     * there is no such field.
     */
{
    symbol_pt   recsym;
    symbol_pt   sym;
    typeinfo_pt ltype = type_of(lnode);
    typeinfo_pt rectype;

    assert(field->node_kind == _Ident );
    if (indirect) {
	assert(is_access_to_record(ltype));
	rectype = ltype->type_next;
    } else {
	rectype = ltype;
	assert(rectype->type_kind==struct_of || rectype->type_kind==union_of);
    }
    recsym = rectype->type_base;
    if (recsym->aliases) {
	recsym = recsym->sym_value.aliased_sym;
    }

    /* A simple linear search */
    for (sym= recsym->sym_tags; sym; sym = sym->sym_parse_list) {
	if (!strcmp(field->node.id.name, sym->sym_ident->node.id.name)) {
	    return sym;
	}
    }
    return (symbol_pt)0;
}
    


static stmt_pt
combine_stmts( stmt_pt s1, stmt_pt s2, stmt_pt s3)
{
    return concat_stmts( concat_stmts(s1,s2), s3);
}

static stmt_pt
combine_stmt_ctxt(stmt_pt    s,
		  ctxt_pt    ctxt,
		  file_pos_t pos,
		  scope_id_t scope)
    /*
     * NB: frees context ctxt
     */
{
    stmt_pt s2;

    s2 = combine_stmts( pre_stmts(ctxt), s, post_stmts(ctxt));
    if (added_decls(ctxt)) {
	set_current_scope( ctxt_scope(ctxt) );
	return new_stmt_Compound(pos,
				 decls(ctxt,0),
				 s2);
    } else {
	set_stmts_scope( s2, scope);
	return s2;
    }
    free_context(ctxt);
}

static stmt_pt 
fix_stmt_itself( stmt_pt stmt )
{
    ctxt_pt ctxt;
    stmt_pt s1;
    stmt_pt result;

    if (!stmt) return 0;

    ctxt = new_context(stmt->scope);
    s1 = fix_stmt( stmt, ctxt );

    result = combine_stmt_ctxt(s1, ctxt, stmt->stmt_def, stmt->scope);

    free_context(ctxt);
    return result;
}

static node_pt zero_of_type( typeinfo_pt t );

void
fix_func_body( symbol_pt func )
{
    stmt_pt body;

    assert( func->has_initializer );
    in_initializer = FALSE;
    body = fix_stmt_itself( func->sym_value.body );
    if (!func->has_return && is_function(func)) {
	/* 
	 * func is a function (i.e. not returning void)
	 * but has no return statements in its body.
	 * So it's probably a function with no explicit
	 * return type that really should have been
	 * declared returning-void.
	 *
	 * But we'll play along, and synthesize a return statement.
	 */
	stmt_pt ret_stmt =
	    new_stmt_Return(body->stmt_def, 
			    zero_of_type(func->sym_type->type_next));
	assert(body->stmt_kind==_Compound);
	body->stmt.compound.stmts =
	    concat_stmts( body->stmt.compound.stmts, ret_stmt );
	warning_at( body->stmt_def,
		    "function %s has no return statement; dummy generated",
		    func->sym_ada_name );
    }
    func->sym_value.body = body;
}
	

static typeinfo_pt 
integral_promotion( typeinfo_pt type )
    /* TBD: this might belong in types.c */
{
    switch (type->type_kind) {
    case int_type:
	if (type->_long || type->_long_long) return type;
	if (type->_sizeof < SIZEOF_INT) return type_int();
	if (type->_unsigned) return type_unsigned();
	return type_int();

    case enum_type:
	/* TBD: do we ever need to worry about enums that
	 * don't fit in int? 
	 */
	return type_int();

    default:
	return type;

    }
}

static boolean
is_type_long_double( typeinfo_pt t )
{
    return t->type_kind==float_type && t->_long && t->_sizeof==SIZEOF_DOUBLE;
}

static boolean 
is_type_double( typeinfo_pt t )
{
    /* assume that typeof_typespec has been called somewhere along the line */
    return t->type_kind==float_type && !t->_long && t->_sizeof==SIZEOF_DOUBLE;
}    

static boolean
is_type_float( typeinfo_pt t )
{
    return t->type_kind==float_type && t->_sizeof==SIZEOF_FLOAT;
}

static boolean
is_type_unsigned_long( typeinfo_pt t )
{
    return t->type_kind==int_type && t->_long && t->_unsigned;
}

static boolean
is_type_long( typeinfo_pt t )
{
    return t->type_kind==int_type && t->_long && !t->_unsigned;
}

static boolean
is_type_unsigned( typeinfo_pt t)
{
    /* assumes type is not short */
    return t->type_kind==int_type && !t->_long && t->_unsigned;
}

static typeinfo_pt 
common_type( node_pt e1, node_pt e2 )
    /* 
     * Calculate the common type of two operands according to the
     * "usual arithmetic conversion" in C Std 3.2.1.5 
     * TBD: are all of these conversions explicity required in Ada?
     */
{
    typeinfo_pt t1 = type_of(e1);
    typeinfo_pt t2 = type_of(e2);

    /* TBD: this rule doesn't take into account the integral
     * promotions on short types.
     */
    if (equal_types(t1,t2)) return t1;

    /* TBD: assert arithmetic types */

    if (is_type_long_double(t1) || is_type_long_double(t2)) {
	return type_long_double();

    } else if (is_type_double(t1) || is_type_double(t2)) {
	return type_double();

    } else if (is_type_float(t1) || is_type_float(t2)) {
	return type_float();
    }

    t1 = integral_promotion(t1);
    t2 = integral_promotion(t2);

    if (is_type_unsigned_long(t1) || is_type_unsigned_long(t2)) {
	return type_unsigned_long();

    } else if ( (( is_type_long(t1))  && (is_type_unsigned(t2)))  ||
	       (( is_type_long(t2)) && (is_type_unsigned(t1)) ) ) {
	return (SIZEOF_LONG>SIZEOF_INT) ? 
	         type_long() :
		 type_unsigned_long();

    } else if (is_type_long(t1) || is_type_long(t2)) {
	return type_long();

    } else if (is_type_unsigned(t1) || is_type_unsigned(t2)) {
	return type_unsigned();

    } else {
	return type_int();
    }
}

static boolean
is_void_ptr( typeinfo_pt type )
{
    symbol_pt basetype = type->type_base;
    if (!basetype) return FALSE;
    return streq(basetype->sym_ada_name, "System.Address");
}

static node_pt
unchecked_conversion(node_pt e, typeinfo_pt to_type)
{
    typeinfo_pt from_type = type_of(e);
    file_pos_t  pos       = e->node_def;
    symbol_pt   conversion;
    boolean     in_header = in_initializer && current_unit_is_header;

    all_types_gened(from_type, pos);
    all_types_gened(to_type,   pos);

    conversion = unchecked_conversion_func(from_type, to_type, pos, in_header);

    return new_pos_node(pos,
			_Func_Call,
			new_pos_node(pos,_Sym,conversion),
			e);
}

static node_pt promote( node_pt e, typeinfo_pt to_type, ctxt_pt ctxt );

static node_pt
type_cast( node_pt e, typeinfo_pt to_type)
{
    file_pos_t  pos       = e->node_def;
    typeinfo_pt from_type = type_of(e);
    if (e->char_lit) from_type = type_char();

    if (from_type->type_kind == pointer_to &&
	to_type->type_kind   == pointer_to &&
	!(is_void_ptr(from_type) ||
	  assignment_equal_types(to_type,from_type)) ) {

	return unchecked_conversion( e, to_type );

    } else if (from_type->type_kind == enum_type) {

	return promote( unchecked_conversion(e, type_int()), to_type, 0 );

    } else if (to_type->type_kind == enum_type) {

	return unchecked_conversion( promote(e, type_int(), 0), to_type );

    } else if (to_type->type_kind == pointer_to && is_null_ptr_value(e)) {

	e->type = to_type;
	return e;

    } else if (from_type->type_kind == array_of &&
	       to_type->type_kind   == pointer_to ) {

	node_pt first = new_pos_node(pos, _Array_Index, e, zero());
	node_pt fixed_first = fix_expr(first, 0, 0);
	return promote(new_pos_node(pos, _Addrof, fixed_first),
		       to_type,
		       0);
		       

    } else if (from_type->type_kind == pointer_to &&
	       to_type->type_kind   != pointer_to   ) {

	/* In general, we're skunked.  We'll emit an unchecked
	 * conversion, so at least there's a handle for
	 * post-fixup.
	 */
	return unchecked_conversion( e, to_type );

    } else if (from_type->type_kind != pointer_to &&
	       to_type->type_kind   == pointer_to    ) {

	/* see comment above */
	return unchecked_conversion( e, to_type );
			
    } else {

	node_pt result = new_pos_node(pos,
				      _Type_Cast,
				      new_pos_node(pos, _Type, to_type),
				      e);
	/* TBD: we're hijacking the baseval field for 
	 * a special purpose here.
	 */
	if (from_type->type_kind==int_type &&
	    to_type->type_kind  ==int_type &&
	    !has_type_univ_int(e) &&
	    to_type->_unsigned != from_type->_unsigned) {

	    /* Flag that an unchecked conversion is required */
	    /* 
	     * TBD: better place for flag; this is using an int
	     * field that is normally only used for
	     * _Int_Number nodes.
	     */
	    if (!to_type->_unsigned && to_type->_sizeof > from_type->_sizeof) {
		/* larger signed type can hold all the values
		 * of a smaller unsigned type 
		 */
		result->baseval = FALSE;
	    } else if (to_type->_unsigned) {
		/* Then, apparently, we need nothing */
		result->baseval = FALSE;
	    } else {
		result->baseval = TRUE;
	    }
	}
	return result;
    }

} /* type_cast() */

static node_pt
type_convert( node_pt e, typeinfo_pt to_type )
{
    typeinfo_pt from_type = type_of(e);
    boolean to_char_type = is_char_type(to_type);
    boolean from_char_type = is_char_type(from_type) || e->char_lit;

    assert(to_type);
    all_types_gened(to_type, e->node_def);
    if (to_char_type && e->char_lit) {
	if (assignment_equal_types(to_type,type_char())) {
	    return e;
	}
    }
    if (to_char_type && !from_char_type) {
	e = new_pos_node(e->node_def,_Int_to_Char, e);
	if (assignment_equal_types(to_type,type_char())) {
	    return e;
	}
    } else if (from_char_type && !to_char_type) {
	e = new_pos_node(e->node_def,_Char_to_Int, e);
	if (to_type->type_kind==int_type) {
	    return e;
	}
    } 

    return type_cast(e, to_type);
}

static symbol_pt static_string_lit(node_pt text,
				   boolean is_const) ; /* forward ref */

static node_pt
ptr_to_static_string_lit( node_pt e, boolean is_const )
{
    symbol_pt sym = static_string_lit(e, is_const);
    return new_pos_node(sym->sym_def, _Sym, sym);
}


static node_pt 
promote( node_pt e, typeinfo_pt to_type, ctxt_pt ctxt )
{
    /*
     * NB: ctxt is currently unused, and some
     *     places in the code now pass a null pointer as a placeholder
     */
    typeinfo_pt from_type = type_of(e);
    if (e->char_lit) from_type = type_char();

    if (e->node_kind==_String) {

	boolean sim_types = equal_types(from_type, to_type);

	boolean is_const = sim_types? to_type->type_next->_constant : FALSE;
	node_pt node = ptr_to_static_string_lit(e, is_const);
	return promote( node, to_type, ctxt );

    } else if ( has_type_univ_int(e) &&
	        to_type->type_kind==int_type &&
	        !equal_types( to_type, type_char()) ) {

	/* It's unnecessary to cast a universal integer */
	return e;

    } else if (equal_types(from_type, to_type)) {

	if ( from_type->type_base == to_type->type_base) {
	    
	    return e;

	} else if (from_type->type_kind == enum_type) {

	    return e;

	} else if (from_type->type_kind == pointer_to &&
		   from_type->type_next->type_kind ==function_type) {

	    return e;

	} else {
	    
	    return type_convert( e, to_type );
	}

    } else {

	return type_convert( e, to_type );
    }

}

static node_pt
int_if_char( node_pt e )
{
    if (is_char_type(type_of(e))) {
	return new_pos_node(e->node_def, _Char_to_Int, e);
    }
    return e;
}

static node_pt
promote_integer( node_pt e )
{
    return promote( e, integral_promotion( type_of(e) ), 0 );
}


static typeinfo_pt 
type_of(node_pt expr)
{
    typeinfo_pt result = 0; 

    if (expr->type) return expr->type;

    /* If expr->type isn't filled in, figure out the type,
     * put it in expr->type, and return it.
     */

    switch (expr->node_kind) {
    case _FP_Number:
	/* TBD: we lost type information in scan.c! */
	result = type_double();
	break;
    case _Int_Number:
	/* TBD: we lost type information in scan.c! */
	result = type_int();
	break;
    case _Char_Lit:
	/* TBD: dichotomy between Ada and C type */
	result = type_char();
	break;
    case _Type:
	result = SAME_TYPE(expr->node.typ);
	break;
    case _Sym:
	result = SAME_TYPE(expr->node.sym->sym_type);
	break;
    case _String:
	result = type_string();
	break;
    case _Macro_ID:
	{
	    macro_t * m = expr->node.macro;
	    assert(m->macro_evald);
	    result = m->const_value.explicit_type;
	    if (!result) {
		switch (m->const_value.eval_result_kind) {
		case eval_int:
		    if (strchr(m->macro_body, '\'')) {
			/* assume macro is a char literal */
			result = type_char();
		    } else {
			result = type_int();
		    }
		    break;
		case eval_float:
		    result = type_double();
		    break;
		case eval_string:
		    result = type_char_array();
		    break;
		default:
		  /*		    assert(("macro has no value",0));*/
		    assert("macro has no value");
                    exit(1);
		}
	    }
	}
	break;

    case _Dot_Selected:
    case _Arrow_Selected:
	/*
	 * We have to look up the identifier with reference to
	 * the record containing it.
	 */
	{
	    symbol_pt sym;
	    node_pt   field = expr->node.binary.r;
	    if (field->node_kind==_Ident) {
		sym = find_field(expr->node.binary.l,
				 field,
				 expr->node_kind==_Arrow_Selected);
	    } else if (field->node_kind==_Sym) {
		sym = field->node.sym;
	    }
	    assert(sym!=0);
	    result = sym->sym_type;
	}
	break;
	
    case _Array_Index:
	{ 
	    /* Peel off indices and types in tandem */
	    node_pt node = expr->node.binary.r;
	    result = type_of(expr->node.binary.l)->type_next;
	    while (node->node_kind == _List) {
		result = result->type_next;
		node   = node->node.binary.l;
	    }
	}
	break;
	  
	
    case _Func_Call:
	{
	    typeinfo_pt t1 = type_of(expr->node.binary.l);
	    if (t1->type_kind==pointer_to) {
		t1 = t1->type_next;
	    }
	    assert(t1->type_kind==function_type);
	    result = t1->type_next;
	}
	break;

    case _Type_Cast:
	result = type_of(expr->node.binary.l);
	break;

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
	result = type_of(expr->node.binary.l);
	break;

    case _Eq:
    case _Ne:
    case _Lt:
    case _Le:
    case _Gt:
    case _Ge:
    case _Land:
    case _Lor:
	/* TBD: This is the C type of the expression.  type_boolean()
	 * would be the Ada type.  What's it gonna be?
	 */
	result = type_int();
	break;
    case _Band:
    case _Bor:
    case _Xor:
    case _Mul:
    case _Div:
    case _Rem:
	result = common_type(expr->node.binary.l, expr->node.binary.r);
	break;

    case _Add:
    case _Sub:
	{
	    typeinfo_pt tl = type_of(expr->node.binary.l);
	    typeinfo_pt tr = type_of(expr->node.binary.r);

	    if (tl->type_kind == pointer_to) {
		if (tr->type_kind == pointer_to) {
		    /* must be _Sub, in which case "right" answer
		       is ptrdiff_t */
		    result = type_int();
		} else {
		    result = tl;
		}
	    } else if (tr->type_kind == pointer_to) {
		result = tr;
	    } else {
		result = common_type( expr->node.binary.l,
				      expr->node.binary.r);
	    }
	}
	break;

    case _Shl:
    case _Shr:
	result = integral_promotion( type_of( expr->node.binary.l ));
	break;

    case _Exp:
	/* Since this is used only for Ada output, this implements
	 * the Ada type rule.
	 */
	result = type_of( expr->node.binary.l );
	break;

    case _Sizeof:	
	/* TBD: This is defined as size_t in C,
	 * which is Interfaces.C.size_t in Ada.
	 * On the other hand, the translation to Ada may involve a construct
	 * with a different type in Ada.
	 */
	result = type_int();  /* a not unreasonable approximation */
	break;

    case _Pre_Inc:
    case _Pre_Dec:
    case _Post_Inc:
    case _Post_Dec:
	result = SAME_TYPE(type_of(expr->node.binary.l));
	break;

    case _Addrof:
	result = add_pointer_type( type_of(expr->node.unary) );
	break;

    case _Unary_Plus:
    case _Unary_Minus:
    case _Ones_Complement:
	result = integral_promotion( type_of(expr->node.unary) );
	break;

    case _Not:
	result = type_int();
	break;


    case _Indirect:
	{
	    typeinfo_pt t = type_of(expr->node.unary);
	    assert(t->type_kind==pointer_to);
	    result = t->type_next;
	    break;
        }

    case _Cond:
	{
	    typeinfo_pt t1 = type_of(expr->node.cond.tru);
	    typeinfo_pt t2 = type_of(expr->node.cond.fals);

	    if (equal_types(t1,t2)) {
		result = t1;
	    } else if (t1->type_kind==int_type || t1->type_kind==float_type) {
		assert(t1->type_kind==int_type || t2->type_kind==float_type);
		result = common_type(expr->node.cond.tru,
				     expr->node.cond.fals);
	    } else if (t1->type_kind==pointer_to) {
		/* TBD: check for type compatibility */
		result = t1;
	    } else if (t2->type_kind==pointer_to) {
		/* TBD: really need to calculate common type */
		result = t2;
	    } else {
		/* TBD: more tractable cases */
		error(file_name(expr->node_def),
		      line_number(expr->node_def),
		      "can't determine type of conditional expression");
	    }
	}
	break;

    case _UnBool:
	result = type_int();
	break;

    case _Char_to_Int:
	result = type_univ_int();
	break;

    case _Int_to_Char:
	result = type_char();
	break;

    case _Ident:	/* This should only show up as a field identifier. */
    case _List:         /* in prelim fixup to function call */
	result = 0;
	break;


    case _Aggregate:
    default:
	warning_at(expr->node_def, "can't figure out out type");
	result = 0;
    }

    expr->type = result;
    return result;
}

static typeinfo_pt
ada_type_of( node_pt e)
{
    typeinfo_pt ctype = type_of(e);
    switch (e->node_kind) {
    case _Int_Number: 
	if (e->char_lit) return type_char();
	break;
    case _String:
	return type_char_array();
    case _Eq:
    case _Ne:
    case _Lt:
    case _Le:
    case _Gt:
    case _Ge:
    case _Land:
    case _Lor:
	return type_boolean();
    default:
       break;
    }
    return ctype;
}


static boolean 
takes_bool(node_pt e)
    /* returns TRUE if node_kind is translated in Ada into an
     * operator that takes boolean argument(s).
     */
{
    switch (e->node_kind) {
    case _Land:
    case _Lor:
	return TRUE;
    case _Not:
	return TRUE;  /* TBD: is this right? */
    default:
	return FALSE;
    }
}

static boolean
gives_bool(node_pt e)
    /*
     * returns TRUE iff node_kind is translated into an Ada
     * operator that returns boolean.
     */
{
    switch (e->node_kind) {
    case _Eq:
    case _Ne:
    case _Lt:
    case _Gt:
    case _Ge:
    case _Le:
    case _Land:
    case _Lor:
    case bool:
	return TRUE;
    case _Not:
	return FALSE; /* TBD: Not could be a special case */
    default:
	return FALSE;
    }
}

static node_pt
zero_of_type( typeinfo_pt t)
{
    node_pt e0 = 0;
    if (t->type_kind==pointer_to) {
	e0 = null(t);
    } else if (is_char_type(t)) {
	e0 = zero();
	e0->char_lit = TRUE;
    } else if (t->type_kind==int_type) {
	e0 = zero();
    } else if (t->type_kind==float_type) {
	e0 = new_node(_FP_Number, 0.0);
    } else {
	/* TBD */
    }
    return e0;
}

static node_pt
adjust_bool( node_pt e, boolean is_bool )
    /* converts e to or from boolean, as appropriate */
{
    boolean boolexpr =
	equal_types(type_of(e), type_boolean()) || gives_bool(e);
    node_pt result;
    typeinfo_pt t;
    node_pt e0 = 0;

    if (is_bool==boolexpr) {
	result = e;
    } else if (is_bool) {
	t = type_of(e);
	record_type_usage(e);
	e0 = zero_of_type(t);

	if (e0) {
	    result = new_pos_node(e->node_def, _Ne, e, e0);
	} else {
	    result = new_pos_node(e->node_def, bool, e);
	}
    } else {
	result = new_pos_node(e->node_def, _UnBool, e);
    }
    if (is_bool) result->type = type_boolean();
    return result;
}

static void
record_type_usage( node_pt e )
    /*
     * If the node will generate an Ada operator, note the
     * usage of the type.
     */
{
    typeinfo_pt type = type_of(e);
    assert(type);
    if (type != type_boolean()) {
	use_type( pos_unit(e->node_def), type );
    }
}

static node_pt 
fix_expr_Binop( node_pt e, ctxt_pt ctxt , usage_flags usage )
{

    usage_flags bool_subexprs = takes_bool(e)? USE(Is_bool) : 0;
    node_pt el = fix_expr( e->node.binary.l, ctxt, bool_subexprs);
    node_pt er = fix_expr( e->node.binary.r, ctxt, bool_subexprs);

    if (!bool_subexprs &&
	!has_pointer_type(el) && !has_pointer_type(er)) {

	/* TBD: or just: common = type_of( e ); */
	typeinfo_pt common = common_type( el, er );
	
	el = promote(el, common, ctxt);
	er = promote(er, common, ctxt);
	record_type_usage(el);
    }
    e-> node.binary.l = el;
    e-> node.binary.r = er;
	
    return e;
}

static node_pt
fix_expr_Binop_additive( node_pt e, ctxt_pt ctxt, usage_flags flags )
    /* + and - can involve pointer arithmetic */
{
    node_pt el = fix_expr( e->node.binary.l, ctxt, 0);
    node_pt er = fix_expr( e->node.binary.r, ctxt, 0);
    boolean el_is_ptr = has_pointer_type(el);
    boolean er_is_ptr = has_pointer_type(er);
    boolean el_is_array = has_array_type(el);
    boolean er_is_array = has_array_type(er);
    typeinfo_pt el_type = type_of(el);
    typeinfo_pt er_type = type_of(er);
    typeinfo_pt ptrs_type;
    typeinfo_pt ptrdiff_type = type_ptrdiff();

    if (el_is_ptr && er_is_ptr) {
	assert(e->node_kind = _Sub);

	/* TBD: const & volatile don't make any difference */
	assert(equal_types(el_type, er_type));

	/* We have to make sure an appropriate instantiation
	 * of Interfaces.C.Pointers is available.
	 */
	ptrs_type = ptrs_type_for(el_type, ctxt, e->node_def);
	el = type_cast(el, ptrs_type);
	er = type_cast(er, ptrs_type);
	e-> node.binary.l = el;
	e-> node.binary.r = er;
	e->type = type_ptrdiff();
	return e;

    }

    if (el_is_ptr) {

	ptrs_type = ptrs_type_for(el_type, ctxt, e->node_def);
	el = promote(el, ptrs_type, ctxt);
	er = promote(er, ptrdiff_type, ctxt);

    } else if (er_is_ptr) {

        ptrs_type = ptrs_type_for(er_type, ctxt, e->node_def);
	er = promote(er, ptrs_type, ctxt);
	el = promote(el, ptrdiff_type, ctxt);

    } else if (el_is_array) {

	return new_pos_node(e->node_def,
			    _Addrof,
			    new_pos_node(e->node_def, _Array_Index, el, er));

    } else if (er_is_array) {

	return new_pos_node(e->node_def,
			    _Addrof,
			    new_pos_node(e->node_def, _Array_Index, er, el));

    } else {
	
	e->node.binary.l = el;
	e->node.binary.r = er;
	return fix_expr_Binop(e, ctxt, flags);

    }

    e-> node.binary.l = el;
    e-> node.binary.r = er;
    record_type_usage(e);
    return e;
}


	

    

static typeinfo_pt
type_to_unsigned( typeinfo_pt type )
{
    typeinfo_pt to_type;
    /* TBD: enum_types are another possibility. */
    assert(type->type_kind==int_type);

    if (is_type_long(type)) {
	to_type = type_unsigned_long();
    } else if (equal_types(type, type_int())) {
	to_type = type_unsigned();
    } else if (equal_types(type, type_short())) {
	to_type = type_unsigned_short();
    } else if (is_char_type(type)) {
	to_type = type_unsigned();
    } else {
	to_type = 0;
    }
    return to_type;
}

static typeinfo_pt
type_to_builtin( typeinfo_pt type )
    /* returns the canonical builtin type for 
     * a possibly typedef'd integral type
     */
{
    typedef typeinfo_pt (*tf)(void);
    static tf tarray[] = {type_int,
			      type_unsigned,
			      type_short,
			      type_unsigned_short,
			      type_long,
			      type_unsigned_long,
			      type_unsigned_char,
			      type_signed_char   };

    static int tarray_size = sizeof(tarray)/sizeof(tf);
    int i;
    typeinfo_pt btype;
    
    for (i =0; i<tarray_size; i++) {
	btype = (*tarray[i])();
	if (equal_types(type, btype)) return btype;
    }
    return 0;
}

static node_pt
to_unsigned( node_pt e )
    /* Cast result to unsigned integer type if it's signed */
    /* or if it's a char type */
{
    typeinfo_pt type = type_of(e);
    typeinfo_pt to_type = 0;


    to_type = type_to_unsigned(type);

    if (to_type) {
	return type_convert(e, to_type);
    } else {
	return e;
    }
}

static boolean
val_must_be_unsigned( host_int_t val, int size )
{
    /* In bitwise operations involving a literal, we must take
     * care about whether to use a signed operation. If we
     * translate (x | 0x80000000) as a signed "or", then we'll
     * get a constraint error trying to treat 16#80000000# as
     * a signed integer.  The suitable range is therefore
     * (0..MAX_<size>_INT).
     */
     
    int nbits = size * BITS_PER_BYTE;
    return (val >> (nbits-1)) != 0;
}

static boolean
ok_signing( node_pt e, typeinfo_pt type )
{

    int size = type_sizeof(type);
    if (type->_unsigned) return TRUE;
    switch (e->node_kind) {
    case _Int_Number:
	return !val_must_be_unsigned(e->node.ival, size);
    case _Macro_ID:
	{
	    macro_t * m = e->node.macro;
	    if (m->const_value.eval_result_kind==eval_int) {
		return !val_must_be_unsigned(m->const_value.eval_result.ival,
					     size);
	    } else {
		return TRUE;
	    }
	}
    default:
	return TRUE;
    }
}
    

static node_pt
fix_expr_Binop_bitwise( node_pt e, ctxt_pt ctxt, usage_flags usage )
{
    node_pt el = fix_expr( e->node.binary.l, ctxt, 0);
    node_pt er = fix_expr( e->node.binary.r, ctxt, 0);
    typeinfo_pt to_type;

    {	/* calculate target type */

	typeinfo_pt common = common_type(el, er);
	boolean ok_signing_l = ok_signing( el, common );
	boolean ok_signing_r = ok_signing( er, common );

	if (ok_signing_l && ok_signing_r) {
	    to_type = type_to_builtin(common);
	} else {
	    to_type = type_to_unsigned(common);
	}
    }
    
    if (!(has_type_univ_int(el) && has_type_univ_int(er))) {
	
	el = promote(el, to_type, ctxt);
	er = promote(er, to_type, ctxt);
    }
    /* NB: These promotions may have resulted in two universal
     * types: the promotion of a char to int yields universal.
     * (Thus these if clauses aren't structured using if/else.)
     */
	 
    if (has_type_univ_int(el) && has_type_univ_int(er)) {

	/* one of the operands must be explicitly typed */
	el = type_cast(el, to_type);
    }

    e->node.binary.l = el;
    e->node.binary.r = er;
    record_type_usage(e);
    return e;
}


static boolean
has_side_effects( node_pt e )
{
    switch (e->node_kind) {
    case _FP_Number:
    case _Int_Number:
    case _Type:
    case _Ident:
    case _String:
	return FALSE;
    case _Sym:
	return e->node.sym->_volatile;
    case _List:
    case _Comma:
    case _Dot_Selected:
    case _Arrow_Selected:
    case _Array_Index:
    case _Type_Cast:
	return has_side_effects(e->node.binary.l) ||
	       has_side_effects(e->node.binary.r);
    case _Func_Call:
	return TRUE;
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
	return TRUE;
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
    case _Exp:
	return has_side_effects(e->node.binary.l) ||
	       has_side_effects(e->node.binary.r);
    case _Sizeof:
	return FALSE;
    case _Pre_Inc:
    case _Pre_Dec:
    case _Post_Inc:
    case _Post_Dec:
	return TRUE;

    case _Addrof:
    case _Unary_Plus:
    case _Unary_Minus:
    case _Ones_Complement:
    case _Not:
    case _Indirect:					/* last unary */
    case bool:
    case _UnBool: 
	return has_side_effects(e->node.unary);

    case _Cond:
	return has_side_effects(e->node.cond.bool) ||
	       has_side_effects(e->node.cond.tru)  ||
	       has_side_effects(e->node.cond.fals);

    case _Macro_ID:
	/* assuming that it's a constant macro at this point */
	return FALSE;
	
    case _Aggregate:
    default:
	UNIMPLEMENTED("has_side_effects");
	return FALSE;
    }
}

static boolean
is_complicated( node_pt e )
{
    return FALSE;
}
	


static node_pt
fix_expr_Binop_Assign( node_pt e, ctxt_pt ctxt, usage_flags usage )
    /*
     * e1 += e2 unfolds to e1 = e1' + e2;
     * where e1' is a symbol renaming
     * 
     */
{
    node_pt e1 = fix_expr( e->node.binary.l, ctxt, 0 );
    node_pt e2 = fix_expr( e->node.binary.r, ctxt, 0 );
    node_pt e1_op_e2;  /* rhs of assignment */
    node_pt result;
    node_kind_t op = non_assign_op(e->node_kind);
    
    if (has_side_effects(e1) || is_complicated(e1)) {
	symbol_pt tmpvar = new_tmp_var( type_of(e), e->node_def );

	tmpvar->renames = TRUE;
	tmpvar->sym_value.initializer = e1;
	append_decl(ctxt, tmpvar);

	e1 = new_pos_node(e->node_def, _Sym, tmpvar);
    }
    e1_op_e2 = fix_expr( new_pos_node(e->node_def, op, e1, e2), ctxt, 0 );
    result = fix_expr(new_pos_node(e->node_def, _Assign, e1, e1_op_e2),
		      ctxt,
		      usage );
    return result;

} /* fix_expr_Binop_Assign */
    

static node_pt
fix_expr_Simple_Binop( node_pt e, ctxt_pt ctxt , usage_flags usage )
    /*
     * This is the basic fix routine for binop expressions
     * that do not require promotion.
     * i.e. _List and _Type_Cast
     */
{
    node_pt el = fix_expr( e->node.binary.l, ctxt, 0 );
    node_pt er = fix_expr( e->node.binary.r, ctxt, 0 );
    e->node.binary.l = el;
    e->node.binary.r = er;
    return e;
}

static node_pt
fix_expr_Type_Cast( node_pt e, ctxt_pt ctxt, usage_flags usage )
{
    if (USAGE(Is_stmt)) {
	/* probably a cast to void, but irrelevant in any case */
	return fix_expr( e->node.binary.r, ctxt, usage );
    } else {
	node_pt er;
	e = fix_expr_Simple_Binop( e, ctxt, usage );
	er = e->node.binary.r;
	if (er->node_kind==_String) {
	    er = ptr_to_static_string_lit(er, FALSE);
	}
	return type_convert(er, e->node.binary.l->node.typ);
    }
}

static node_pt
fix_expr_Array_Index( node_pt e, ctxt_pt ctxt, usage_flags usage )
{
    node_pt el = fix_expr( e->node.binary.l, ctxt, 0 );
    node_pt er = fix_expr( e->node.binary.r, ctxt, 0 );
    typeinfo_pt eltype ;

    if (el->node_kind == _Array_Index) {

	e->node.binary.l = el->node.binary.l;
	e->node.binary.r =
	    new_pos_node(e->node_def, _List, el->node.binary.r, er);
	return e;

    } else if ( (eltype=type_of(el))->type_kind == pointer_to ) {
	
	/* convert a[i] => *(a+i) */
	return fix_expr(new_pos_node(e->node_def, 
				     _Indirect,
				     new_pos_node(e->node_def,_Add,el,er)),
			ctxt, 0);

    } else {

	if (is_char_array_type( type_of(el) )) {
	    er = promote(er, type_size_t_(), ctxt);
	} else {
	    er = promote(er, type_int(),     ctxt);
	}

	e->node.binary.l = el;
	e->node.binary.r = er;
	return e;

    }

}


static node_pt
fix_expr_Comma( node_pt e, ctxt_pt ctxt, usage_flags usage )
    /* The first subexpression in a comma expression is evaluated
     * just for its side-effects; the value is the value of the 
     * second subexpression.
     * And note there's a sequence point between the two expressions
     */
{
    /* TBD: we'll assume that the first subexpression is
     * really present for its side effects.
     */
    stmt_pt s0 = new_scoped_stmt_Expr(ctxt_scope(ctxt), e->node.binary.l);
    stmt_pt s1 = fix_stmt_itself(s0);
    append_pre(ctxt, s1);

    return fix_expr( e->node.binary.r, ctxt, usage );
}
    


static node_pt
fix_expr_Select( node_pt e, ctxt_pt ctxt, usage_flags usage )
{
    node_pt   el    = fix_expr( e->node.binary.l, ctxt, 0 );
    node_pt   er    = fix_expr( e->node.binary.r, ctxt, 0 );
    symbol_pt field = 0;

    if (er->node_kind==_Ident) {
	field = find_field(el, er, e->node_kind==_Arrow_Selected);

	if (!field) {
	    fatal(__FILE__,__LINE__,
		  "field '%s' not in type; file %s line %d",
		  er->node.id.name,
		  file_name(e->node_def),
		  line_number(e->node_def));
	}
    } else if (er->node_kind==_Sym) {
	field = er->node.sym;
    } else {
      /*assert(("bad field type",0)); */
	assert("bad field type");
        exit(1);
    }

    if (el->node_kind==_Add || el->node_kind==_Sub) {
	/* pointer arithmetic */
	node_pt tmpvar =
	    new_tmpvar_node( type_of(el), ctxt, el->node_def, el, TRUE);
	e->node.binary.l = tmpvar;
    } else {
	e->node.binary.l = el;
    }
    e->node.binary.r = new_pos_node(er->node_def,_Sym,field);
    return e;
}

static node_pt
to_natural( node_pt e )
{
    return new_pos_node(e->node_def,
			_Type_Cast, 
			new_pos_node(e->node_def, _Type, type_natural()),
			e);
}


static node_pt 
fix_expr_Shift( node_pt e, ctxt_pt ctxt , usage_flags usage )
    /* _Shl or _Shr */
{
    node_pt el = fix_expr( e->node.binary.l, ctxt, 0 );
    node_pt er = fix_expr( e->node.binary.r, ctxt, 0 );

    el = promote(el, integral_promotion( type_of(el) ), ctxt);

    if (!has_type_univ_int(er)) {
	/* both the shift functions and exponentiation require
	 * a Natural argument.
	 */
	er = to_natural(er);
    }

    if (type_of(el)->_unsigned) {
	e->node.binary.l = el;
	e->node.binary.r = er;
	return e;
    } else {
	/* x<<y ==> x*(2**y) */
	/* x>>y ==> x/(2**y) */
	node_pt exp;
	node_pt mul;
	file_pos_t pos = e->node_def;
        exp = new_pos_node(er->node_def, _Exp, two(), er);
	mul = new_pos_node(pos, e->node_kind==_Shl? _Mul: _Div, el, exp);
	record_type_usage(el);
	return mul;
    }
}

static node_pt
fix_expr_Addrof( node_pt e, ctxt_pt ctxt , usage_flags usage )
{
    node_pt esub = fix_expr( e->node.unary, ctxt, 0 );
    if (esub->node_kind==_Indirect) {
	return esub->node.unary;
    } else {
	e->node.unary = esub;
	return e;
    }

}



static node_pt
fix_expr_Unop( node_pt e, ctxt_pt ctxt , usage_flags usage )
    /* for unary plus, unary minus */
    /* See also fix_expr_Not, fix_expr_Ones_Complement */
{
    node_pt esub ;
    boolean to_bool = takes_bool(e);

    if (to_bool) {
	esub = fix_expr(e->node.unary, ctxt, USE(Is_bool));
    } else {
	esub = promote_integer( fix_expr( e->node.unary, ctxt, 0 ));
    }
    e->node.unary = esub;
    return e;
}

static node_pt
fix_expr_Ones_Complement( node_pt e, ctxt_pt ctxt, usage_flags usage )
{
    node_pt esub;
    esub = fix_expr(e->node.unary, ctxt, 0);
    if (has_type_univ_int(esub)) {
	esub = type_cast(esub, type_unsigned());
    } else {

	typeinfo_pt itype = integral_promotion( type_of(esub) );	
	if (!ok_signing(esub, itype)) {
	    esub = type_convert( esub, type_to_unsigned(itype));
	}
	e->node.unary = esub;
	return fix_expr_Unop( e, ctxt, usage );
#if 0

	if (itype->_unsigned) {
	    esub = promote(esub, itype);
	} else {
	    esub = type_convert( esub, type_to_unsigned(itype));
	}
#endif
    }
    e->node.unary = esub;
    record_type_usage(esub);
    return e;

}

static node_pt
fix_expr_Not( node_pt e, ctxt_pt ctxt , usage_flags usage )
{
    node_pt e0 = fix_expr(e->node.unary, ctxt, usage);
    node_pt result;
    node_pt e1;

    /* gen_expr generally relies on this field being filled in */
    e0->type = type_of(e0);

    if (USAGE(Is_bool)) {
	/* TBD: any other operators to invert? */
	if (e0->node_kind==_Ne) {
	    e0->node_kind = _Eq;
	    result = e0;
	} else {
	    e->node.unary = e0;
	    result = e;
	}
	result->type = type_boolean();
    } else {
	e1 = zero_of_type( e0->type );
	if (e1) {
	    result = new_pos_node(e->node_def, _Eq, e1, e0);
	} else {
	    error_at( e->node_def, "can't form '!' expression");
	}
    }
    return result;
}

static node_pt
fix_expr_Crement( node_pt e, ctxt_pt ctxt , usage_flags usage )
    /* fix up pre- and post- increment and decrement */
{  /* cases:   ++i  ==>  {i:=i+i} i {}
                ++(*x) ==> {x.all := x.all + 1} x.all {}
                ++(*x++) => {x.all := x.all+1} x.all {INC(x)}
                ++(*f(z)) => { declare x;
		               begin x = f(z); end} 
			     x.all
			     {}
    */

    /* TBD: assure that subexpr is sufficiently simple: i.e,
     * that the transform  ++e ==> {e:=e+1} e {}  is valid
     */
    node_pt esub = fix_expr(e->node.unary, ctxt, 0);
    node_kind_t op;
    node_pt incr;
    node_pt assn;

    switch (e->node_kind) {
    case _Pre_Inc:
    case _Post_Inc:
	op = _Add;
	break;
    case _Pre_Dec:
    case _Post_Dec:
	op = _Sub;
	break;
    default:
        break;
    }

    incr = new_pos_node( esub->node_def, op, esub, one() );
    assn = new_pos_node( esub->node_def,_Assign, esub, incr );

    /* This call to fix_expr is intended to generate the appropriate
     * pointers package, if it doesn't already exist.
     * (It also records type usage.)
     */
    assn = fix_expr( assn, ctxt, USE(Is_stmt) );

    if (USAGE(Is_stmt)) {
	return assn;
    } else {
	stmt_pt stmt = new_scoped_stmt_Expr( ctxt_scope(ctxt), assn );
	switch (e->node_kind) {
	case _Pre_Inc:
	case _Pre_Dec:
	    append_pre( ctxt, stmt );
	    break;
	case _Post_Inc:
	case _Post_Dec:
	    append_post( ctxt, stmt );
	    break;
        default:
            break;
	}
	return esub;
    }
}


static node_pt
fix_expr_Assign( node_pt e, ctxt_pt ctxt , usage_flags usage )
{
    node_pt el = fix_expr( e->node.binary.l, ctxt, 0 );
    node_pt er = fix_expr( e->node.binary.r, ctxt, 0 );
    stmt_pt stmt;

    er = promote( er, type_of(el), ctxt );

    e->node.binary.l = el;
    e->node.binary.r = er;

    if (USAGE(Is_stmt)) {
	return e;
    } else {
	/* TBD: el must be a suitable l-value */

	stmt = new_scoped_stmt_Expr( ctxt_scope(ctxt), e );
	append_pre(ctxt, stmt);
	
	return el;
    }
}

static node_pt
fix_expr_Indirect( node_pt e, ctxt_pt ctxt, usage_flags usage )
{
    node_pt base = fix_expr( e->node.unary, ctxt, 0 );
    if (base->node_kind==_Add || base->node_kind==_Sub) {
	/* pointer arithmetic */
	node_pt tmpvar =
	    new_tmpvar_node( type_of(base), ctxt, base->node_def, base, TRUE );
	e->node.unary = tmpvar;
    } else {
	e->node.unary = base;
    }
    return e;
}

static node_pt
fix_expr_Sizeof( node_pt e, ctxt_pt ctxt, usage_flags usage )
{
    /* TBD: trivial fixup for now */
    return e;
}


static symbol_pt
next_formal( symbol_pt * formal_iter )
{
    symbol_pt result;
    if (!formal_iter) {
	return 0;
    } else if ( *formal_iter==ellipsis_sym) { 
        return ellipsis_sym;
    } else {
	result = *formal_iter;
	if (result) {
	    *formal_iter = (*formal_iter)->sym_parse_list;
	} else {
	    *formal_iter = 0;
	}
	return result;
    }
}

static typeinfo_pt
arg_promotion( typeinfo_pt type )
    /* Return the type resulting from "default argument promotion"
     * of 'type'.
     */
{
    switch (type->type_kind) {
    case int_type:
	return integral_promotion(type);
    case float_type:
	if (equal_types(type, type_float())) {
	    return type_double();
	} else {
	    return type;
	}
    case enum_type:
	return type_int();
    case array_of:
	return add_pointer_type(type->type_next);
    default:
	return type;
    }
} /* arg_promotion */
    

static node_pt
promote_arg( node_pt e, symbol_pt formal, ctxt_pt ctxt )
{
    
    if (formal) {
	return promote(e, formal->sym_type, ctxt);
    } else {
	/* Else we have to do the C "default argument promotions". */
	typeinfo_pt type = type_of(e);
	switch (type->type_kind) {

	case int_type:
	    return promote_integer( e );

	case float_type:
	    if (equal_types(type,type_float())) {
		return type_convert(e, type_double());
	    } else {
		return e;
	    }

	case enum_type:
	    return type_convert(e, type_int());

	case array_of:
	    /* Arrays are passed as pointers */
	    {
		typeinfo_pt el_type = type->type_next;
		typeinfo_pt ptr_type = add_pointer_type(el_type);
		return type_convert(e, ptr_type);
	    }

	case pointer_to:
	    /* Going through promote catches some string cases */
	    return promote(e, type, ctxt);

	default:
	    return e;
	}
    }
} /* promote_arg */

static node_pt
fix_undeclared_func_id( node_pt id, node_pt eArgs, scope_id_t scope )
{
    symbol_pt  sym;
    file_pos_t pos = id->node_def;


    assert(id->node_kind==_Ident);

    /* First let's see if function is now declared */
    sym = find_sym( id->node.id.name );
    if (sym) {
	/* TBD?: check that sym is suitable */

	if (pos_in_current_unit(sym->sym_def)) {

	    /* Function was defined or declared later in same file. */
	    symbol_pt decl = copy_sym(sym);
	    decl->has_initializer = FALSE;
	    gen_ada_func(decl, scope_parent_func(scope));

	} else {
	    unit_dependency( current_unit(), pos_unit(sym->sym_def), TRUE);
	}

    } else {
	/* We have to declare the function */

	node_pt declarator;
	symbol_pt params;

	if (eArgs) 
        {
	    node_iter_t arg_iter;
	    node_pt *   argp;
	    for (arg_iter=init_node_iter(&eArgs), params = 0;
		 (argp = next_list_ref(&arg_iter)) ; ) 
            {

		typeinfo_pt atype = type_of(*argp);
		typeinfo_pt ptype = arg_promotion(atype);
		symbol_pt param = noname_simple_param(ptype);

		params = concat_symbols(params, param);
	    }
	} 
        else 
        {
	    params = copy_sym (type_void()->type_base );
	}

	declarator =
	    new_pos_node(pos, 
			 _Func_Call,
			 id, 
			 params? new_pos_node(pos, _Sym, params):0 );
	sym = var_declaration(type_int(), declarator);
	sym->sym_def = pos;
	gen_ada_func(sym, scope_parent_func(scope));
    }
    return new_pos_node(pos, _Sym, sym);
} /* fix_undeclared_func */

static node_pt
fix_expr_Func_Call( node_pt e, ctxt_pt ctxt, usage_flags usage )
{
    node_pt eFunc = fix_expr(e->node.binary.l, ctxt, USE(Is_called));
    node_pt eArgs = e->node.binary.r;
    typeinfo_pt functype = type_of(eFunc);
    symbol_pt formals;

    if (functype) {

	formals = functype->type_info.formals;
	if (!formals) {
	    warning_at(e->node_def,
		       "call to function with no parameter information");
	}
    } else {
	/* presumably an undeclared function ID */
	assert(eFunc->node_kind==_Ident);

	eFunc = fix_undeclared_func_id( eFunc, eArgs, ctxt_scope(ctxt) );
	functype = type_of(eFunc);
	assert(functype);
	formals = functype->type_info.formals;
    }

    if (eArgs) {
	node_iter_t arg_iter;
	node_pt	*   argp;
	symbol_pt   formal;
	symbol_pt   formal_iter = formals;
	node_pt     varargs = 0;
	node_pt *   varargs_p = 0;
	unit_n      unit = pos_unit(e->node_def);

	eArgs = fix_expr(eArgs, ctxt, 0);
	for (arg_iter=init_node_iter(&eArgs);
	     (argp = next_list_ref(&arg_iter)) ; ) {

	    formal = next_formal(&formal_iter);
	    if (formal==ellipsis_sym) {

		node_pt arg;
		typeinfo_pt arg_type;

		/* arg list will look like stdarg.empty [ & arg]+ */
		arg = promote_arg(*argp, 0, ctxt);

		arg_type = type_of(arg);
		all_types_gened(arg_type, arg->node_def);

		/* make sure "&" is defined for arg types */
		use_stdarg_concat(unit, arg_type);

		if (!varargs) {
		    varargs = stdarg_empty_node(arg->node_def);
		    varargs_p = node_iter_tail(&arg_iter);
		}
		varargs = new_pos_node(arg->node_def, _Concat, varargs, arg);

	    } else {

		*argp = promote_arg(*argp, formal, ctxt);
	    }
	}
	if (varargs_p) {
	    *varargs_p = varargs;
	    varargs->fixed = TRUE;
	}
    }

    e->node.binary.l = eFunc;
    e->node.binary.r = eArgs;
    return e;
}

static node_pt
fix_expr_Land( node_pt e, ctxt_pt ctxt, usage_flags usage )
{
    scope_id_t scope1 = new_block_scope( ctxt_scope(ctxt) );
    ctxt_pt ctxt1 = new_context(scope1);
    node_pt el    = fix_expr(e->node.binary.l, ctxt, USE(Is_bool));
    node_pt er    = fix_expr(e->node.binary.r, ctxt1, USE(Is_bool));
    if (changed_pre(ctxt1) || changed_post(ctxt1) || added_decls(ctxt1)) {
	/*
	 * given A && B=f(...)))
	 * and assuming no postB:
	 * want to generate
	 *     if A:               s1
	 *        preB             s2
	 *        return B         s3 -- may be compound statement
         *     else
         *        return False     s4
	 */
	stmt_pt s1, s3pre, s3, s4;
	symbol_pt localfunc;

	set_current_scope(scope1);
	s4 = return_bool_stmt(FALSE, e->node_def);
	s3pre = new_stmt_Return(e->node_def, er);
	s3 = combine_stmt_ctxt( s3pre, ctxt1, e->node_def, scope1 );
	s1 = new_stmt_Ifelse(el->node_def, 0, el, s3, s4);
	
	localfunc = new_local_func(type_boolean(),
				   0,
				   new_stmt_list( s1 ),
				   e->node_def,
				   scope1);
	localfunc->has_return = TRUE;
	append_decl(ctxt, localfunc);
	return new_pos_node(e->node_def,
			    _Func_Call,
			    new_pos_node(e->node_def, _Sym, localfunc),
			    0);
    } else {
	e->node.binary.l = el;
	e->node.binary.r = er;
	return e;
    }
} /* fix_expr_Land */


static node_pt
fix_expr_Lor( node_pt e, ctxt_pt ctxt, usage_flags usage )
{
    scope_id_t scope1 = new_block_scope( ctxt_scope(ctxt) );
    ctxt_pt    ctxt1  = new_context(scope1);
    node_pt    el     = fix_expr(e->node.binary.l, ctxt, USE(Is_bool));
    node_pt    er     = fix_expr(e->node.binary.r, ctxt1, USE(Is_bool));
    if (changed_pre(ctxt1) || changed_post(ctxt1) || added_decls(ctxt1)) {
	/*
	 * given A && B=f(...)))
	 * and assuming no postB:
	 * want to generate
         *     if A:               s1
	 *        return True      s2
         *     else             
	 *        preB             s3
         *        return B         s4
	 */
	stmt_pt   s1, s2, s4, s4pre;
	symbol_pt localfunc;
	node_pt   result;

	set_current_scope(scope1);
	s2 = return_bool_stmt(TRUE, e->node_def);

	s4pre = new_stmt_Return(e->node_def, er);
	s4 = combine_stmt_ctxt( s4pre, ctxt1, e->node_def, scope1 );

	s1 = new_stmt_Ifelse(el->node_def, 0, el, s2, s4);
	
	localfunc = new_local_func(type_boolean(),
				   0,
				   new_stmt_list( s1 ),
				   e->node_def,
				   scope1);
	localfunc->has_return = TRUE;
	append_decl(ctxt, localfunc);
	result = new_pos_node(e->node_def,
			      _Func_Call,
			      new_pos_node(e->node_def,_Sym,localfunc),
			      0);
	result->type = type_boolean();
	return result;
    } else {
	e->node.binary.l = el;
	e->node.binary.r = er;
	return e;
    }
}


static node_pt
fix_expr_Cond( node_pt e, ctxt_pt ctxt, usage_flags usage )
{
    /* TBD: consider is_stmt */
    symbol_pt tmpvar;      /* new tmp var */
    node_pt evar;          /* node representing tmpvar */
    node_pt e1,e2,e3 ;     /* subexprs of (e1?e2:e3) */
    stmt_pt s2, s3   ;     /* stmts for var = e2, var=e3 */
    stmt_pt sCond    ;     /* if-else stmt */

    set_current_scope( ctxt_scope(ctxt) );

    tmpvar = new_tmp_var(type_of(e), e->node_def);
    append_decl(ctxt, tmpvar);
    evar   = new_pos_node(e->node_def,_Sym, tmpvar);

    e1     = fix_control_expr( e->node.cond.bool, ctxt, TRUE, TRUE, FALSE );

    /* NB: e2 and e3 are fixed here: it's important that they be
     * fixed in a separate context (new_stmt_Ifelse) because of
     * the sequence points.
     */
    e2     = e->node.cond.tru;
    e3     = e->node.cond.fals;

    set_current_scope( ctxt_scope(ctxt) );
    s2     = new_stmt_Expr(new_pos_node(e2->node_def,_Assign, evar, e2 ));
    s3     = new_stmt_Expr(new_pos_node(e3->node_def,_Assign, evar, e3 ));

    sCond  = fix_stmt_itself(new_stmt_Ifelse(e->node_def,0,e1, s2, s3));
    append_pre(ctxt, sCond);

    return evar;
    

}

     

static node_pt
fix_expr_Trivially( node_pt e, ctxt_pt ctxt, usage_flags usage )
{
    return e;
}

static node_pt
fix_expr_Sym( node_pt e, ctxt_pt ctxt, usage_flags usage )
{
    typeinfo_pt type = type_of(e);
    symbol_pt   sym  = e->node.sym;
    unit_dependency( current_unit(), pos_unit(sym->sym_def), TRUE);
    if (!USAGE(Is_called) && type->type_kind == function_type) {
	return new_pos_node(e->node_def,_Addrof, e);
    } else {
	return e;
    }
}

static node_pt
fix_expr_Macro_ID( node_pt e, ctxt_pt ctxt, usage_flags usage )
    /* No fixup; just record unit dependency */
{
    struct macro_t * macro = e->node.macro;
    unit_dependency(current_unit(),
		    pos_unit(macro->macro_definition),
		    !(current_unit_is_header && in_initializer));
    return e;
}

static node_pt
fix_expr_Type( node_pt e, ctxt_pt ctxt, usage_flags usage )
{
    all_types_gened(e->node.typ, e->node_def);
    return e;
}

static node_pt
fix_expr_Unimplemented( node_pt e, ctxt_pt ctxt,
		        usage_flags usage )
{
    warning_at(e->node_def,
	       "unimplemented expression type (%s) in fix_expr",
	       nameof_node_kind(e->node_kind));
    return e;
}

typedef node_pt (*fix_expr_func_t)(node_pt, ctxt_pt, usage_flags usage );

static fix_expr_func_t
fix_expr_func( node_pt expr )
{
    switch (expr->node_kind) {

    case _Error:
    case _Ellipsis:
	/* shouldn't show up in an expression */
	return fix_expr_Unimplemented;

    case _FP_Number:
    case _Int_Number:
    case _String:
    case _Ident:
	return fix_expr_Trivially;

    case _Macro_ID:
	return fix_expr_Macro_ID;
    case _Sym:
	return fix_expr_Sym;
    case _Type:
	return fix_expr_Type;

    case _Bit_Field:
	/* This shouldn't show up in an expression */
	return fix_expr_Unimplemented;

    case _Dot_Selected:
    case _Arrow_Selected:
	return fix_expr_Select;

    case _Func_Call:
	return fix_expr_Func_Call;

    case _Array_Index:
	return fix_expr_Array_Index;
    case _List:
	return fix_expr_Simple_Binop;
    case _Type_Cast:
	return fix_expr_Type_Cast;

    case _Comma:
	return fix_expr_Comma;

    case _Assign:
	return fix_expr_Assign;

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
	return fix_expr_Binop_Assign;

    case _Eq:
    case _Ne:
    case _Lt:
    case _Le:
    case _Gt:
    case _Ge:
	return fix_expr_Binop;

    case _Land:
	return fix_expr_Land;

    case _Lor:
	return fix_expr_Lor;

    case _Band:
    case _Bor:
    case _Xor:
	return fix_expr_Binop_bitwise;
	
    case _Add:
    case _Sub:
	return fix_expr_Binop_additive;

    case _Mul:
    case _Div:
    case _Rem:
	return fix_expr_Binop;

    case _Shl:
    case _Shr:
	return fix_expr_Shift;

    case bool:
    case _UnBool:
    case _Exp:
	/*
	 * These operators never show up in C source; they're
	 * introduced by various fixups.  In certain cases (for,
	 * instance, local functions), fix_stmt will be called
	 * redundantly, so these might be encountered.  Since the
	 * subexpression under these nodes has already been fixed,
	 * fix-up is trivial
	 */
	return fix_expr_Trivially;

    case _Sizeof:
	return fix_expr_Sizeof;

    case _Pre_Inc:
    case _Pre_Dec:
    case _Post_Inc:
    case _Post_Dec:
	return fix_expr_Crement;

    case _Addrof:
	return fix_expr_Addrof;

    case _Unary_Plus:
    case _Unary_Minus:
	return fix_expr_Unop;

    case _Ones_Complement:
	return fix_expr_Ones_Complement;

    case _Not:
	return fix_expr_Not;

    case _Aggregate:
        /* shouldn't show up in an expression */
	return fix_expr_Unimplemented;

    case _Indirect:
	return fix_expr_Indirect;

    case _Cond:
	return fix_expr_Cond;

    default:
	return fix_expr_Unimplemented;
    }
}


static node_pt
fix_expr( node_pt expr, ctxt_pt ctxt, usage_flags usage )
{
    node_pt result;
    typeinfo_pt result_type, explicit_type;
    fix_expr_func_t func;

    if (expr->fixed) return expr;

    func = fix_expr_func(expr);
    result = (*func)(expr, ctxt, usage);
    result = adjust_bool( result, USAGE(Is_bool));

    explicit_type = result->type;
    result_type = type_of(result);
    if (explicit_type && !equal_types(explicit_type,result_type)) {
	warning_at(expr->node_def,
		   "inconsistent type determination during fixup");
    }
    result->fixed = TRUE;
    return result;
}

static node_pt
string_lit_as_ptr( node_pt e, typeinfo_pt t )
{
    node_pt result;
    e->no_nul = TRUE;
    result = new_pos_node(e->node_def,
			  _Func_Call,
			  new_pos_node(e->node_def,
				       _Sym,
				       predef_new_string_func()),
			  e);
    result->fixed = TRUE;
    return result;
}

static symbol_pt 
static_string_lit(node_pt text, boolean is_const)
{
    static int counter;
    symbol_pt sym;

    sym = new_sym();
    sym->sym_kind = var_symbol;
    sym->sym_ada_name = new_strf("String_%d", ++counter);
    sym->sym_def = text->node_def;

    /*
     * We turn off this flag to allow the string literals to be generated
     * in the body.  This assumes that the only place this function gets
     * called from is code generation, which is always in the body.  The
     * flag could be set if we were translating a macro or inline function
     * from a header.
     */
    sym->_declared_in_header = current_unit_is_header && in_initializer;

    text->no_nul = TRUE;

    sym->has_initializer = TRUE;
    sym->sym_value.initializer = string_lit_as_ptr( text, 0 );
    sym->sym_type = is_const? type_const_charp() : type_string();

    sym->gened = TRUE;
    gen_ada_lit(sym);

    return sym;

} /* static_string_lit() */


node_pt
fix_initializer_expr( node_pt e, typeinfo_pt t )
{
    node_pt result;

    if (e->fixed) return e;
    in_initializer = TRUE;

    if (!t) {
	t = ada_type_of(e);
    }

    if ( e->node_kind == _String && equal_types(t, type_string())) {

	result = string_lit_as_ptr( e, t );

    } else if (e->node_kind==_String && 
	       t->type_kind==array_of) {

	if (t->type_info.array.elements != -1) {

	    result = new_pos_node(e->node_def,
				  _Type_Cast,
				  new_pos_node(e->node_def, _Type, t),
				  e);
	} else {
	    
	    result = e;
	}

    } else {

	ctxt_pt ctxt = new_context(0 /* TBD */);
	result = fix_expr( e, ctxt, 0 );
	result = promote( result, t, ctxt );
	
	assert( !changed_pre(ctxt)  );
	assert( !changed_post(ctxt) );
	assert( !added_decls(ctxt)  );
	free_context(ctxt);
    }
    return result;
}



static stmt_pt
ignore_result( node_pt e, ctxt_pt ctxt )
{
    typeinfo_pt t = type_of(e);
    ctxt_pt ctxt1 = new_context( ctxt_scope(ctxt) );
    symbol_pt sym;
	
    

    /* These two cases handled in gen_expr */
    if (same_ada_type(t, type_int())) return 0;
    if (same_ada_type(t, type_string())) return 0;

    all_types_gened(t, e->node_def);
    sym = new_tmp_var( t, e->node_def);
    sym->has_initializer = TRUE;
    sym->emit_initializer = TRUE;
    sym->sym_value.initializer = e;
    append_decl(ctxt1, sym);
    return combine_stmt_ctxt(0, ctxt1, e->node_def, ctxt_scope(ctxt));
}


static stmt_pt
fix_stmt_Expr( stmt_pt stmt, ctxt_pt ctxt )
{
    node_pt expr = fix_expr( stmt->stmt.expr, ctxt, USE(Is_stmt) );
    stmt_pt result = 0;

    if (expr->node_kind==_Func_Call &&
	type_of(expr)->type_kind != void_type) {

	/* we have to throw away value */

	result = ignore_result( expr, ctxt );
    }

    if (result) {
	return result;
    } else {
	stmt->stmt.expr = expr;
	return stmt;
    }
}

static stmt_pt
fix_stmt_SList( stmt_pt stmt, ctxt_pt ctxt )
{
    stmt_pt s;
    for (s = stmt; s; s = s->stmt.stmt_list.rest) {
	s->stmt.stmt_list.first = fix_stmt_itself( s->stmt.stmt_list.first );
	
    }
    return stmt;
}

static stmt_pt
fix_post_return_expr( node_pt e, ctxt_pt ctxt )
    /*
     * Fixes a return value that must be followed by post-statements;
     * generates a return statement for the value of the expression.
     */
{
    scope_id_t scope1 = ctxt_scope(ctxt);
    
    /* need a temp variable to hold result */
    node_pt tmpv = 
	new_tmpvar_node( type_of(e), ctxt, e->node_def, 0, FALSE );
    stmt_pt asn_tmpv;
    stmt_pt fstmts;

    set_current_scope(scope1);
    asn_tmpv = new_stmt_Expr(new_pos_node(e->node_def, _Assign, tmpv, e));
    fstmts = combine_stmts(pre_stmts(ctxt), asn_tmpv, post_stmts(ctxt));
    clear_pre_and_post(ctxt);
    append_stmt(fstmts, new_stmt_Return( tmpv->node_def, tmpv ));
    return fstmts;
}

static stmt_pt
fix_stmt_Return( stmt_pt stmt, ctxt_pt ctxt )
{

    node_pt e = stmt->stmt.return_value;
    stmt_pt s1;


    if (e && e->type != type_boolean()) {
	/* if it were boolean, we would know it was already fixed up */
	/* But if it's not boolean, we don't turn it into boolean. */

	symbol_pt func = scope_parent_func(stmt->scope);
	typeinfo_pt return_type = func->sym_type->type_next;
	node_pt e1 = promote( fix_expr( e, ctxt, 0 ), return_type, ctxt);
	func->has_return = TRUE;
	if (changed_post(ctxt)) {
	    s1 = fix_post_return_expr(e1, ctxt);
	    return s1;
	} else {
	    e = e1;
	}
    }
	    
    stmt->stmt.return_value = e;
    return stmt;
}


/* TBD: What's really going to make dealing with control booleans
 * more tractable is to concentrate the logic of allowing pre- and
 * post-statements, and any necessary auxilary functions or (other)
 * declarations, combined in a single place.  This requires auxiliary
 * flags to deal with specifying the allowable pre- and post-
 * positions. I.e. we'll pass a context, and two flags specifying
 * whether pre-statements and post-statements are allowed.  If fixing
 * an expression requires such and they're disallowed, then an auxiliary
 * function is generated hiding the gory details; a call to the function
 * is the fixed expression.
 */
static node_pt
fix_control_expr( node_pt e, ctxt_pt ctxt,
		 boolean control_bool, boolean pre_ok, boolean post_ok)
{
    scope_id_t scope1 = new_block_scope( ctxt_scope(ctxt) );
    ctxt_pt    c1     = new_context(scope1);
    node_pt    e1     = fix_expr(e, c1, control_bool? USE(Is_bool) : 0);
    file_pos_t pos    = e->node_def;
    node_pt    result;

    if ( (!pre_ok  && changed_pre(c1)) ||
	(!post_ok && changed_post(c1))  ) {


	stmt_pt fstmts;
	symbol_pt localfunc;

	/* must create local function */
	if (changed_post(c1)) {
	    fstmts = fix_post_return_expr( e1, c1 );
	} else {
	    fstmts = pre_stmts(c1);
	    set_current_scope(scope1);
	    append_stmt(fstmts, new_stmt_Return( e1->node_def, e1 ));
	}
	localfunc = new_local_func(type_boolean(),
				   decls(c1,0),
				   fstmts,
				   pos,
				   scope1);
	localfunc->has_return = TRUE;
	append_decl(ctxt, localfunc);
	result = new_pos_node(pos,
			      _Func_Call,
			      new_pos_node(pos,_Sym,localfunc),
			      0);
    } else {
	/* can use expression as is; just merge contexts. */
	append_decls(ctxt, decls(c1,0));
	append_pre( ctxt, pre_stmts(c1));
	append_post( ctxt, post_stmts(c1));
	result = e1;
    }
    free_context(c1);
    return result;
}

static stmt_pt
fix_controlled_stmt(stmt_pt s,
		    ctxt_pt ctxt,
		    boolean bool_control, /* whether control expr is boolean*/
		    boolean pre_ok,
		    boolean post_ok)
{
    node_pt expr = 
	fix_control_expr(s->stmt.controlled.expr,
			 ctxt, bool_control, pre_ok, post_ok);
    stmt_pt stmt = fix_stmt_itself( s->stmt.controlled.stmt );
    s->stmt.controlled.expr = expr;
    s->stmt.controlled.stmt = stmt;
    return s;
}

static stmt_pt
fix_stmt_If( stmt_pt s, ctxt_pt ctxt )
{
    return fix_controlled_stmt( s, ctxt, TRUE, TRUE, FALSE );
}

static stmt_pt
fix_stmt_While( stmt_pt s, ctxt_pt ctxt )
{
    return fix_controlled_stmt( s, ctxt, TRUE, FALSE, FALSE );
}

static stmt_pt
fix_stmt_Do( stmt_pt s, ctxt_pt ctxt )
{
    return fix_controlled_stmt( s, ctxt, TRUE, FALSE, TRUE );
}

static stmt_pt
fix_stmt_Switch( stmt_pt s, ctxt_pt ctxt )
{
    node_pt expr = fix_control_expr(s->stmt.controlled.expr,
				    ctxt, FALSE, TRUE, FALSE);
    stmt_pt stmt = s->stmt.controlled.stmt;

    if (stmt->stmt_kind == _Compound &&
	!stmt->stmt.compound.decls   &&
	stmt->stmt.compound.stmts->stmt_kind == _SList) {

	/* This is a "normal" switch statement */
	stmt_pt s1, s2;
	for (s1 = stmt->stmt.compound.stmts;
	     s1;
	     s1 = s1->stmt.stmt_list.rest) {

	    s2 = s1->stmt.stmt_list.first;
	    if (s2->stmt_kind==_Case) {
		/* 
		 * we have to make sure that the expression type
		 * in the case statement matches the type of the
		 * control expression.
		 */
		s2->stmt.label.id = promote(s2->stmt.label.id,
					    type_of(expr),
					    ctxt);
		s2->stmt.label.stmt = fix_stmt_itself(s2->stmt.label.stmt);
		
	    } else {
		s1->stmt.stmt_list.first = fix_stmt_itself( s2 );
	    }
	}
    } else {
	error_at(s->stmt_def, "abnormal switch statement");
	stmt = fix_stmt_itself(stmt);
    }
    s->stmt.controlled.expr = expr;
    s->stmt.controlled.stmt = stmt;
    return s;
}



static stmt_pt
fix_stmt_Ifelse( stmt_pt s, ctxt_pt ctxt )
{
    node_pt expr;
    stmt_pt sIf;
    stmt_pt sElse;

    expr = fix_control_expr(s->stmt.if_else_stmt.expr,
			    ctxt, TRUE,TRUE, FALSE );
    /* TBD: check context for added post_statements */
    sIf   = fix_stmt_itself( s->stmt.if_else_stmt.then_stmt );
    sElse = fix_stmt_itself( s->stmt.if_else_stmt.else_stmt );
    s->stmt.if_else_stmt.expr = expr;
    s->stmt.if_else_stmt.then_stmt = sIf;
    s->stmt.if_else_stmt.else_stmt = sElse;
    return s;
}

static stmt_pt
fix_stmt_For( stmt_pt s, ctxt_pt ctxt )
{
    /*
     * gen_stmt will handle the _For stmt form, so the job of this
     * routine is to make sure that all the expressions are pure
     * and any auxilary statements get moved to the right place.
     * On the other hand, the necessary placement of pre- and post-
     * statements may require reorganization.
     * so for 
     *     for (e1; e2; e3) s;
     * we want:
     *     stmt_expr(e1);                   s1
     *     while ({funcized e2}) {          s2
     *        fixed(s);                     s3
     *        stmt_expr(e3);                s4
     *     }
     */
    node_pt e1,e2;
    stmt_pt s3, sBody, sWhile;
    scope_id_t scope = ctxt_scope(ctxt);

    /* e1 gets turned into pre-statements */
    if (s->stmt.for_stmt.e1) {
	e1 = fix_expr( s->stmt.for_stmt.e1, ctxt, USE(Is_stmt));
	append_pre( ctxt, new_scoped_stmt_Expr( scope, e1 ));
    }
    /* e2 gets turned into control expr in While statement */
    if (s->stmt.for_stmt.e2) {
	e2 = fix_control_expr( s->stmt.for_stmt.e2, ctxt, TRUE, FALSE, FALSE);
    } else {
	e2 = value_boolean(TRUE);
    }

    /* Now we compose the body of the While statement */
    sBody = fix_stmt_itself(s->stmt.for_stmt.stmt);
    s3 = fix_stmt_itself(new_scoped_stmt_Expr(scope, s->stmt.for_stmt.e3));
    sBody = concat_stmts(sBody, s3);

    
    sWhile = (set_current_scope(scope), 
	      new_stmt_While(e2->node_def, 0, e2, sBody));
    return sWhile;

}



static stmt_pt
fix_stmt_Compound( stmt_pt s, ctxt_pt ctxt )
{
    /*
     * The main thing that has to be done with Compound statements
     * is to make sure that any initializers for declarations
     * get performed.
     */
    symbol_pt sym;
    int       scope_id;
    node_pt   expr;
    node_pt   assn;
    stmt_pt   stmt;
    stmt_pt   stmts; /* accumulator for generated statements */
    file_pos_t pos;
    
    stmts = 0;
    for (sym=s->stmt.compound.decls, scope_id = sym?sym->sym_scope_id:0;
	 sym;
	 sym=sym->sym_parse_list) {

	/* make sure that any necessary types get generated */
	if (sym->sym_kind == var_symbol &&
	    sym->sym_type) {

	    all_types_gened(sym->sym_type, sym->sym_def);
	}

	if (sym->sym_kind == var_symbol &&
	    sym->sym_scope_id==scope_id &&
	    !sym->_static &&
	    sym->has_initializer &&
	    !sym->emit_initializer) {

	    assert(sym->sym_kind==var_symbol);
	    expr = sym->sym_value.initializer;
	    pos  = expr->node_def;
	    assn = new_pos_node(pos,
				_Assign,
				new_pos_node(pos,_Sym,sym),
				expr);
	    stmt = new_scoped_stmt_Expr(scope_id, assn);
	    stmt->stmt_def = pos;
	    stmt->scope    = sym->sym_scope_id;
	    stmts = concat_stmts( stmts, fix_stmt_itself(stmt));
	}

    }
    s->stmt.compound.stmts =
	concat_stmts(stmts,
		     fix_stmt_itself(s->stmt.compound.stmts));
	
	
    return s;
}


static stmt_pt
fix_stmt_Labelled( stmt_pt s, ctxt_pt ctxt )
{
    stmt_pt s1 = fix_stmt_itself(s->stmt.label.stmt);
    s->stmt.label.stmt = s1;
    return s;
}

static stmt_pt
fix_stmt_Default( stmt_pt s, ctxt_pt ctxt )
{
    stmt_pt s1 = fix_stmt(s->stmt.default_stmt, ctxt);
    s->stmt.default_stmt = s1;
    return s;
}

static stmt_pt
fix_stmt_trivially( stmt_pt s, ctxt_pt ctxt )
{
    return s;
}    


static stmt_pt
fix_stmt_Unimplemented( stmt_pt stmt, ctxt_pt ctxt )
{
    warning(file_name(stmt->stmt_def), line_number(stmt->stmt_def),
	    "unimplemented statement type (%s) in fix_stmt",
	    nameof_stmt_kind(stmt->stmt_kind));
    return stmt;
}
    
typedef stmt_pt (*fix_stmt_func_t)(stmt_pt stmt, ctxt_pt ctxt);

static fix_stmt_func_t
fix_stmt_func( stmt_pt stmt )
{
    switch (stmt->stmt_kind) {
    case _Return:  return fix_stmt_Return;
    case _Expr:    return fix_stmt_Expr;
    case _SList:   return fix_stmt_SList;
    case _Compound:return fix_stmt_Compound;
    case _If:      return fix_stmt_If;
    case _While:   return fix_stmt_While;
    case _Do:      return fix_stmt_Do;
    case _Switch:  return fix_stmt_Switch;
    case _Ifelse:  return fix_stmt_Ifelse;
    case _For:     return fix_stmt_For;

    case _Labelled:return fix_stmt_Labelled;
    case _Case:    return fix_stmt_Labelled;

    case _Default: return fix_stmt_Default;

    case _Goto:
    case _Continue:
    case _Break:
    case _Null:
	/* TBD: some of these may need more elaborate fixes,
	 * especially if they occur in a For statement that gets
	 * fixed.
	 */
	return fix_stmt_trivially;

    case _FuncDef:
    case _MacroBody:
    default:	
	return fix_stmt_Unimplemented;
    }
}
	    


static stmt_pt 
fix_stmt( stmt_pt stmt, ctxt_pt ctxt)
{
    fix_stmt_func_t func = fix_stmt_func(stmt);
    return (*func)(stmt,ctxt);
}
