#ifndef _H_MACRO_
#define _H_MACRO_

#include "boolean.h"
#include "cpp_eval.h"

/*
 * If a macro is a function, split up the right-hand-side
 * into function name, # parameters, and parameter vector
 */
typedef struct macro_function_t {
    char *	mf_fname;	/* function name */
    int 	mf_nparams;	/* number of parameters */
    char **	mf_params;	/* list of parameters */
    char *	mf_coercion;	/* type coercion, if any */
    int		mf_is_pointer;	/* is it a pointer-to-function? */
    char *	mf_rhs;		/* right-hand-side of definition */
} macro_function_t;

typedef struct macro_t {
    char *		macro_name;
    char *		macro_ada_name;
    char *		macro_body;
    int			macro_body_len;
    int			macro_params;
    char **		macro_param_vec;
    file_pos_t		macro_definition;
    struct macro_t *	macro_next;
    hash_t 		macro_hash;
    struct macro_t *	macro_hash_link;
    macro_function_t *  macro_func;
    struct comment_block * comment;   /* preceding block comment */
    char *              eol_comment;  /* comment on end-of-line  */
    cpp_eval_result_t   const_value;
    boolean	        macro_declared_in_header: 1;
    boolean	        macro_gened: 1;
    boolean	        macro_valid: 1;
    boolean	        macro_eval_tried: 1;
    boolean             macro_evald:1;
} macro_t;

typedef struct {
    macro_t *	expand_macro;
    char **	expand_actuals;
    int		expand_nactuals;
} macro_expansion_t;

extern macro_t *unit_macros[];

extern void macro_init(int);
extern void macro_undef(char*);
extern void macro_def(char*, char*, int, int,
				  char **, file_pos_t, char*);
extern macro_t *macro_find(char*);

#endif /* _H_MACRO_ */
