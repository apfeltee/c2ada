/* $Source: /home/CVSROOT/c2ada/context.h,v $ */
/* $Revision: 1.1.1.1 $  $Date: 1999/02/02 12:01:51 $ */

#ifndef H_CONTEXT
#define H_CONTEXT

#include "boolean.h"
#include "stmt.h"

typedef struct context_t ctxt_t, *ctxt_pt;

ctxt_pt new_context(scope_id_t scope);
ctxt_pt copy_context(ctxt_pt);
void free_context(ctxt_pt);

boolean changed_pre( ctxt_pt ctxt );
boolean changed_post( ctxt_pt ctxt );
boolean added_decls( ctxt_pt ctxt);
void    reset_changed( ctxt_pt ctxt );
void    clear_pre_and_post(ctxt_pt ctxt );

stmt_pt pre_stmts( ctxt_pt ctxt );
void    set_pre( ctxt_pt ctxt, stmt_pt stmts );
void    append_pre(ctxt_pt ctxt, stmt_pt stmt);

stmt_pt post_stmts( ctxt_pt ctxt );
void    set_post(ctxt_pt ctxt, stmt_pt stmts );
void    append_post(ctxt_pt ctxt, stmt_pt stmt);

symbol_pt decls(ctxt_pt ctxt, int scope_id );
void      set_decls(ctxt_pt ctxt, symbol_pt decls);  /* TBD: how linked? */
void      append_decl(ctxt_pt ctxt, symbol_pt decl);
void      append_decls(ctxt_pt ctxt, symbol_pt decls );

scope_id_t ctxt_scope(ctxt_pt ctxt);

#endif
