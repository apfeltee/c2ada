/*
 * Main for c2ada translator
 */

#include <stdio.h>
#include <sys/types.h>
#include <string.h>

#include "errors.h"
#include "host.h"
#include "files.h"
#include "hash.h"
#include "buffer.h"
#include "cpp.h"
#include "il.h"
#include "allocate.h"
#include "types.h"
#include "vendor.h"
#include "units.h"
#include "symset.h"
#include "configure.h"

#undef NULL
#define NULL	0

extern int Num_Errors;

int enum_reps;			/* Flag to force enum reps */
int export_from_c;		/* Flag to export funcs and vars from .c file */
boolean suppress_record_repspec = TRUE;	/* Suppress record rep clauses */
int repspec_flag;		/* Flag to force record rep clauses */
int comment_size;		/* Flag to force type size output */
boolean auto_package = TRUE;	/* Flag to gen packages automatically */
int flag_unions;
boolean import_decls = FALSE;	/* Import declarations from included headers */
int translate_comments;		/* Attempt to retain C comments */
boolean macro_warnings = TRUE;	/* Warn about untranslated macros */
int map_files = 0;		/* Map file cbind.map used to map unit names */
int output_refs = 1;		/* Output references from Ada back to C */

vendor_t ada_compiler = GNAT;
int ada_version = 1995;

extern void yylex_init(void);
extern void yyparse(void);

#define streq(s1,s2) (!strcmp(s1,s2))

static void
show_flag( char * flag, char * explanation )
{
    if (flag) {
	fprintf(stderr, "\t%s\n", flag);
    }
    if (explanation) {
	fprintf(stderr, "\t\t%s\n\n", explanation);
    }
}



static int
usage(prog)
    char *prog;
{
    fputs("Illegal invocation\n\n", stderr);
    fprintf(stderr, "Usage: %s [flags] input files\n", prog);
    fputs("  flags:\n\n", stderr);
    show_flag("-Dname[=value]",
	      "Define a macro with an optional value.  By default\n"
	      "\t\t\tmacros will be defined with the value 1.");
    show_flag("-Uname", "Undefine a builtin macro.");
    show_flag("-Idir", "Add a search path for finding include files.");
    show_flag("-S", "Add a search path for system include files");
    show_flag("-builtin", "Display all predefined macros.");
    show_flag("-fun", "Flag all union declarations.");
    show_flag("-cs", "Add sizeof and alignof comments for all decls.");
    show_flag("-erc", "Always gen enum rep clauses.");
    show_flag("-exp",
	      "Export functions, variables from .c file to Ada spec.");
    show_flag("-sih",
	      "Suppress import declarations from included headers."
	      "(default)");
    show_flag("-rrc", "Always gen record rep clauses.");
    show_flag("-src", "Suppress all record rep clauses.(default)");
    show_flag("-ap", "Automatic packaging. (default)");
    show_flag("-C","Attempt to retain C comments in the translation.");
    show_flag("-noref", "No reference comments from Ada back to C.");
    show_flag("-mwarn", "Warnings about untranslated macros.(default)");
    show_flag("-rational | -vads | -gnat | -icc",
	      "Rational | VADS | GNAT(default) | Irvine"
	      " as target compiler.");
    show_flag("-95", "Output Ada 95 (default).");
    show_flag("-pp", "Predefined package name, default is C.");
    show_flag("-mf", "Map file cbind.map used to map unit names.");
    show_flag("-Pfilename", "project configuration file name");
    show_flag("-Opathname", "output directory (default=bindings)");

    return 1;
}

static void
do_define(name)
    char *name;
{
    char *val;

    if (name[0] == 0) return;

    val = strchr(name, '=');

    if (val == NULL) {
	macro_def(name, "1", 1, -1, NULL, 0, NULL);
	return;
    }

    *val = '\0';
    val++;

    macro_def(name, val, strlen(val), -1, NULL, 0, NULL);
}

int
main(argc, argv)
    int argc;
    char *argv[];
{
    extern void gen();

    int i;
    int fstart = 0;
    int show = 0;

    macro_init(0);
    symset_init();

    for (i = 1; i < argc; i++) {
	if (argv[i][0] == '-') {
	    if (fstart != 0) {
		fputs("Place flags before files\n", stderr);
		return usage(argv[0]);
	    }

	    switch (argv[i][1]) {
	    case 'a':
		if (!streq(argv[i], "-ap")) {
		    return usage(argv[0]);
		} else {
		    auto_package = 1;
		}
		break;
	    case 'C':
		if (argv[i][2] == 0) {
		    translate_comments = 1;
		} else {
		    return usage(argv[0]);
		}
		break;
	    case 'c':
		if (!strcmp(argv[i], "-cs")) {
		    comment_size = 1;
		} else {
		    return usage(argv[0]);
		}
		break;
	    case 'e':
		if (!strcmp(argv[i], "-erc")) {
		    enum_reps = 1;
		} else if(!strcmp(argv[i], "-exp")) {
		    export_from_c = 1;
		} else {
		    return usage(argv[0]);
		}
		break;
	    case 'f':
		if (strcmp(argv[i], "-fun")) {
		    return usage(argv[0]);
		} else {
		    flag_unions = 1;
		}
		break;
	    case 'D':
		do_define(&argv[i][2]);
		break;
	    case 'U':
		macro_undef(&argv[i][2]);
		break;
	    case 'I':
		cpp_search_path(&argv[i][2]);
		break;
	    case 'O':
		set_output_dir(&argv[i][2]);
		break;
	    case 'S':
		cpp_system_search_path(&argv[i][2]);
		break;
	    case 'b':
		if (strcmp(argv[i], "-builtin")) {
		    return usage(argv[0]);
		} else {
		    show = 1;
		}
		break;
	    case 'm':
		if (!strcmp(argv[i], "-mwarn")) {
		    macro_warnings = 1;
		} else if (!strcmp(argv[i], "-mf")) {
		    map_files = 1;
		} else {
		    return usage(argv[0]);
		}
		break;
	    case 'M':
		{
		    extern boolean do_const_macros;
		    do_const_macros = FALSE;
		    break;
		}
	    case 'v':
		if (strcmp(argv[i], "-vads")) {
		    return usage(argv[0]);
		} else {
		    ada_compiler = VADS;
		}
		break;
	    case 'g':
		if (strcmp(argv[i], "-gnat")) {
		    return usage(argv[0]);
		} else {
		    ada_compiler = GNAT;
		}
		break;
	    case 'i':
		if (!strcmp(argv[i], "-icc")) {
		    ada_compiler = ICC;
		} else {
		    return usage(argv[0]);
		}
		break;
	    case 'n':
		if (!strcmp(argv[i], "-noref")) {
		    output_refs = 0;
		} else {
		    return usage(argv[0]);
		}
		break;
	    case 's':
		if (!strcmp(argv[i], "-sih")) {
		    import_decls = 0;
		} else if (!strcmp(argv[i], "-src")) {
		    suppress_record_repspec = 1;
		    repspec_flag = 0;
		} else {
		    return usage(argv[0]);
		}
		break;
	    case 'r':
		if (!strcmp(argv[i], "-rational")) {
		    ada_compiler = Rational;
		} else if (!strcmp(argv[i], "-rrc")) {
		    repspec_flag = 1;
		} else {
		    return usage(argv[0]);
		}
		break;
	    case 'p':
		if (!strcmp(argv[i], "-pp") && (i < argc)) {
		    predef_pkg = argv[++i];
		} else {
		    return usage(argv[0]);
		}
		break;
	    case 'P':
		configure_project(&argv[i][2]);
		break;
	    case '9':
		if (!strcmp(argv[i], "-95")) {
		    ada_version = 1995;
		} else {
		    return usage(argv[0]);
		}
		break;
	    default:
		return usage(argv[0]);
	    }
	} else if (fstart == 0) {
	    fstart = i;
	}
    }

    if (!configured) configure_project(0);

    if (show) {
	cpp_show_predefines();
    }

    if (fstart == 0) {
	return usage(argv[0]);
    }

    type_init();

    for (i = fstart; i < argc; i++) {
	if (cpp_open(argv[i]) != 0) {
	    syserr(argv[i],0);
	    continue;
	}

	yylex_init();
	yyparse();

	cpp_cleanup();
    }

    gen();

    return !!Num_Errors;
}
