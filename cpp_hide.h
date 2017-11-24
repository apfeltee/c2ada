#ifndef _H_CPP_HIDE_
#define _H_CPP_HIDE_

/*
 * The builtin cpp macros will be added to the hash table as normal
 * macros.  They are identified by their num_params field which will
 * have one of the following builtin values.
 */
#define BUILTIN_FILE	-2
#define BUILTIN_LINE	-3

#define BAD_INPUT		3

typedef struct {
	int skip_else;
	int cur_scope;
	int gen_scope;
	int _parsing;
	file_pos_t position;
} cpp_control_state_t;

/*
 * Character classes for the preprocessor
 */
#define PUNCT			0
#define PARAM_START		1
#define END_INPUT		2
#define ALPHA			4
#define DIGIT			8
#define XDIGIT			16
#define WHITE			32
#define END_OF_LINE		64
#define MSTART			128

enum {
	Define = 1,
	Elif,
	Else,
	Endif,
	Error,
	Ident,
	If,
	Ifdef,
	Ifndef,
	Include,
	Line,
	Pragma,
	Undef
};

#define is_eof(c)				((cpp_char_class[(int)(c)] & END_INPUT) != 0)
#define is_eol(c)				((cpp_char_class[(int)(c)] & END_OF_LINE) != 0)
#define is_hex_digit(c)			((cpp_char_class[(int)(c)] & XDIGIT) != 0)
#define is_octal_digit(c)		((c) >= '0' && (c) <= '7')
#define is_digit(c)				((cpp_char_class[(int)(c)] & DIGIT) != 0)
#define is_white(c)				((cpp_char_class[(int)(c)] & WHITE) != 0)
#define is_punct(c)				(cpp_char_class[(int)(c)] == PUNCT)
#define is_alpha(c)				((cpp_char_class[(int)(c)] & ALPHA) != 0)
#define is_alpha_numeric(c)		((cpp_char_class[(int)(c)] & (ALPHA | DIGIT)) != 0)
#define classof(c)				(cpp_char_class[(int)(c)])

#define int_modifier(c)			((c) == 'l' || (c) == 'L' || (c) == 'u' || (c) == 'U')
#define float_modifier(c)		((c) == 'F' || (c) == 'f' || (c) == 'D' || (c) == 'd')
#define is_magnitude(c)			((c) == 'E' || (c) == 'e')

extern unsigned char cpp_char_class[];
extern macro_t *macro_list_head;

extern int cpp_getc_from(buffer_t*);
extern void cpp_set_state(scan_position_t*, cpp_control_state_t*, scan_position_t**, cpp_control_state_t*);

#endif /* _H_CPP_HIDE_ */
