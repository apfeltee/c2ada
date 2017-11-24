/* $Source: /home/CVSROOT/c2ada/stmt.h,v $ */
/* $Revision: 1.1.1.1 $  $Date: 1999/02/02 12:01:51 $  */

/*
 * This module contains types and routines for manipulating statements.
 * A statement is represented by stmt_t structure.
 */

#ifndef _H_STMT_
#define _H_STMT_

#include "il.h"

typedef struct stmt_t * stmt_pt;

typedef enum {
    _Labelled,
    _Case,
    _Default,
    _Compound,
    _SList,
    _Expr,
    _If,
    _Ifelse,
    _Switch,
    _While,
    _Do,
    _For,
    _Goto,
    _Continue,
    _Break,
    _Return,
    _Null,
    _FuncDef,
    _MacroBody
} stmt_kind_t;


/* A statement is represented by a stmt_t structure. */

typedef struct stmt_t {
    file_pos_t          stmt_def;
    stmt_kind_t		stmt_kind;
    comment_block_pt    comment;
    scope_id_t          scope;

    union /* (stmt_kind) */ {

    /* case _Labelled, _Case: */
	struct{
	    node_pt id;
	    stmt_pt stmt;
	} label;

    /* case _Default: */
	stmt_pt         default_stmt;

    /* case _SList: */
	struct {
	    stmt_pt first;   /* a single statement */
	    stmt_pt rest;    /* a statement list, or null */
	} stmt_list;

    /* case _Compound */
	struct {
	    symbol_pt decls;
	    stmt_pt stmts;
	} compound;

    /* case _FuncDef */
	struct {
	    symbol_pt decl;
	    stmt_pt   body;
	} funcdef;
	    

    /* case _Expr: */
        node_pt         expr;

    /* case _If, _Switch, _While, _Do: */
	struct{
	    node_pt expr;
	    stmt_pt stmt;
	} controlled;

    /* case _Ifelse */
	struct{
	    node_pt expr;
	    stmt_pt then_stmt, 
		    else_stmt;
	} if_else_stmt;

    /* case _For: */
	struct{
	    node_pt     e1,e2,e3;
	    stmt_pt     stmt;
        } for_stmt;

    /* case _Goto: */
	node_pt         goto_label;

    /* case _Return: */
	node_pt         return_value;

    /* case _MacroBody: */
	char *		macro_body;

    /* case _Null, _Continue, _Break, - nothing */

    } stmt;
} stmt_t;


/* Interface for generating a new statement. */

extern stmt_pt new_stmt_Labelled( node_pt label, comment_block_pt com,
				  stmt_pt stmt );
extern stmt_pt new_stmt_Case( file_pos_t pos, comment_block_pt com,
			      node_pt expr, stmt_pt stmt);
extern stmt_pt new_stmt_Default( file_pos_t pos, comment_block_pt com,
				 stmt_pt stmt );
extern stmt_pt new_stmt_Null( file_pos_t pos );
extern stmt_pt new_stmt_Expr( node_pt expr );
extern stmt_pt new_stmt_Compound( file_pos_t pos,
				  symbol_pt decls,
				  stmt_pt stmts );
extern stmt_pt new_stmt_If( file_pos_t pos, comment_block_pt com,
			    node_pt cond, stmt_pt stmt );
extern stmt_pt new_stmt_Ifelse( file_pos_t pos, comment_block_pt com,
			        node_pt cond,
 			        stmt_pt then_stmt,
			        stmt_pt else_stmt);
extern stmt_pt new_stmt_Switch( file_pos_t pos, comment_block_pt com,
			        node_pt cond,
			        stmt_pt stmt );
extern stmt_pt new_stmt_While( file_pos_t pos, comment_block_pt com,
			       node_pt expr, stmt_pt stmt );
extern stmt_pt new_stmt_Do( file_pos_t pos, comment_block_pt com,
			    node_pt expr, stmt_pt stmt );
extern stmt_pt new_stmt_For( file_pos_t pos, comment_block_pt com,
			     node_pt e1,
 			     node_pt e2,
			     node_pt e3,
			     stmt_pt stmt);
extern stmt_pt new_stmt_Goto( file_pos_t pos, node_pt label );
extern stmt_pt new_stmt_Continue( file_pos_t pos );
extern stmt_pt new_stmt_Break( file_pos_t pos );
extern stmt_pt new_stmt_Return( file_pos_t pos, node_pt expr );
extern stmt_pt new_stmt_MacroBody( file_pos_t pos, char *body );


/* A function definition comprises the header (basically a function
 * declaration) and the function body (basically a compound statement.
 * For now, a func def will be represented as a variant of a statement.
 */

extern symbol_pt new_func( symbol_pt decl, stmt_pt body );

extern void    define_func( symbol_pt funcdef, comment_block_pt comment );

extern void set_stmts_scope( stmt_pt stmts, scope_id_t scope );

/* A statement list is implemented as a variant of stmt_t */

extern stmt_pt new_stmt_list (stmt_pt stmt);
extern stmt_pt append_stmt( stmt_pt stmts, stmt_pt stmt);
extern stmt_pt concat_stmts( stmt_pt s1, stmt_pt s2 ); /* destructive */



/* 
 * Case statements transformed from the parser format
 * to one that's more useful for generating Ada
 */

typedef struct case_alist {	/* list of alternatives */
    node_pt            exp;	/* case 1:   case 2:   ... */
    struct case_alist *rest;
} case_alist, *case_alist_pt;

typedef struct case_slist {	/* list of statements after an alternative */
    stmt_pt            stm;
    struct case_slist *rest;
} case_slist, *case_slist_pt;

typedef struct case_blist {	/* list of blocks in case */
    case_alist_pt     alts;	/* a block is an alist followed by an slist */
    case_slist_pt     stms;
    struct case_blist *rest;
    int               has_default;
    stmt_pt           last_stmt;
} case_blist, *case_blist_pt;

typedef struct case_stmt {	/* C switch stmt, Ada case stmt */
    node_pt       exp;
    case_blist_pt branches;
    case_blist_pt default_branch;
} case_stmt, *case_stmt_pt;

extern void gen_funcdef(symbol_pt funcdef, int indent);

#endif
