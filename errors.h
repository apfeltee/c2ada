#ifndef _H_ERRORS_
#define _H_ERRORS_

#include <stdarg.h>

typedef enum {
    Report_fatal,
    Report_error,
    Report_warning,
    Report_inform
} report_t;

extern void vreport(report_t severity,
		    char* filename,
		    int   linenum,
		    char *format,
		    va_list ap);

extern void fatal(char*,int,char*,...);
extern void error(char*,int,char*,...);
extern void warning(char*,int,char*,...);
extern void inform(char*,int,char*,...);

extern void assert_failed(char*,int,char*);
extern void syserr(char*,int);

#endif /* _H_ERRORS_ */

