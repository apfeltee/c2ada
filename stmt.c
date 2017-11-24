/* $Source: /home/CVSROOT/c2ada/stmt.c,v $ */
/* $Revision: 1.2 $  $Date: 1999/02/03 19:45:04 $ */

/*
 * This module implements the "stmt_t" type, which represents statements.
 * It contains the statement creation routines called from the grammar.
 */

#include <assert.h>

#include "errors.h"
#include "stmt.h"
#include "gen_stmt.h"
#include "fix_stmt.h"
#include "allocate.h"
#include "types.h"
#include "format.h"
#include "gen.h"
#include "stab.h"
#include "package.h"

/* from scan.c */
extern file_pos_t yypos;
extern comment_block_pt fetch_comment_block(void);

/**************** allocation of stmt structs *************************/
stmt_t*
new_stmt( stmt_kind_t stmt_kind,
	  file_pos_t pos,
	  boolean default_com,
	  comment_block_pt com )
{
	static stmt_t *free = NULL;
	static int free_index;

	stmt_t *stmt;

	if (free == NULL || free_index > 63) {
		free = (stmt_t*) allocate(sizeof(stmt_t) * 64);
		free_index = 0;
	}

	stmt = &free[free_index++];

	stmt->stmt_def  = pos;
	stmt->stmt_kind = stmt_kind;
	if (default_com) {
	    stmt->comment = fetch_comment_block();
	} else {
	    stmt->comment = com;
	}
	stmt->scope = current_scope();

	return stmt;
}

/************************ create new stmt *********************************/
/* These routines are all called from grammar.y */

stmt_pt
new_stmt_Labelled(node_pt label, comment_block_pt com,  stmt_pt s) 
{
    stmt_pt stmt = new_stmt(_Labelled, label->node_def, FALSE, com);
    stmt->stmt.label.id   = label;
    stmt->stmt.label.stmt = s;
    return stmt;
}

stmt_pt
new_stmt_Case(file_pos_t pos, comment_block_pt com, node_pt expr, stmt_pt s) 
{
    stmt_pt stmt = new_stmt(_Case, pos, FALSE, com);
    stmt->stmt.label.id   = expr;
    stmt->stmt.label.stmt = s;
    return stmt;
}


stmt_pt
new_stmt_Default(file_pos_t pos, comment_block_pt com, stmt_pt s)
{
    stmt_pt stmt = new_stmt(_Default, pos, FALSE, com);
    stmt->stmt.default_stmt = s;
    return stmt;
}


stmt_pt
new_stmt_Null(file_pos_t pos) 
{
    return new_stmt(_Null, pos, TRUE, 0);
}


stmt_pt
new_stmt_Expr(node_pt expr)
{
    stmt_pt stmt = new_stmt(_Expr, expr->node_def, TRUE, 0);
    stmt->stmt.expr = expr;
    return stmt;
}
    

stmt_pt
new_stmt_Compound(file_pos_t pos, symbol_pt decls, stmt_pt s)
{
    stmt_pt stmt = new_stmt(_Compound, pos, TRUE, 0);
    stmt->stmt.compound.decls = decls;
    stmt->stmt.compound.stmts = s;
    return stmt;
}

stmt_pt
new_stmt_If(file_pos_t pos, comment_block_pt com, node_pt cond, stmt_pt s) 
{
    stmt_pt stmt = new_stmt(_If, pos, FALSE, com);
    stmt->stmt.controlled.expr = cond;
    stmt->stmt.controlled.stmt = s;
    return stmt;
}


stmt_pt
new_stmt_Ifelse(file_pos_t       pos,
		comment_block_pt com,
		node_pt          cond,
		stmt_pt          then_stmt,
		stmt_pt          else_stmt) 
{
    stmt_pt stmt = new_stmt(_Ifelse, pos, FALSE, com);
    stmt->stmt.if_else_stmt.expr      = cond;
    stmt->stmt.if_else_stmt.then_stmt = then_stmt;
    stmt->stmt.if_else_stmt.else_stmt = else_stmt;
    return stmt;
}


stmt_pt
new_stmt_Switch(file_pos_t pos, comment_block_pt com, node_pt cond, stmt_pt s) 
{
    stmt_pt stmt = new_stmt(_Switch, pos, FALSE, com);
    stmt->stmt.controlled.expr = cond;
    stmt->stmt.controlled.stmt = s;
    return stmt;
}


stmt_pt
new_stmt_While(file_pos_t pos, comment_block_pt com, node_pt expr, stmt_pt s) 
{
    stmt_pt stmt = new_stmt(_While, pos, FALSE, com);
    stmt->stmt.controlled.expr = expr;
    stmt->stmt.controlled.stmt = s;
    return stmt;
}


stmt_pt
new_stmt_Do(file_pos_t pos, comment_block_pt com, node_pt expr, stmt_pt s) 
{
    stmt_pt stmt = new_stmt(_Do, pos, FALSE, com);
    stmt->stmt.controlled.expr = expr;
    stmt->stmt.controlled.stmt = s;
    return stmt;
}


stmt_pt
new_stmt_For(file_pos_t pos, comment_block_pt com,
	     node_pt e1, node_pt e2, node_pt e3, stmt_pt s) 
{
    stmt_pt stmt = new_stmt(_For, pos, FALSE, com);
    stmt->stmt.for_stmt.e1   = e1;
    stmt->stmt.for_stmt.e2   = e2;
    stmt->stmt.for_stmt.e3   = e3;
    stmt->stmt.for_stmt.stmt = s;
    return stmt;
}


stmt_pt
new_stmt_Goto(file_pos_t pos, node_pt label) 
{
    stmt_pt stmt = new_stmt(_Goto, pos, TRUE, 0);
    stmt->stmt.goto_label = label;
    return stmt;
}

stmt_pt
new_stmt_Continue(file_pos_t pos)
{
    return new_stmt(_Continue, pos, TRUE, 0);
}


stmt_pt
new_stmt_Break(file_pos_t pos)
{
    return new_stmt(_Break, pos, TRUE, 0);
}


stmt_pt 
new_stmt_MacroBody( file_pos_t pos, char *body )
{
    stmt_pt stmt = new_stmt(_MacroBody, pos, TRUE, 0);
    stmt->stmt.macro_body = body;
    return stmt;
}


stmt_pt
new_stmt_Return( file_pos_t pos, node_pt expr )
{
    stmt_pt s = new_stmt(_Return, pos, TRUE, 0);
    s->stmt.return_value = expr;
    return s;
}


/********************* statement lists *********************************/

stmt_pt
new_stmt_list (stmt_pt stmt)
{
    stmt_pt stmts = new_stmt(_SList, stmt->stmt_def, TRUE, 0);
    stmts->stmt.stmt_list.first = stmt;
    stmts->stmt.stmt_list.rest  = 0;
    return stmts;

}

stmt_pt
append_stmt(stmt_pt stmts, stmt_pt stmt)
{
    /* Append stmt to the end of stmts */

    stmt_pt s1, s2;

    /* find the end of the statement list */
    for (s1=stmts; (s2=s1->stmt.stmt_list.rest) ; s1=s2) {}

    s1->stmt.stmt_list.rest = new_stmt_list(stmt);
    return stmts;
}

stmt_pt
concat_stmts( stmt_pt s1, stmt_pt s2 )
{
    stmt_pt sx1;
    stmt_pt sx2 ;
    stmt_pt si1, si2;

    if (!s1) return s2;
    if (!s2) return s1;

    sx1 = (s1->stmt_kind==_List)? s1 : new_stmt_list(s1);
    sx2 = (s2->stmt_kind==_List)? s2 : new_stmt_list(s2);

    /* find end of first statment list */
    for (si1=sx1; (si2=si1->stmt.stmt_list.rest) ; si1=si2) {}

    si1->stmt.stmt_list.rest = sx2;
    return sx1;
}

void
set_stmts_scope( stmt_pt stmt, scope_id_t scope)
{
    if (stmt->stmt_kind==_List) {
	stmt->stmt.stmt_list.first->scope = scope;
	set_stmts_scope( stmt->stmt.stmt_list.rest, scope);
    }
    stmt->scope = scope;
}


/*********************  function definition *****************************/

symbol_pt
new_func (symbol_pt decl, stmt_pt body) 
{
    decl->has_initializer = 1;
    decl->sym_value.body  = body;
    grok_func_param_decls(decl);
    set_scope_symbol(body->scope, decl);
    return decl;
}


void
define_func(symbol_pt funcdef, comment_block_pt comment)
{
    assert(funcdef->sym_kind==func_symbol);
    assert(funcdef->has_initializer);
    assert(funcdef->sym_value.body->stmt_kind==_Compound);

    function_def(funcdef);
    funcdef->comment = comment;
}

/* gen_funcdef: print a function definition. */

void
gen_funcdef(symbol_pt funcdef, int indent)
{
    /* s is the compound statement which is the function def */
    stmt_pt s = funcdef->sym_value.body;
    symbol_pt sym;
    int       scope_id;

    if (s->stmt.compound.decls) {
	/* Emit relevant declarations, indented (indent+4) deep. */
	for (sym=s->stmt.compound.decls, scope_id = sym->sym_scope_id;
             sym;
             sym=sym->sym_parse_list) {

	    if (sym->sym_kind==var_symbol &&
		sym->sym_scope_id==scope_id &&
		!sym->_static) {

                gen_var_or_field(sym, indent+4, 20, -1, NULL, 0);

	    } else if (sym->sym_kind==pkg_symbol) {

		gen_pkg_def( sym, indent+4 );
	    }
	}
    }
	
    indent_to(indent);
    put_string("begin");
    new_line();

    
    if (s->stmt.compound.stmts) {
	gen_stmt(s->stmt.compound.stmts, indent+4);
    } else {
	indent_to(indent+4);
	put_string("null;");
	new_line();
    }

    indent_to(indent);
    put_string("end ");
    /* put_string(funcdef->sym_ident->node.id.name); */
    put_string(funcdef->sym_ada_name);
    put_char(';');
    new_line();
}
