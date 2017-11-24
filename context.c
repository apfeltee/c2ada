/* $Source: /home/CVSROOT/c2ada/context.c,v $ */
/* $Revision: 1.2 $  $Date: 1999/02/03 19:45:03 $ */

/*
 * The context module defines an abstract data type, ctxt_t, which
 * holds information about the context of a statement or expression
 * being transformed into an Ada-compatible form.  It holds whatever
 * additional declarations, preceding statements, or following (post-) 
 * statements are required in the transformation.
 */

#include "boolean.h"
#include "context.h"
#include "allocate.h"
#include "types.h"
#include "stmt.h"
#include "stab.h"

struct context_t {
    boolean	changed_pre:1;
    boolean	changed_post:1;
    boolean	added_decls:1;

    symbol_pt	decls;
    stmt_pt	pre_stmts;
    stmt_pt	post_stmts;

    scope_id_t  scope;
};

ctxt_pt
new_context(scope_id_t scope)
{
    ctxt_pt ctxt = (ctxt_pt) allocate( sizeof(ctxt_t) );
    ctxt->scope = scope;
    return ctxt;
}

ctxt_pt
copy_context( ctxt_pt ctxt )
{
    ctxt_pt result = new_context(ctxt->scope);
    *result = *ctxt;
    return result;
}

void
free_context(ctxt_pt ctxt)
{
    deallocate(ctxt);
}


boolean
changed_pre( ctxt_pt ctxt )
{
    return ctxt->changed_pre;
}

boolean
changed_post( ctxt_pt ctxt )
{
    return ctxt->changed_post;
}

boolean
added_decls( ctxt_pt ctxt )
{
    return ctxt->added_decls;
}

void
reset_changed( ctxt_pt ctxt )
{
    ctxt->changed_pre  = FALSE;
    ctxt->changed_post = FALSE;
    ctxt->added_decls  = FALSE;
}

void
clear_pre_and_post( ctxt_pt ctxt )
{
    ctxt->pre_stmts = 0;
    ctxt->post_stmts = 0;
    ctxt->changed_pre = FALSE;
    ctxt->changed_post = FALSE;
}


stmt_pt
pre_stmts( ctxt_pt ctxt )
{
    return ctxt->pre_stmts;
}

stmt_pt
post_stmts( ctxt_pt ctxt )
{
    return ctxt->post_stmts;
}

void
set_pre (ctxt_pt ctxt, stmt_pt stmts )
{
    set_stmts_scope( stmts, ctxt->scope );
    ctxt->pre_stmts = stmts;
}
    
void
set_post( ctxt_pt ctxt, stmt_pt stmts )
{
    set_stmts_scope(stmts, ctxt->scope);
    ctxt->post_stmts = stmts;
}

void
append_pre( ctxt_pt ctxt, stmt_pt stmt )
{
    if (!stmt) return;
    set_stmts_scope( stmt, ctxt->scope );
    if (ctxt->pre_stmts) {
	append_stmt( ctxt->pre_stmts, stmt );
    } else {
	ctxt->pre_stmts = new_stmt_list( stmt );
    }
    ctxt->changed_pre = TRUE;
}

void
append_post( ctxt_pt ctxt, stmt_pt stmt )
{
    if (!stmt) return;
    set_stmts_scope( stmt, ctxt->scope );
    if (ctxt->post_stmts) {
	append_stmt( ctxt->post_stmts, stmt );
    } else {
	ctxt->post_stmts = new_stmt_list( stmt );
    }
    ctxt->changed_post = TRUE;
}

static void
set_decls_scope( symbol_pt decls, scope_id_t scope)
{
    symbol_pt sym;
    int level = scope_level(scope);
    for (sym=decls; sym; sym=sym->sym_parse_list) {
	sym->sym_scope_id = scope;
	sym->sym_scope    = level;
    }
}



symbol_pt
decls(ctxt_pt ctxt, int scope_id)
{

    if (scope_id) {
	/* nonzero scope_id is a signal to change all the scope_ids in the
	 * list
	 */
	set_decls_scope(ctxt->decls, scope_id);

    }
    return ctxt->decls;
}

void
set_decls(ctxt_pt ctxt, symbol_pt decls)
{
    if (!decls) return;
    set_decls_scope( decls, ctxt->scope);
    ctxt->decls = decls;
}

void
append_decls( ctxt_pt ctxt, symbol_pt decls )
{
    if (!decls) return;
    set_decls_scope(decls, ctxt->scope);
    ctxt->decls = concat_symbols( ctxt->decls, decls );
    ctxt->added_decls = TRUE;
}

void
append_decl( ctxt_pt ctxt, symbol_pt decl)
{
    append_decls(ctxt, decl);
}

scope_id_t
ctxt_scope(ctxt_pt ctxt)
{
    return ctxt->scope;
}
