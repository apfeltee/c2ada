#ifndef _H_CPP_EVAL_
#define _H_CPP_EVAL_

typedef long cpp_int_t;

typedef enum {
	eval_failed,
	eval_int,
	eval_float,
	eval_string,
	eval_type
} cpp_eval_result_kind_t;

typedef struct {
	cpp_eval_result_kind_t	eval_result_kind;
	union {
		cpp_int_t			ival;
		double				fval;
		char				*sval;
		typeinfo_pt                     tval;
	}            eval_result;
	int          base;				/* 10, 8, or 16 */
	typeinfo_pt  explicit_type;
} cpp_eval_result_t;

extern cpp_eval_result_t cpp_eval(char*);

#define EVAL_FAILED(x)			((x).eval_result_kind == eval_failed)
#define IS_EVAL_INT(x)			((x).eval_result_kind == eval_int)
#define IS_EVAL_FLOAT(x)		((x).eval_result_kind == eval_float)
#define IS_EVAL_STRING(x)		((x).eval_result_kind == eval_string)

#define EVAL_INT(x)				((x).eval_result.ival)
#define EVAL_FLOAT(x)			((x).eval_result.fval)
#define EVAL_STRING(x)			((x).eval_result.sval)

#endif /* _H_CPP_EVAL_ */
