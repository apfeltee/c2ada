#ifndef _PRINT_H_
#define _PRINT_H_

#include "macro.h"

extern char * nameof_stmt_kind( stmt_kind_t );
extern char * nameof_node_kind( node_kind_t );
extern char * nameof_typekind ( typekind_t  );
extern char * nameof_sym_kind ( sym_kind_t  );

extern void print_stmt_kind(stmt_kind_t s);
extern void print_stmt(stmt_pt s, int indent);
extern void print_node(node_pt n, int indent);
extern void print_symbol(symbol_pt s, int indent);
extern void print_typekind(typekind_t t);
extern void print_node_kind(node_kind_t n);
extern void print_typeinfo(typeinfo_t *t, int indent);
extern void print_sym_kind(sym_kind_t s);
extern void print_case_alist(case_alist_pt ap, int indent);
extern void print_case_slist(case_slist_pt sp, int indent);
extern void print_case_blist(case_blist_pt bp, int indent);
extern void print_case_stmt(case_stmt_pt cp, int indent);
extern void print_macro(macro_t *m, int indent);
extern void print_macro_function(macro_function_t *f, int indent);
extern void print_file_pos(file_pos_t pos);
extern void print_comment_block
   (struct comment_block *bl, int indent);
extern void print_cpp_eval_result_t
   (cpp_eval_result_t *res, int indent);

#endif /* _PRINT_H_ */
