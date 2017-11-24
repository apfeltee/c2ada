#ifndef _H_ADA_NAME_
#define _H_ADA_NAME_

typedef enum {
	Upper,
	Lower,
	Cap
} ident_case_t;

extern int is_ada_keyword(char*);
extern void make_ada_identifier(char*, char*);
extern char *uniq_name(char*, int);
extern char *ada_name(char*, int);
extern ident_case_t id_case(char*);
extern void id_format(char*,ident_case_t);

/* return a pointer to the final component name in an Ada name */
extern char* tail(char * component);

#endif /* _H_ADA_NAME_ */
