/* $Source: /home/CVSROOT/c2ada/gen_stmt.c,v $ */
/* $Revision: 1.3 $  $Date: 1999/02/09 18:16:51 $ */

/*
 * This module primarily implements gen_stmt(), a routine to print
 * out a statement tree as Ada statements.  It is assumed that all 
 * the statement forms encountered are directly representable in Ada:
 * an earlier phase (fix_stmt) is responsible for transforming any C
 * statements, and the expressions therein, into a suitable form.
 */

#include <assert.h>
#include <stdio.h>

#include "errors.h"
#include "format.h"
#include "gen_stmt.h"
#include "gen_expr.h"
#include "print.h"
#include "gen.h"
#include "allocate.h"
#include "anonymous.h"
#include "types.h"
#include "package.h"

/********************************************
 * stmt, stmt_Slist, Compound, Expr, and Null
 ********************************************/

static void
do_null(indent, pos)
    int indent;
    file_pos_t pos;
{
    indent_to(indent);
    put_string("null;");
    print_position(pos);
}

static void
gen_stmt_SList(stmt_pt s, int indent)
{
    int first_indent, next_indent;
    /* 
     * TBD: make this iterative so long lists won't blow
     * us out of the water.
     */
    if (s == NULL) return;
    first_indent = next_indent = indent;
    /* if((s->stmt.stmt_list.first != NULL) &&
       (s->stmt.stmt_list.first->stmt_kind == _Case)) {
       first_indent -= 4;
       next_indent  += 4;
    } */
    gen_stmt(s->stmt.stmt_list.first, first_indent);
    gen_stmt(s->stmt.stmt_list.rest,  next_indent);
}

static void
must_do_slist(stmt_pt s, int indent, file_pos_t pos)
{
    if (s == NULL)
	do_null(indent, pos);
    else
	gen_stmt(s, indent);
}

static void 
gen_stmt_Compound(stmt_pt s, int indent)
{
    symbol_pt sym;
    int       scope_id;

    if (s == NULL) return;
    if (s->stmt.compound.decls) {
	indent_to(indent);
	put_string("declare\n");
	for (sym=s->stmt.compound.decls, scope_id = sym->sym_scope_id; 
	     sym; 
	     sym=sym->sym_parse_list) {

	    if (scope_id != sym->sym_scope_id) continue;

	    if (sym->sym_kind==func_symbol && sym->sym_value.body) {

		gen_local_func(sym, indent+4);

	    } else if (sym->sym_kind==pkg_symbol) {

		gen_pkg_def( sym, indent+4 );

	    } else if (sym->sym_kind==var_symbol  &&
		       !sym->_static) {
		
		gen_var_or_field(sym, indent+4, indent+16, -1, NULL, 0);

	    }
	}
	indent_to(indent);
	put_string("begin\n");
	must_do_slist(s->stmt.compound.stmts, indent+4, s->stmt_def);
	indent_to(indent);
	put_string("end;");
	print_position(s->stmt_def);
    } else {
	/*
	 * If there are no decls, don't bother to output an 
	 * Ada begin-end.  If we do, we get things like
	 * if ... then begin ... end; end if;
	 */
	must_do_slist(s->stmt.compound.stmts, indent, s->stmt_def);
    }
}

static void 
gen_stmt_Expr(stmt_pt s, int indent)
{
    node_pt e;
    boolean bracket_call = FALSE;
    static char * c_call_name;

    if (!c_call_name) {
	c_call_name = predef_name_copy("call");
    }

    if ((s == NULL) || (!(e=s->stmt.expr))) return;
    if (e->node_kind==_Func_Call) {
	typeinfo_pt type = e->type;
	bracket_call = !type || !equal_types(type,typeof_void());
    }
	
    indent_to(indent);
    if (bracket_call) {
	put_string(c_call_name);
	put_string("( ");
    }
    gen_expr(s->stmt.expr, 0);
    if (bracket_call) {
	put_string(" )");
    }
    put_string(";");
    print_position(s->stmt_def);
}

static void 
gen_stmt_Null (stmt_pt s, int indent)
{
    if (s == NULL) return;
    do_null(indent, s->stmt_def);
}

static void 
gen_stmt_MacroBody (stmt_pt s, int indent)
{
    if (s == NULL) return;
    indent_to(indent);
    put_string(s->stmt.macro_body);
    print_position(s->stmt_def);
}

/***************
 * If and Ifelse
 ***************/

static void 
gen_stmt_If (stmt_pt s, int indent)
{
    if (s == NULL) return;
    indent_to(indent);
    put_string("if ");
    gen_expr(s->stmt.controlled.expr, 0);
    put_string(" then");
    print_position(s->stmt_def);
    gen_stmt(s->stmt.controlled.stmt, indent+4);    
    indent_to(indent);
    put_string("end if;\n");
}

static void 
gen_stmt_Ifelse (stmt_pt s, int indent)
{
    if (s == NULL) return;
    indent_to(indent);
    put_string("if ");
    gen_expr(s->stmt.if_else_stmt.expr, 0);
    put_string(" then");
    print_position(s->stmt_def);
    gen_stmt(s->stmt.if_else_stmt.then_stmt, indent+4);    
    indent_to(indent);
    put_string("else\n");
    gen_stmt(s->stmt.if_else_stmt.else_stmt, indent+4);    
    indent_to(indent);
    put_string("end if;\n");
}

/********************************************
 * common code for "continue" and "break",
 * getting out of loops and switch statements
 ********************************************/

static void enter_loop();
static void enter_switch();
static void leave_loop();
static void leave_switch();
static int is_in_loop();
static int is_in_loop_or_switch();
static int loop_is_most_local_nesting();
static char *break_label(file_pos_t pos);
static char *continue_label(file_pos_t pos);
static void gen_continue_label(int indent);
static void gen_break_label(int indent);

#define MAX_NESTING 100			/* loops and switches */
static int cur_nesting_level = -1;
static struct nesting_info_struct {
    int is_loop;
    char *label;
} nesting_info[MAX_NESTING];

static void enter_loop()
{
    assert(cur_nesting_level < MAX_NESTING);
    ++cur_nesting_level;
    nesting_info[cur_nesting_level].is_loop = 1;
}

static void enter_switch()
{
    assert(cur_nesting_level < MAX_NESTING);
    ++cur_nesting_level;
    nesting_info[cur_nesting_level].is_loop = 0;
}

static void leave_loop()
{
    assert((cur_nesting_level >= 0) &&
           (nesting_info[cur_nesting_level].is_loop));
    if (nesting_info[cur_nesting_level].label != NULL)
	deallocate(nesting_info[cur_nesting_level].label);
    --cur_nesting_level;
}

static void leave_switch()
{
    assert((cur_nesting_level >= 0) &&
           (!nesting_info[cur_nesting_level].is_loop));
    if (nesting_info[cur_nesting_level].label != NULL)
	deallocate(nesting_info[cur_nesting_level].label);
    --cur_nesting_level;
}

static int is_in_loop()
{
    int i;
    for(i=cur_nesting_level; i>=0; i--)
	if (nesting_info[cur_nesting_level].is_loop)
	    return 1;
    return 0;
}

static int is_in_loop_or_switch()
{
    return (cur_nesting_level >= 0);
}

static int loop_is_most_local_nesting()
{
    return((cur_nesting_level >= 0) && 
	   nesting_info[cur_nesting_level].is_loop);
}

static char *get_label(int level, char *s, file_pos_t pos)
{
    char buf[100];

    assert(level >= 0);
    if (nesting_info[level].label == NULL) {
	sprintf(buf, "%s%d", s, (int) line_number(pos));
	nesting_info[level].label = new_string(buf);
    }
    return nesting_info[level].label;
}

static char *break_label(file_pos_t pos)
{
    return(get_label(cur_nesting_level, "break", pos));
}

static char *continue_label(file_pos_t pos)
{
    int i;

    for(i=cur_nesting_level; (i>=0) && (!nesting_info[i].is_loop); i--)
	;
    return(get_label(i, "continue", pos));
}

static void do_label(char *label, int indent)
{
    indent_to(indent-4);
    put_string("<<");
    put_string(label);
    put_string(">>");
}

static void gen_continue_or_break_label(int is_loop, int indent)
{
    assert((cur_nesting_level >= 0) &&
           (nesting_info[cur_nesting_level].is_loop == is_loop));
    if (nesting_info[cur_nesting_level].label != NULL) {
	do_label(nesting_info[cur_nesting_level].label, indent);
	deallocate(nesting_info[cur_nesting_level].label);
	nesting_info[cur_nesting_level].label = NULL;
	put_string("\n");
	indent_to(indent);
	put_string("null;\n");
    }
}

static void gen_continue_label(int indent)
{
    gen_continue_or_break_label(1, indent);
}

static void gen_break_label(int indent)
{
    gen_continue_or_break_label(0, indent);
}

/***************************
 * Switch, Case, and Default
 ***************************/

case_alist_pt
new_case_alist()
{
    return allocate(sizeof(case_alist));
}

case_slist_pt
new_case_slist()
{
    return allocate(sizeof(case_slist));
}

case_blist_pt
new_case_blist()
{
    return allocate(sizeof(case_blist));
}

case_stmt_pt
new_case_stmt()
{
    return allocate(sizeof(case_stmt));
}

static void 
free_case_stmt(case_stmt_pt cp)
{
    case_alist_pt ap, next_ap;
    case_slist_pt lp, next_lp;
    case_blist_pt bp, next_bp;

    for(bp = cp->branches; bp != NULL; bp = next_bp) {
	for(ap = bp->alts; ap != NULL; ap = next_ap) {
	    next_ap = ap->rest;
	    deallocate(ap);
	}
	for(lp = bp->stms; lp != NULL; lp = next_lp) {
	    next_lp = lp->rest;
	    deallocate(lp);
	}
	next_bp = bp->rest;
	deallocate(bp);
    }
    deallocate(cp);
    
}

static case_blist_pt
add_to_blist(case_stmt_pt cp, case_blist_pt bp)
{
    case_blist_pt new_bp;

    new_bp = new_case_blist();
    if (cp->branches == NULL)
	cp->branches = new_bp;
    else
	bp->rest = new_bp;
    return new_bp;
}

static case_alist_pt
add_to_alist(case_blist_pt bp, case_alist_pt ap, node_pt exp)
{
    case_alist_pt new_ap;

    new_ap = new_case_alist();
    new_ap->exp = exp;
    if (bp->alts == NULL)
	bp->alts = new_ap;
    else
	ap->rest = new_ap;
    return new_ap;
}

static case_slist_pt
add_to_slist(case_blist_pt bp, case_slist_pt sp, stmt_pt stm)
{
    case_slist_pt new_sp;

    new_sp = new_case_slist();
    new_sp->stm = stm;
    if (bp->stms == NULL)
	bp->stms = new_sp;
    else
	sp->rest = new_sp;
    return new_sp;
}

static void 
set_default_branch(case_stmt_pt cp, case_blist_pt bp, 
		   int indent, file_pos_t pos)
{
    if (cp->default_branch != NULL) {
	indent_to(indent);
	put_string("{warning: switch with multiple defaults}");
	print_position(pos);
    }
    bp->has_default = 1;
    cp->default_branch = bp;
}

/*
 * process a case ... case list
 * return pointer to first non-case non-default statement
 */
static stmt_pt 
build_alist(node_pt n1, stmt_pt s, case_stmt_pt cp, 
	    case_blist_pt bp, int indent)
{
    case_alist_pt ap;
    stmt_pt s1;

    ap = add_to_alist(bp, NULL, n1);

    for(s1 = s; s1 != NULL; ) {
	switch(s1->stmt_kind) {
	    case _Case:
		ap = add_to_alist(bp, ap, s1->stmt.label.id);
		s1 = s1->stmt.label.stmt;
		break;
	    case _Default:
		set_default_branch(cp, bp, indent, s1->stmt_def);
		ap = add_to_alist(bp, ap, NULL);
		s1 = s1->stmt.default_stmt;
		break;
	    default:
		return s1;
	}
    }
    return NULL;
}

static case_stmt_pt 
build_switch_stmt(node_pt exp, stmt_pt s, int indent)
{
    case_stmt_pt  cp;
    case_blist_pt bp = NULL;
    case_slist_pt sp;
    stmt_pt first, next, rest;

    cp = new_case_stmt();
    cp->exp = exp;

    first = s->stmt.stmt_list.first;
    rest  = s->stmt.stmt_list.rest;

    for(;;) {
	if (first == NULL)
	    break;
	switch(first->stmt_kind) {
	    case _Case:
		if (bp != NULL)
		    bp->last_stmt = sp->stm;
		bp = add_to_blist(cp, bp);
		next = build_alist(first->stmt.label.id, 
				   first->stmt.label.stmt, cp, bp, indent);
		sp = add_to_slist(bp, NULL, next);
		break;
	    case _Default:
		if (bp != NULL)
		    bp->last_stmt = sp->stm;
		bp = add_to_blist(cp, bp);
		set_default_branch(cp, bp, indent, sp->stm->stmt_def);
		next = first->stmt.default_stmt;
		sp = add_to_slist(bp, NULL, next);
		break;
	    default:
		sp = add_to_slist(bp, sp, first);
		break;
	}
	if (rest == NULL)
	    break;
	first = rest->stmt.stmt_list.first;
	rest  = rest->stmt.stmt_list.rest;
    }
    return cp;
}

static void
gen_case_slist(case_slist_pt sp, int indent)
{
    if ((sp != NULL) && (sp->stm->stmt_kind == _Break) && (sp->rest == NULL)) {
	indent_to(indent);
	put_string("null;\n");
    } else {
	for(; sp != NULL; sp = sp->rest) 
	    if ((sp->stm->stmt_kind != _Break) || (sp->rest != NULL))
		gen_stmt(sp->stm, indent);
    }
}

static int 
fallthrough(stmt_pt s)		/* fall through to next case? */
{
    if (s == NULL)
	return 1;
    switch(s->stmt_kind) {
	case _Goto:
	case _Break:
	case _Return:
	case _Continue:
	    return 0;
	default:
	    return 1;
    }
}

static void 
gen_stmt_Switch (stmt_pt s, int indent)
{
    int debug = 0;
    case_stmt_pt  cp;
    case_blist_pt bp, next_bp;
    case_alist_pt ap;

    stmt_pt next;
    int first_alt;
    file_pos_t first_pos;

    if (s == NULL) return;
    next = s->stmt.controlled.stmt;
    assert((next != NULL) &&
	   (next->stmt_kind == _Compound) &&
	   (next->stmt.compound.stmts != NULL) &&
	   (next->stmt.compound.stmts->stmt_kind == _SList) &&
	   (next->stmt.compound.decls == NULL));

    cp = build_switch_stmt(s->stmt.controlled.expr, 
			   next->stmt.compound.stmts, indent);
    if (debug)
	print_case_stmt(cp, indent);

    enter_switch();
    if (debug) print_stmt(s, indent);
    indent_to(indent);
    put_string("case ");
    gen_expr(cp->exp, 0);
    put_string(" is");
    print_position(s->stmt_def);

    for(bp=cp->branches; bp != NULL; bp = bp->rest) {
	/* generate default last in Ada, even if not last in C */
	if (bp != cp->default_branch) {
	    first_alt = 1;
	    for(ap=bp->alts; ap != NULL; ap = ap->rest) {
		if (first_alt) {
		    indent_to(indent+4);
		    put_string("when ");
		    first_alt = 0;
		    first_pos = ap->exp->node_def;
		} else 
		    put_string(" | ");
		gen_expr(ap->exp, 0);
	    }
	    if (bp->alts != NULL) {
		put_string(" =>");
		print_position(first_pos);
	    }

	    for(next_bp=bp; next_bp!=NULL; next_bp=next_bp->rest) {
		gen_case_slist(next_bp->stms, indent+8);
		if ((next_bp->rest != NULL) &&
		   (fallthrough(next_bp->last_stmt))) {
		    indent_to(indent+8);
		    put_string("-- warning - duplicated code in case\n");
		} else {
		    break;
		}
	    }
	}
    }

    /* this isn't always correct but I think it's best on balance */
    indent_to(indent+4);
    put_string("when others =>\n");
    indent_to(indent+8);
    if (cp->default_branch == NULL)
	put_string("null;\n");
    else 
	gen_case_slist(cp->default_branch->stms, indent+8);

    indent_to(indent);
    put_string("end case;\n");
    gen_break_label(indent);
    leave_switch();
}

static void 
gen_stmt_Case (stmt_pt s, int indent)
{
    assert(0);
}

static void 
gen_stmt_Default (stmt_pt s, int indent)
{
    assert(0);
}


/*******
 * loops
 *******/

static void 
basic_loop(stmt_pt s1, stmt_pt s2, int indent, file_pos_t pos)
{
    indent_to(indent);
    put_string("loop");
    print_position(pos);
    gen_stmt(s1, indent+4);
    gen_continue_label(indent+4);
    gen_stmt(s2, indent+4);
}

static void 
while_or_do_loop(node_pt expr, stmt_pt s1, stmt_pt s2, 
		 int indent, file_pos_t pos, int is_while)
{
    enter_loop();
    if (expr == NULL) {
	basic_loop(s1, s2, indent, pos);
    } else if (is_while) {
	indent_to(indent);
	put_string("while ");
	gen_expr(expr, 0);
	put_string(" loop");
	print_position(pos);
	gen_stmt(s1, indent+4);
	gen_continue_label(indent+4);
	gen_stmt(s2, indent+4);
    } else {
	/* do loop */
	indent_to(indent);
	put_string("loop");
	print_position(pos);
	gen_stmt(s1, indent+4);
	gen_continue_label(indent+4);
	/* gen_stmt(s2, indent+4); not needed in "do" */
	indent_to(indent+4);
	put_string("exit when not ");
	gen_expr(expr, 1);
	print_position(pos);
    }
    indent_to(indent);
    put_string("end loop;\n");
    leave_loop();
}

static void 
gen_stmt_For (stmt_pt s, int indent)
{
    stmt_pt e3;

    if (s == NULL) return;
    if (s->stmt.for_stmt.e1 != NULL)
	gen_stmt_Expr(new_stmt_Expr(s->stmt.for_stmt.e1), indent);
    if (s->stmt.for_stmt.e3 == NULL)
	e3 = NULL;
    else
	e3 = new_stmt_Expr(s->stmt.for_stmt.e3);
    while_or_do_loop(s->stmt.for_stmt.e2, s->stmt.for_stmt.stmt, e3, 
		     indent, s->stmt_def, 1);
}

static void 
gen_stmt_While (stmt_pt s, int indent)
{
    if (s == NULL) return;
    while_or_do_loop(s->stmt.controlled.expr, s->stmt.controlled.stmt, NULL, 
		     indent, s->stmt_def, 1);
}

static void 
gen_stmt_Do (stmt_pt s, int indent)
{
    if (s == NULL) return;
    while_or_do_loop(s->stmt.controlled.expr, s->stmt.controlled.stmt, NULL, 
		     indent, s->stmt_def, 0);
}

/*********************************
 * Labelled, Goto, Continue, Break
 *********************************/

static void 
gen_stmt_Labelled (stmt_pt s, int indent)
{
    if (s == NULL) return;
    do_label(s->stmt.label.id->node.id.name, indent);
    print_position(s->stmt_def);
    gen_stmt(s->stmt.label.stmt, indent);
}

static void
do_goto(char *label, int indent, file_pos_t pos)
{
    indent_to(indent);
    put_string("goto ");
    put_string(label);
    put_string(";");
    print_position(pos);
}

static void 
gen_stmt_Goto (stmt_pt s, int indent)
{
    if (s == NULL) return;
    do_goto(s->stmt.goto_label->node.id.name, indent, s->stmt_def);
}

static void 
gen_stmt_Continue (stmt_pt s, int indent)
{

    if (s == NULL) return;
    if (!is_in_loop()) {
	indent_to(indent);
	put_string("{warning: continue not inside loop}");
	print_position(s->stmt_def);
    } else {
	do_goto(continue_label(s->stmt_def), indent, s->stmt_def);
    }
}

static void 
gen_stmt_Break (stmt_pt s, int indent)
{
    if (s == NULL) return;
    indent_to(indent);
    if (!is_in_loop_or_switch()) {
	put_string("{warning: break not inside loop or switch}");
	print_position(s->stmt_def);
    } else {
	if (loop_is_most_local_nesting()) {
	    put_string("exit;");
	    print_position(s->stmt_def);
	} else {
	    do_goto(break_label(s->stmt_def), indent, s->stmt_def);
	}
    }
}

/********************
 * FuncDef and Return
 ********************/

static void 
gen_stmt_FuncDef (stmt_pt s, int indent)
{
    if (s == NULL) return;
    indent_to(indent);
    put_string("{funcdef statement}");
    print_position(s->stmt_def);
    assert(0);
}

static void
gen_stmt_Return(stmt_pt s, int indent)
{
    if (s == NULL) return;
    indent_to(indent);
    put_string("return");
    if (s->stmt.return_value) {
	put_char(' ');
	gen_expr(s->stmt.return_value, 0 /*false*/);
    }
    put_string(";");
    print_position(s->stmt_def);
}

/******
 * Misc
 ******/

static void
gen_stmt_Unimplemented(stmt_pt s, int indent)
{
    if (s == NULL) return;
    /* TBD: beef up with source location information */
    indent_to(indent);
    put_string("{UNIMPLEMENTED STATEMENT TYPE}");
    print_position(s->stmt_def);
    print_stmt(s, indent);
}

/*
 * The bulk of this module is static functions gen_stmt_*, one for
 * each statement kind. gen_stmt() mostly just dispatches to the
 * proper function, calling gen_stmt_func to determine the
 * proper function.
 */

typedef void (*gen_stmt_func_t) (stmt_pt s, int indent);

static gen_stmt_func_t 
gen_stmt_func(stmt_pt s)
{
    switch (s->stmt_kind) {
	case _Return:   return gen_stmt_Return;
	case _SList:    return gen_stmt_SList;
	case _Compound: return gen_stmt_Compound;
	case _Expr:     return gen_stmt_Expr;
	case _Labelled: return gen_stmt_Labelled;
	case _Case:     return gen_stmt_Case;
	case _Default:  return gen_stmt_Default;
	case _If:       return gen_stmt_If;
	case _Ifelse:   return gen_stmt_Ifelse;
	case _Switch:   return gen_stmt_Switch;
	case _While:    return gen_stmt_While;
	case _Do:       return gen_stmt_Do;
	case _For:      return gen_stmt_For;
	case _Goto:     return gen_stmt_Goto;
	case _Continue: return gen_stmt_Continue;
	case _Break:    return gen_stmt_Break;
	case _Null:     return gen_stmt_Null;
	case _FuncDef:  return gen_stmt_FuncDef;
	case _MacroBody:return gen_stmt_MacroBody;
	default:        return gen_stmt_Unimplemented;
    }
}

void
gen_stmt(stmt_pt s, int indent)
{
    /* dispatch to the right routine */
    if (s == NULL) return;
    put_comment_block(s->comment, indent);
    (*gen_stmt_func(s)) (s, indent);
}
