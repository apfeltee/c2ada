#ifndef _H_FORMAT_
#define _H_FORMAT_

#undef START_INDENT
#define START_INDENT	0

#ifndef COMMENT_POS
#define COMMENT_POS		64
#endif

extern int output_line(void);
extern void reset_output_line(void);

extern void reset_indent(void);
extern void new_line(void);
extern void indent_to(int);
extern void put_string(char*);
extern void put_char(int);
extern int cur_indent(void);

extern void putf(char*,...);

#endif /* _H_FORMAT_ */
