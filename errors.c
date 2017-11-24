#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "errors.h"

int Num_Errors;
int Num_Warnings;

static void
prefix(file, line, msg)
	char *file, *msg;
	int line;
{
	fflush(stdout);

	if (file) {
		fprintf(stderr, "%s, ", file);
	}

	if (line) {
		fprintf(stderr, "line %d, ", line);
	}

	if (msg) {
		fprintf(stderr, "%s: ", msg);
	}
}

static void
endmsg()
{
	fputc('\n', stderr);
	fflush(stderr);
}

static char * pfx_text( report_t severity )
{
    switch (severity) {
    case Report_fatal:   return "Internal error";
    case Report_error:   return "Error";
    case Report_warning: return "Warning";
    case Report_inform:  return "Info";
    default:             return "";  /* to suppress warning */
    }
}

void
vreport(report_t severity,
	char * filename,
	int linenum,
	char * format,
	va_list ap )
{
    prefix( filename, linenum, pfx_text(severity));
    vfprintf( stderr, format, ap );
    endmsg();

    switch (severity) {
    case Report_fatal:
	exit(-1); break;
    case Report_error:
	Num_Errors++;
	break;
    case Report_warning:
	Num_Warnings++;
	break;
    case Report_inform:
	break;
    }
}

void
fatal( char * f, int l, char * fmt, ... )
{
	va_list args;

	va_start(args, fmt);
	vreport(Report_fatal, f, l, fmt, args);
	va_end(args);  /* for symmetry: we should never get here */
}

void
error( char * f, int l, char * fmt, ... )
{
	va_list args;

	va_start(args, fmt);
	vreport(Report_error, f, l, fmt, args);
	va_end(args);
}

void
warning( char * f, int l, char * fmt, ... )
{
	va_list args;

	va_start(args, fmt);
	vreport(Report_warning, f, l, fmt, args);
	va_end(args);
}

void
inform(char *f, int l, char*  fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vreport(Report_inform, f, l, fmt, args);
	va_end(args);
}

#if 0
void
assert_failed(f, l, msg)
	char *f, *msg;
	int l;
{
	prefix(f,l,"Assertion Failed");
	fputs(msg, stderr);
	endmsg();
	exit(1);
}
#endif

static void
unix_error()
{
	if (errno != 0) {
        	fprintf(stderr,"%s", strerror(errno));
	}
}

void
syserr(file, line)
	char *file;
	int line;
{
	Num_Errors++;
	prefix(file,line,"Error");
	unix_error();
	endmsg();
}
