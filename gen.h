#ifndef _H_GEN_
#define _H_GEN_

#include "boolean.h"

extern void gen_ada_type( symbol_pt sym );
extern void gen_ada_func( symbol_pt sym, symbol_pt ante );
extern void gen_ada_var( symbol_pt sym );
extern void gen_ada_lit( symbol_pt sym );

extern void gen_any_func_ptr_type( symbol_pt sym );

extern char * int_type_builtin_name( typeinfo_pt typ );
extern char * type_nameof( typeinfo_pt, int use_parent_type, int is_param);

extern void gen_var_or_field(
    symbol_t *sym, int tabpos, int colonpos, int import,
    char * rename, int hidden);

extern void print_value( host_int_t val, int base );
extern void print_fp_value( host_float_t val );
extern void print_value(host_int_t val, int base);
extern void print_position(file_pos_t pos);

extern char * string_name(int is_wide);
extern void   gen_char_array(char* name,
			     char* val,
			     boolean is_wide_string,
			     boolean is_const);

extern int should_import(void);
extern void print_string_value( char* val,
 			        int expected_len,
			        boolean c_string );
extern void print_char_value( int val );

extern void subtype_decl(
    char *subtype_name,
    char *package_name,
    char *type_name,
    int indent,
    node_t *ident,
    file_pos_t pos);

extern void gen_local_func(symbol_pt sym, int indent);

extern void gen_subp(
    symbol_t *sym, char *rename,
    int is_spec, int is_inline, int is_interfaced);

extern void print_comment( char * );
extern void c_comment(node_t *n);
extern int is_function(symbol_t *subp);

extern struct typeinfo_t *bogus_type;

extern void gen_access_type (symbol_pt sym, boolean private_part);
extern typeinfo_t* return_type(symbol_t *subp);

#define MAX_INDENT(x) {int _i = cur_indent(); if (_i > (x)) (x) = _i;}
extern int max_const_name_indent;
extern char * char_to_string( int c, boolean c_string );
extern void interface_c(symbol_t *sym, int indent);

extern void init_predef_names(void);
extern char * c_array_index_name(void);

#endif /* _H_GEN_ */
