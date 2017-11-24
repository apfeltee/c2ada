/*
 * cc like driver for the C to Ada bindings generator
 */
#include "lowlevel.h"
#include "errors.h"
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include "units.h"

#undef NULL
#define NULL			0

#undef MAXARGS
#define MAXARGS			1024

#ifndef C2ADA_TRANSLATOR
#define C2ADA_TRANSLATOR		"cbfe"
#endif

#ifndef C2ADA_PREPROCESSOR
#define C2ADA_PREPROCESSOR		"cbpp"
#endif

static char *vec[MAXARGS];
int vec_index = 1;

int output_refs = 1;            /* Output references from Ada back to C */

static int
usage(prog)
	char *prog;
{
	fputs("Illegal invocation\n\n", stderr);
	fprintf(stderr, "Usage: %s [flags] input files\n", prog);
	fputs("\n\tflags:\n\n", stderr);
	fputs("\t\t-Dname\n", stderr);
	fputs("\t\t-Dname=value\n", stderr);
	fputs("\t\t\tDefine a macro with an optional value.  By default\n",
	      stderr);
	fputs("\t\t\tmacros will be defined with the value 1.\n\n", stderr);
	fputs("\t\t-Uname\n", stderr);
	fputs("\t\t\tUndefine a builtin macro.\n\n", stderr);
	fputs("\t\t-Idir\n", stderr);
	fputs("\t\t\tAdd a search path for finding include files.\n\n", stderr);
	fputs("\t\t-builtin\n", stderr);
	fputs("\t\t\tDisplay all predefined macros.\n\n", stderr);
	fputs("\t\t-p\n", stderr);
	fputs("\t\t\tRun the preprocessor only.\n\n", stderr);
	fputs("\t\t-s\n", stderr);
	fputs("\t\t\tShow invocation only.\n\n", stderr);
	fputs("\t\t-fun\n", stderr);
	fputs("\t\t\tFlag all union declarations.\n\n", stderr);
	fputs("\t\t-cs\n", stderr);
	fputs("\t\t\tAdd sizeof and alignof comments for all decls.\n\n",
	      stderr);
	fputs("\t\t-erc\n", stderr);
	fputs("\t\t\tAlways gen enum rep clauses.\n\n", stderr);
	fputs("\t\t-exp\n", stderr);
	fputs("\t\t\tExport functions, variables from .c file to Ada spec.\n\n",
	      stderr);
	fputs("\t\t-sih\n", stderr);
	fputs("\t\t\tSuppress import declarations from included headers.\n\n",
	      stderr);
	fputs("\t\t-src\n", stderr);
	fputs("\t\t\tSuppress all record rep clauses.\n\n", stderr);
	fputs("\t\t-rrc\n", stderr);
	fputs("\t\t\tAlways gen record rep clauses.\n\n", stderr);
	fputs("\t\t-ap\n", stderr);
	fputs("\t\t\tAutomatic packaging.\n\n", stderr);
	fputs("\t\t-C\n", stderr);
	fputs("\t\t\tAttempt to retain C comments in the translation.\n\n",
	      stderr);
	fputs("\t\t-noref\n", stderr);
	fputs("\t\t\tNo reference comments from Ada back to C.\n\n", stderr);

	fputs("\t\t-mf\n", stderr);
	fputs("\t\t\tMap file cbind.map used to map unit names.\n\n", stderr);
	fputs("\t\t-mwarn\n", stderr);
	fputs("\t\t\tWarnings about untranslated macros.\n\n", stderr);
	fputs("\t\t-rational | -vads | -gnat | -icc\n", stderr);
	fputs("\t\t\tRational | VADS | GNAT | Irvine as target compiler.\n\n",
	      stderr);
	fputs("\t\t-95\n", stderr);
	fputs("\t\t\tOutput Ada 95 (default is Ada 83).\n\n", stderr);
	fputs("\t\t-pp\n", stderr);
	fputs("\t\t\tPredefined package name, default is C.\n\n", stderr);

	return 1;
}

static int
show_command()
{
	int i;

	for (i = 0; i < vec_index; i++) {
		fputc(' ', stdout);
		fputs(vec[i], stdout);
	}

	fputc('\n', stdout);

	return 0;
}

static void
addarg(arg)
	char *arg;
{
	if (vec_index >= (MAXARGS-1)) {
		fatal(0,0,"To many arguments");
	}
	vec[vec_index++] = arg;
	vec[vec_index] = NULL;
}

int
main(argc, argv)
	int argc;
	char *argv[];
{
	int i;
	int cpp_only = 0;
	int show_only = 0;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			  case 'a':
				if (strcmp(argv[i], "-ap")) {
					return usage(argv[0]);
				} else {
					addarg(argv[i]);
				}
				break;
			  case 'b':
				if (strcmp(argv[i], "-builtin")) {
					return usage(argv[0]);
				} else {
					addarg(argv[i]);
				}
				break;
			  case 'C':
				if (argv[i][2] == 0) {
					addarg(argv[i]);
				} else {
					return usage(argv[0]);
				}
				break;
			  case 'c':
				if (!strcmp(argv[i], "-cs")) {
					addarg(argv[i]);
				} else {
					return usage(argv[0]);
				}
				break;
			  case 'e':
				if (!strcmp(argv[i], "-erc") ||
				    !strcmp(argv[i], "-exp")) {
					addarg(argv[i]);
				} else {
					return usage(argv[0]);
				}
				break;
			  case 'f':
				if (strcmp(argv[i], "-fun")) {
					return usage(argv[0]);
				} else {
					addarg(argv[i]);
				}
				break;
			  case 'U':
			  case 'I':
			  case 'D':
				addarg(argv[i]);
				break;
			  case 'i':
				if (!strcmp(argv[i], "-icc")) {
					addarg(argv[i]);
				} else {
					return usage(argv[0]);
				}
				break;
			  case 'g':
				if (strcmp(argv[i], "-gnat")) {
					return usage(argv[0]);
				} else if (!cpp_only) {
					addarg(argv[i]);
				}
				break;
			  case 'm':
				if (!strcmp(argv[i], "-mwarn"))
                                {
					addarg(argv[i]);
				}
				else
                                {
                                        if (!strcmp(argv[i], "-mf"))
                                        {
					   addarg(argv[i]);
				        }
                                        else
                                        {
					   return usage(argv[0]);
					}
				}
				break;

                          case 'n':
                                if (!strcmp(argv[i], "-noref")) {
                                        output_refs = 0;
                                } else {
                                        return usage(argv[0]);
                                }
                                break;
			  case 'p':
                                if (!strcmp(argv[i], "-pp") && (i < argc)) {
                                        addarg(argv[i++]);
                                        addarg(argv[i]);
                                } else if (argv[i][2] != 0) {
					return usage(argv[0]);
				} else {
					cpp_only = 1;
				}
                                break;
			  case 'r':
				if (!strcmp(argv[i], "-rational")) {
					addarg(argv[i]);
				} else if (!strcmp(argv[i], "-rrc")) {
					addarg(argv[i]);
				} else {
					return usage(argv[0]);
				}
				break;
			  case 's':
				if (argv[i][2] == 0) {
					show_only = 1;
				} else if (!strcmp(argv[i], "-sih")) {
					addarg(argv[i]);
				} else if (!strcmp(argv[i], "-src")) {
					addarg(argv[i]);
				} else {
					return usage(argv[0]);
				}
				break;
			  case 'v':
				if (strcmp(argv[i], "-vads")) {
					return usage(argv[0]);
				} else if (!cpp_only) {
					addarg(argv[i]);
				}
				break;
			  case '9':
				if (strcmp(argv[i], "-95")) {
					return usage(argv[0]);
				} else if (!cpp_only) {
					addarg(argv[i]);
				}
				break;
			  default:
				return usage(argv[0]);
			}
		}
	}

	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-') {
			addarg(argv[i]);
		}
	}

	if (vec_index == 1) {
		return usage(argv[0]);
	}


	vec[0] = cpp_only ? C2ADA_PREPROCESSOR : C2ADA_TRANSLATOR;

	if (show_only) {
		return show_command();
	}

	(void) execvp(vec[0], vec);
	syserr(vec[0], 0);
	return 0;
}
