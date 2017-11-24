#ifndef _H_CPP_
#define _H_CPP_

#include "il.h"
#include "macro.h"

typedef enum {
    scan_file,
    scan_macro_expansion,
    scan_text
} scan_kind_t;

struct cpp_file_t;
typedef struct cpp_file_t cpp_file_t, *cpp_file_pt;

typedef struct scan_position_t {
    scan_kind_t			scan_kind;
    union {
	macro_expansion_t	expansion;
	cpp_file_pt		file;
	char *                  text;
    }				scan;
    file_pos_t			scan_pos;
    int				scan_index;
    struct scan_position_t *    scan_next;
} scan_position_t;

extern int  cpp_open(char *path);
extern void cpp_cleanup(void);
extern int  cpp_getc(void);
extern void cpp_search_path(char*);
extern void cpp_system_search_path(char*);
extern int  in_system_search_path(char*);
extern void cpp_show_predefines(void);

extern boolean at_file_start;

#endif /* _H_CPP_ */
