/*
 * Main for a standard C preprocessor
 */
#include "lowlevel.h"
#include "errors.h"
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include "printf.h"
#include "files.h"
#include "hash.h"
#include "buffer.h"
#include "cpp.h"
#include "allocate.h"

#undef NULL
#define NULL	0

extern int Num_Errors;

int translate_comments;			/* Attempt to retain C comments */

static int
usage(prog)
	char *prog;
{
	fputs("Illegal invocation\n\n", stderr);
	fprintf(stderr, "Usage: %s [flags] input files\n", prog);
	fputs("\n\tflags:\n\n", stderr);
	fputs("\t\t-Dname\n", stderr);
	fputs("\t\t-Dname=value\n", stderr);
	fputs("\t\t\tDefine a macro with an optional value.  By default\n", stderr);
	fputs("\t\t\tmacros will be defined with the value 1.\n\n", stderr);
	fputs("\t\t-Uname\n", stderr);
	fputs("\t\t\tUndefine a builtin macro.\n\n", stderr);
	fputs("\t\t-Idir\n", stderr);
	fputs("\t\t\tAdd a search path for finding include files.\n\n", stderr);
	fputs("\t\t-C\n", stderr);
	fputs("\t\t\tRetain C comments in preprocessed output.\n\n", stderr);
	fputs("\t\t-builtin\n", stderr);
	fputs("\t\t\tDisplay all predefined macros.\n", stderr);

	return 1;
}

static void
do_define(name)
	char *name;
{
	char buf[128];
	char *val, *p, *s;

	if (name[0] == 0) return;

	val = strchr(name, '=');

	if (val == NULL) {
		macro_def(name, "1", 1, -1, NULL, 0);
		return;
	}

	val++;

	for (p = buf, s = name; s != val; s++, p++) {
		*p = *s;
	}

	*p = 0;
	macro_def(new_string(buf), val, strlen(val), -1, NULL, 0);
}

int
main(argc, argv)
	int argc;
	char *argv[];
{
	int i, c;
	int fstart = 0;
	int show = 0;

	macro_init(0);

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (fstart != 0) {
				fputs("Place flags before files\n", stderr);
				return usage(argv[0]);
			}

			switch (argv[i][1]) {
			  case 'D':
				do_define(&argv[i][2]);
				break;
			  case 'U':
				macro_undef(&argv[i][2]);
				break;
			  case 'I':
				cpp_search_path(&argv[i][2]);
				break;
			  case 'C':
				if (argv[i][2] == 0) {
					translate_comments = 1;
				}
				break;
			  case 'b':
				if (!strcmp(argv[i], "-builtin")) {
					show = 1;
				}
				break;
			  default:
				break;
			}
		}
		else if (fstart == 0) {
			fstart = i;
		}
	}

	if (show) {
		cpp_show_predefines();
	}

	if (fstart == 0) {
		return usage(argv[0]);
	}

	for (i = fstart; i < argc; i++) {
		if (cpp_open(argv[i]) != 0) {
			syserr(argv[i],0);
			continue;
		}

		while (c = cpp_getc()) {
			fputc(c, stdout);
		}

		cpp_cleanup();
	}

	return !!Num_Errors;
}
