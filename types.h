#ifndef _H_TYPES_
#define _H_TYPES_

#include "boolean.h"

#define unimplemented()	not_implemented(__FILE__,__LINE__)

extern void not_implemented(char*,int);

extern void type_init(void);
extern int is_typedef(symbol_t*);
extern int is_access_to_record(typeinfo_t*);
extern int is_enum_literal(symbol_t*);
extern int is_function_pointer(typeinfo_t*);
extern int num_dimensions(typeinfo_t*);
extern int *get_dimensions(typeinfo_t*);

extern boolean equal_types( typeinfo_pt t1, typeinfo_pt t2 );
extern boolean assignment_equal_types( typeinfo_pt tleft, typeinfo_pt tright);

extern typeinfo_t *abstract_declarator_type(typeinfo_pt, node_pt);
extern typeinfo_t *new_type(typekind_t);
extern typeinfo_t *copy_type(typeinfo_t*);
extern typeinfo_t *concat_types(typeinfo_t*, typeinfo_t*);

extern typeinfo_t *typeof_void(void);
extern typeinfo_t *typeof_char(void);
extern typeinfo_t *typeof_int(void);
extern typeinfo_t *typeof_float(void);
extern typeinfo_t *typeof_double(void);

extern typeinfo_t *typeof_typemod(int);
extern typeinfo_t *typeof_typespec(typeinfo_t*);
extern typeinfo_t *typeof_specifier(symbol_t*);

extern typeinfo_t *add_pointer_type(typeinfo_t*);
extern typeinfo_t *add_function_type(typeinfo_t*);
extern typeinfo_pt add_array_type( typeinfo_pt type, node_pt size );
extern typeinfo_t *pointer_to_sym(symbol_t*);

extern typeinfo_pt typeof_char_array(void);

extern decl_class_t decl_class(typeinfo_t*);

extern symbol_t *copy_sym(symbol_t*);
extern symbol_t *concat_symbols(symbol_t*,symbol_t*);
extern symbol_t *grok_enumerator(node_t*,node_t*);
extern symbol_t *concat_ellipsis(symbol_t*);
extern symbol_t *ellipsis_sym;

extern void grok_declarations(symbol_t*);
extern void grok_func_param_decls(symbol_pt fdecl);
extern symbol_t *nested_declarations(symbol_t*);

extern void start_typedef(void);
extern void storage_class(int);
extern void type_mod(int);
extern void type_qual(int);

extern symbol_t *anonymous_enum(symbol_t*);
extern symbol_t *named_enum(node_t*,symbol_t*);
extern symbol_t *enum_reference(node_t*);
extern symbol_t *anonymous_rec(int,symbol_t*);
extern symbol_t *named_rec(int,node_t*,symbol_t*);
extern symbol_t *rec_reference(int,node_t*);
extern symbol_t *novar_declaration(typeinfo_t*);
extern symbol_t *var_declaration(typeinfo_t*, node_t*);
extern symbol_t *field_declaration(typeinfo_t*, node_t*);
extern symbol_pt function_spec( typeinfo_pt t, node_pt f, int scope_level );

extern symbol_t *noname_simple_param(typeinfo_t*);
extern symbol_t *noname_abstract_param(typeinfo_t*, node_t*);
extern symbol_t *named_abstract_param(typeinfo_t*, node_t*);

extern void typed_external_decl(symbol_t*,comment_block_pt);
extern void function_def(symbol_t*);
extern void KnR_params(symbol_t*,symbol_t*);
extern void gen_tag_types(symbol_t*, int);

extern int type_sizeof(typeinfo_t*);

extern node_t* bind_to_sym(node_t*);

/* Unlike the typeof_* functions, these always return the same node */

extern typeinfo_pt type_void(void);

extern typeinfo_pt type_signed_char(void);
extern typeinfo_pt type_unsigned_char(void);
extern typeinfo_pt type_short(void);
extern typeinfo_pt type_unsigned_short(void);
extern typeinfo_pt type_int(void) ;
extern typeinfo_pt type_unsigned(void) ;
extern typeinfo_pt type_long(void) ;
extern typeinfo_pt type_unsigned_long(void) ;

extern typeinfo_pt type_boolean(void);

extern typeinfo_pt type_float(void) ;
extern typeinfo_pt type_double(void) ;
extern typeinfo_pt type_long_double(void) ;

extern typeinfo_pt type_char(void);

extern typeinfo_pt type_string(void) ;   /* == type_charp */
extern typeinfo_pt type_charp(void);
extern typeinfo_pt type_const_charp(void);
extern typeinfo_pt type_char_array(void);


void all_types_gened(typeinfo_pt type, file_pos_t pos);

symbol_pt private_type_null( symbol_pt tsym );

#endif /* _H_TYPES_ */
