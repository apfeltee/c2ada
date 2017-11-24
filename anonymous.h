#ifndef _H_ANONYMOUS_
#define _H_ANONYMOUS_

extern void init_anonymous_types(void);
extern symbol_pt get_anonymous_type( typeinfo_pt );
extern int next_anonymous_ord(void);

extern typeinfo_pt find_anonymous_type( typeinfo_pt );

extern symbol_t *anonymous_function_pointer;

extern void store_anonymous_type(typeinfo_t *typ);

extern char *predef_name(char *s);
extern char *predef_name_copy(char *s);

#endif /* _H_ANONYMOUS_ */
