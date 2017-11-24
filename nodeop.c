#include <assert.h>
#include <stdarg.h>
#include <memory.h>
#include <sys/types.h>
#include <string.h>

#include "errors.h"
#include "host.h"
#include "files.h"
#include "hash.h"
#include "il.h"
#include "allocate.h"
#include "types.h"
#include "macro.h"

#include "nodeop.h"

#undef NULL
#define NULL	0

#define streq(a,b) (!(strcmp(a,b)))

extern file_pos_t yypos;

static node_t *free_list;

typedef enum {
    _Binary,
    _Unary,
    _Pointer,
    _Other
} node_class_t;

static boolean
is_const_int( node_pt n )
{
    assert(n != NULL);
    switch (n->node_kind) {
    case _Int_Number:
	return TRUE;
    case _Sym:
	assert(n->node.sym);
	return n->node.sym->sym_kind == enum_literal;
    case _Macro_ID:
	{
	    macro_t * m = n->node.macro;
	    return m->macro_evald && m->const_value.eval_result_kind==eval_int;
	}
    default:
	return FALSE;
    }
}

static boolean is_const_fp ( node_pt n )
{
    assert( n != NULL );
    switch (n->node_kind) {
    case _FP_Number:
	return TRUE;
    case _Macro_ID:
	{
	    macro_t * m = n->node.macro;
	    return m->macro_evald &&
		m->const_value.eval_result_kind==eval_float;
	}
    default:
	return FALSE;
    }
}

static host_int_t
const_int_val(n)
    node_t *n;
{
    assert(is_const_int(n));
    switch (n->node_kind) {
    case _Int_Number:
	return n->node.ival;
    case _Sym:
	return n->node.sym->sym_value.intval;
    case _Macro_ID:
	return n->node.macro->const_value.eval_result.ival;
    default:
	assert(0);
	return 0; /* bogus return to prevent compilation warning */
    }
}

#define is_constant(x)			(is_const_int(x) || is_const_fp(x))

void
free_node(n)
    node_t *n;
{
    assert(n->node_kind != _Ident);
    n->node_kind = 0;
    n->node.unary = free_list;
    free_list = n;
}

static node_t*
alloc_node(kind)
    node_kind_t kind;
{
    node_t *n;
    int i;

    n = free_list;
    if (n == NULL) {
	n = (node_t*) allocate(sizeof(node_t)*64);
	for (i = 1; i<63; i++) {
	    n[i].node.unary = &n[i+1];
	}
	free_list = &n[1];
    }
    else {
	free_list = n->node.unary;
	memset(n, 0, sizeof(node_t));
    }

    n->node_def = yypos;
    n->node_kind = kind;
    return n;
}

node_class_t
node_classof(node_kind_t kind)
{
    if (kind >= _List && kind <= _Shr) {
	return _Binary;
    }

    if (kind >= _Sizeof && kind <= _Indirect) {
	return _Unary;
    }

    switch (kind) {
    case _Exp:
    case _Concat:
	return _Binary;
    case bool:
    case _UnBool:
    case _Char_to_Int:
    case _Int_to_Char:
	return _Unary;
    case _Ident:
	return _Pointer;
    default:
	break;
    }

    return _Other;
}


node_kind_t
non_assign_op( node_kind_t op )
    /* returns the non-assign operator corresponding to an assign-op */
{
    switch (op) {
    case _Mul_Assign:	return _Mul;
    case _Div_Assign:	return _Div;
    case _Mod_Assign:	return _Rem;
    case _Add_Assign:   return _Add;
    case _Sub_Assign:   return _Sub;
    case _Shl_Assign:   return _Shl;
    case _Shr_Assign:   return _Shr;
    case _Band_Assign:  return _Band;
    case _Xor_Assign:   return _Xor;
    case _Bor_Assign:   return _Bor;
    default:
	return _Error;
    }
}


/* operations on node-linked lists */

node_pt
reshape_list(node_pt e)
    /* turn left-recursive list into right-recursive list:
     * (((a,b),c),d) => (a,(b,(c,d)))
     * The parser builds left-recursive lists, but right-recursive
     * is more convenient in later phases of the translation.
     */
{
    node_pt top = e;
    while (top->node_kind==_List && top->node.binary.l->node_kind==_List) {
	node_pt tmp = top;
	node_pt el, er;

	top = top->node.binary.l;
	el = top->node.binary.l;
	er = top->node.binary.r;
	tmp->node.binary.l = er;
	top->node.binary.r = tmp;
    }
    return top;
}

/* Node iteration.
 * init_node_iter initializes an iterator.
 * next_list_ref returns the head of the list,
 * and modifies the iterator so the next call returns the next element.
 * next_list_ref returns 0 on end of list.
 */

node_iter_t
init_node_iter( node_pt * n )
{
    node_iter_t result;
    result.head = 0; /* will be filled in later */
    result.rest = n;
    return result;
}

node_pt *
next_list_ref( node_iter_t *iter)
{
    if (!iter->rest) {
	return 0;
    } else {
	node_pt rest = *iter->rest;
	if (rest->node_kind==_List) {
	    iter->head = &rest->node.binary.l;
	    iter->tail = iter->rest;
	    iter->rest = &rest->node.binary.r;
	    return iter->head;
	} else {
	    iter->head = iter->rest;
	    iter->tail = iter->rest;
	    iter->rest = 0;
	    return iter->head;
	}
    }
} /* next_list_ref */

node_pt *
node_iter_tail(node_iter_t * iter)
{
    return iter->tail;
}


boolean
is_null_ptr_value( node_pt e )
{
    if (e->node_kind==_Int_Number && e->node.ival==0) return TRUE;
    if (e->node_kind==_Macro_ID && streq(e->node.macro->macro_name,"NULL")) {
	return TRUE;
    }
    return FALSE;
}


static node_pt
new_node_v(node_kind_t kind, va_list args)
{
    node_t *n;

    n = alloc_node(kind);

    switch (node_classof(kind)) {
    case _Binary:
	n->node.binary.l = va_arg(args, node_t*);
	n->node.binary.r = va_arg(args, node_t*);
	break;
    case _Unary:
	n->node.unary = va_arg(args, node_t*);
	break;
    case _Pointer:
	n->node.id.name = va_arg(args, char*);
	break;
    case _Other:
	switch (kind) {
	case _Ellipsis:
	    break;
	case _String:
	    n->node.str.form = va_arg(args, char*);
	    n->node.str.len = va_arg(args, int);
	    break;
	case _Sym:
	    n->node.sym = va_arg(args, symbol_t*);
	    assert(n->node.sym != 0 );
	    break;
	case _Macro_ID:
	    n->node.macro = va_arg(args, macro_t*);
	    break;
	case _Type:
	    n->node.typ = va_arg(args, typeinfo_t*);
	    break;
	case _Cond:
	    n->node.cond.bool = va_arg(args, node_t*);
	    n->node.cond.tru = va_arg(args, node_t*);
	    n->node.cond.fals = va_arg(args, node_t*);
	    break;
	case _FP_Number:
	    n->node.fval = va_arg(args, host_float_t);
	    break;
	case _Int_Number:
	    n->node.ival = va_arg(args, host_int_t);
	    n->baseval = va_arg(args, host_int_t);
	    break;
	default:
	    fatal(__FILE__,__LINE__,"Unandled noded - (%d)", kind);
	    break;
	}
	break;
    default:
	assert(0);
	break;
    }

    return n;
}

node_pt
new_node(node_kind_t kind, ...)
{
    va_list args;
    node_pt result;

    va_start(args,kind);
    result = new_node_v(kind, args);
    va_end(args);
    return result;
}

node_pt
new_pos_node(file_pos_t pos, node_kind_t kind, ...)
{
    va_list args;
    node_pt result;

    va_start(args,kind);
    result = new_node_v(kind, args);
    va_end(args);

    result->node_def = pos;
    return result;
}


static void
promote(n)
    node_t *n;
{
    node_t *l, *r;
    host_float_t fv;

    assert(n != NULL);

    l = n->node.binary.l;
    r = n->node.binary.r;

    assert(l != NULL);
    assert(r != NULL);

    if (is_const_int(l) && is_const_fp(r)) {
	fv = (host_float_t) const_int_val(l);
	l->node_kind = _FP_Number;
	l->node.fval = fv;
    }
    else if (is_const_fp(l) && is_const_int(r)) {
	fv = (host_float_t) const_int_val(r);
	r->node_kind = _FP_Number;
	r->node.fval = fv;
    }
}

static void reduce_binary();

/* Force constants to RHS */
static void
distributive(n)
    node_t *n;
{
    node_t *l, *r, *dr;

    assert(n != NULL);

    l = n->node.binary.l;	assert(l != NULL);
    r = n->node.binary.r;	assert(r != NULL);

    if (is_constant(l) && !is_constant(r)) {
	n->node.binary.l = r;
	n->node.binary.r = l;
    }

    if (is_constant(r) && l->node_kind == n->node_kind) {
	dr = l->node.binary.r;
	if (is_constant(dr)) {
	    /* Transformation
	     *
	     *		 (n)		(n)
	     *		/   \	   /  \
	     *	  (l)   (C)	 (?)  (l)
	     *	 /  \			 /	 \
	     * (?)  (C)		   (C)	 (C)
	     */

	    n->node.binary.l = l->node.binary.l;
	    n->node.binary.r = l;
	    l->node.binary.l = dr;
	    l->node.binary.r = r;
	    reduce_binary(l);
	}
    }
}

static void
reduce_binary(n)
    node_t *n;
{
    node_t *l, *r;

    if (n == NULL) return;

    switch (n->node_kind) {
    case _Add:
	distributive(n);
	promote(n);
	l = n->node.binary.l;	assert(l != NULL);
	r = n->node.binary.r;	assert(r != NULL);
	if (is_constant(l)) {
	    assert(is_constant(r)); /* because distributive() */
	    if (is_const_int(l)) {
		n->node_kind = _Int_Number;
		n->node.ival = const_int_val(l) +
		    const_int_val(r);
	    }
	    else {
		n->node_kind = _FP_Number;
		n->node.fval = l->node.fval + r->node.fval;
	    }
	    free_node(l); free_node(r);
	}
	break;
    case _Sub:
	promote(n);
	l = n->node.binary.l;	assert(l != NULL);
	r = n->node.binary.r;	assert(r != NULL);
	if (is_constant(l) && is_constant(r)) {
	    if (is_const_int(l)) {
		n->node_kind = _Int_Number;
		n->node.ival = const_int_val(l) -
		    const_int_val(r);
	    }
	    else {
		n->node_kind = _FP_Number;
		n->node.fval = l->node.fval - r->node.fval;
	    }
	    free_node(l); free_node(r);
	}
	break;
    case _Mul:
	distributive(n);
	promote(n);
	l = n->node.binary.l;	assert(l != NULL);
	r = n->node.binary.r;	assert(r != NULL);
	if (is_constant(l)) {
	    assert(is_constant(r)); /* because distributive() */
	    if (is_const_int(l)) {
		n->node_kind = _Int_Number;
		n->node.ival = const_int_val(l) *
		    const_int_val(r);
	    }
	    else {
		n->node_kind = _FP_Number;
		n->node.fval = l->node.fval * r->node.fval;
	    }
	    free_node(l); free_node(r);
	}
	break;
    case _Div:
	promote(n);
	l = n->node.binary.l;	assert(l != NULL);
	r = n->node.binary.r;	assert(r != NULL);
	if (is_constant(l) && is_constant(r)) {
	    if (is_const_int(l)) {
		if (const_int_val(r) != 0) {
		    n->node_kind = _Int_Number;
		    n->node.ival = const_int_val(l) /
			const_int_val(r);
		    free_node(l); free_node(r);
		}
	    }
	    else {
		if (r->node.fval != 0.0) {
		    n->node_kind = _FP_Number;
		    n->node.fval = l->node.fval / r->node.fval;
		    free_node(l); free_node(r);
		}
	    }
	}
	break;
    case _Rem:
	l = n->node.binary.l;	assert(l != NULL);
	r = n->node.binary.r;	assert(r != NULL);
	if (is_const_int(l) && is_const_int(r)) {
	    if (const_int_val(r) != 0) {
		n->node_kind = _Int_Number;
		n->node.ival = const_int_val(l) %
		    const_int_val(r);
		free_node(l); free_node(r);
	    }
	}
	break;
    case _Shl:
	l = n->node.binary.l;	assert(l != NULL);
	r = n->node.binary.r;	assert(r != NULL);
	if (is_const_int(l) && is_const_int(r)) {
	    n->node_kind = _Int_Number;
	    n->node.ival = const_int_val(l) << const_int_val(r);
	    free_node(l);
	    free_node(r);
	}
	break;
    case _Shr:
	l = n->node.binary.l;	assert(l != NULL);
	r = n->node.binary.r;	assert(r != NULL);
	if (is_const_int(l) && is_const_int(r)) {
	    n->node_kind = _Int_Number;
	    n->node.ival = const_int_val(l) >> const_int_val(r);
	    free_node(l);
	    free_node(r);
	}
	break;
    case _Bor:
	distributive(n);
	l = n->node.binary.l;	assert(l != NULL);
	r = n->node.binary.r;	assert(r != NULL);
	if (is_const_int(l) && is_const_int(r)) {
	    n->node_kind = _Int_Number;
	    n->node.ival = const_int_val(l) | const_int_val(r);
	    free_node(l);
	    free_node(r);
	}
	break;
    case _Band:
	distributive(n);
	l = n->node.binary.l;	assert(l != NULL);
	r = n->node.binary.r;	assert(r != NULL);
	if (is_const_int(l) && is_const_int(r)) {
	    n->node_kind = _Int_Number;
	    n->node.ival = const_int_val(l) & const_int_val(r);
	    free_node(l);
	    free_node(r);
	}
	break;
    case _Xor:
	distributive(n);
	l = n->node.binary.l;	assert(l != NULL);
	r = n->node.binary.r;	assert(r != NULL);
	if (is_const_int(l) && is_const_int(r)) {
	    n->node_kind = _Int_Number;
	    n->node.ival = const_int_val(l) ^ const_int_val(r);
	    free_node(l);
	    free_node(r);
	}
	break;
    default:
        break;
    }
}

static void
reduce_unary(n)
    node_t *n;
{
    node_t *l;
    typeinfo_t *typ;
    symbol_t *sym;
    host_int_t iv;

    assert(n != NULL);

    l = n->node.unary;
    if (l == NULL) return;

    switch (n->node_kind) {
    case _Sizeof:
	switch (l->node_kind) {
	case _Type:
	    typ = l->node.typ;
	    break;
	case _Sym:
	    sym = l->node.sym;
	    if (sym == NULL) return;
	    typ = sym->sym_type;
	    break;
	default:
	    return;
	}
	if (typ == NULL) return;
	iv = type_sizeof(typ);
	free_node(l);
	n->node_kind = _Int_Number;
	n->node.ival = iv;
	break;
    case _Unary_Plus:
	switch (l->node_kind) {
	case _Int_Number:
	    n->node_kind = _Int_Number;
	    n->node.ival = const_int_val(l);
	    free_node(l);
	    break;
	case _FP_Number:
	    n->node_kind = _FP_Number;
	    n->node.fval = l->node.fval;
	    free_node(l);
	    break;
        default:
            break;
	}
	break;
    case _Unary_Minus:
	switch (l->node_kind) {
	case _Int_Number:
	    n->node_kind = _Int_Number;
	    n->node.ival = -const_int_val(l);
	    free_node(l);
	    break;
	case _FP_Number:
	    n->node_kind = _FP_Number;
	    n->node.fval = -l->node.fval;
	    free_node(l);
	    break;
        default:
            break;
	}
	break;
    case _Ones_Complement:
	if (l->node_kind == _Int_Number) {
	    n->node_kind = _Int_Number;
	    n->node.ival = ~const_int_val(l);
	    free_node(l);
	}
	break;
    case _Not:
	if (l->node_kind == _Int_Number) {
	    n->node_kind = _Int_Number;
	    n->node.ival = !const_int_val(l);
	    free_node(l);
	}
	break;
    case _Pre_Inc:
    case _Pre_Dec:
    case _Post_Inc:
    case _Post_Dec:
    case _Addrof:
    case _Aggregate:
    case _Indirect:
	break;
    default:
         break;
    }
}

void
reduce_node(node_pt n)
    /* reduces a node as much as possible to a constant expression */
{
    if (n == NULL) return;

    switch (node_classof(n->node_kind)) {
    case _Binary:
	reduce_node(n->node.binary.l);
	reduce_node(n->node.binary.r);
	reduce_binary(n);
	break;
    case _Unary:
	reduce_node(n->node.unary);
	reduce_unary(n);
	break;
    default:
	if (n->node_kind!=_Int_Number && is_const_int(n)) {
	    host_int_t val = const_int_val(n);
	    n->node_kind = _Int_Number;
	    n->node.ival = val;
	}
	return;
    }
}

node_t*
access_to(ptr, decl)
    node_t *ptr, *decl;
{
    node_t *n;

    assert(ptr != NULL);
    assert(decl != NULL);
    assert(ptr->node_kind == _Indirect);

    for (n = ptr; n; ) {
	switch (n->node_kind) {
	case _Indirect:
	    if (n->node.unary == NULL) {
		n->node.unary = decl;
		return ptr;
	    }
	    n = n->node.unary;
	    break;
	default:
	    fatal(__FILE__,__LINE__,"Unhandled node (%d)", n->node_kind);
	    break;
	}
    }

    assert(0);
    return ptr;
}

node_t*
id_from_typedef(typ)
    typeinfo_t *typ;
{
    symbol_t *basetype;

    assert(typ != NULL);

    basetype = typ->type_base;
    assert(basetype != NULL);

    assert(basetype->sym_ident != NULL);
    assert(basetype->sym_ident->node_kind == _Ident);
    assert(basetype->sym_ident->node.id.name != NULL);
    return new_node(_Ident, basetype->sym_ident->node.id.name);
}

