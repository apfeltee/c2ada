/*
 * Routines to gen makefile dependencies for C source
 */
#include <sys/types.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "lowlevel.h"
#include "errors.h"
#include "printf.h"
#include "files.h"
#include "hash.h"
#include "buffer.h"
#include "cpp.h"
#include "cpp_hide.h"
#include "allocate.h"

#undef NULL
#define NULL	0

#ifndef PATH_MAX
#define PATH_MAX		1024
#endif

#define MAX_SUPPRESS	16
#define MAX_DEPENDS		64
#define MAX_PARENTS		512
#define NOT_A_FILE		-1

extern int Num_Errors;
int translate_comments;			/* Need for resolving link symbol */


static short depend_graph[MAX_PARENTS][MAX_DEPENDS];
static short parents[MAX_PARENTS];
static int cur_parent;

static char *suppress_list[MAX_SUPPRESS];
static int suppress_index;

static void
init()
{
	int i,j;
	for (i = 0; i < MAX_PARENTS; i++)
		for (j = 0; j < MAX_DEPENDS; j++)
			depend_graph[i][j] = NOT_A_FILE;
}

static void
add_suppress(path)
	char *path;
{
	if (suppress_index >= MAX_SUPPRESS) {
		fatal(path,0,"Too many paths to suppress");
	}
	suppress_list[suppress_index++] = path;
}

static int
suppress(path)
	char *path;
{
	int i, len;
	for (i = 0; i < suppress_index; i++) {
		if (path[0] == suppress_list[i][0]) {
			len = strlen(suppress_list[i]);
			if (len <= strlen(path)) {
				if (!strncmp(path,suppress_list[i],len)) {
					return 1;
				}
			}
		}
	}
	return 0;
}

static char*
file_extension(path)
	char *path;
{
	char *last_dot = NULL;

	for (; *path; path++) {
		switch (*path) {
		  case '.':
			last_dot = path;
			break;
		  case '/':
			last_dot = NULL;
			break;
		}
	}

	return last_dot;
}

static void
strcpy_until(s1, s2, stop)
	char *s1, *s2, *stop;
{
	while (*s2 && s2 != stop) {
		*s1++ = *s2++;
	}

	*s1 = 0;
}

static void
strcpy_root(p, path)
	char *p, *path;
{
	char *last;
	for (last = p; *path; p++, path++) {
		*p = *path;
		if (*path == '/') {
			last = path + 1;
		}
	}
	*last = 0;
}

static char*
output_file(fname)
	char *fname;
{
	static char output_name[PATH_MAX + 16];
	char *ext;
	char *p1, *p2;

	ext = file_extension(fname);

	if (ext == NULL) {
		strcpy(output_name, fname);
	}
	else {
		strcpy_until(output_name, fname, ext);
	}

	strcat(output_name, ".o");
	return output_name;
}

static void
get_depends(file, f)
	int file;
	FILE *f;
{
	char *fname;
	char *lhs;
	char *depend;
	int i;

	fname = file_name_from_ord(file);
	lhs = output_file(fname);

	fprintf(f, "%s: %s\n", lhs, fname);

	for (i = 0; i < MAX_DEPENDS; i++) {
		if (depend_graph[file][i] == NOT_A_FILE) {
			return;
		}
		depend = file_name_from_ord(depend_graph[file][i]);
		if (!suppress(depend)) {
			fprintf(f, "%s: %s\n", lhs, depend);
		}
	}
}

static void
dump_depends(f)
	FILE *f;
{
	int i;
	for (i = 0; i < MAX_PARENTS; i++) {
		if (parents[i]) {
			get_depends(i,f);
		}
	}
}

static int
patch_copy(mf, tf, tfname)
	FILE *mf, *tf;
	char *tfname;
{
	char buf[2048];
	int len;

	while (fgets(buf, sizeof(buf), mf)) {
		len = strlen(buf);
		if (fwrite(buf, 1, len, tf) != len) {
			syserr(tfname, 0);
			return 1;
		}
		if (len >= 3 && strncmp(buf, "###", 3) == 0) {
			break;
		}
	}
	return 0;
}

static void
patch_mf(makefile)
	char *makefile;
{
	FILE *mf, *tf;
	int tfd;
	char tfname[16];

	if ((mf = fopen(makefile, "r")) == NULL) {
		syserr(makefile,0);
		return;
	}

	strcpy(tfname, "mf_XXXXXX");
	if ((tfd = mkstemp(tfname)) == -1) {
		fclose(mf);
		syserr(tfname,0);
		return;
	}

	tf = fdopen(tfd, "w");
	if (tf == NULL) {
		fclose(mf);
		syserr(tfname,0);
		return;
	}

	if (patch_copy(mf, tf, tfname)) {
		fclose(mf);
		fclose(tf);
		unlink(tfname);
		return;
	}

	dump_depends(tf);

	fclose(mf);
	fclose(tf);

	unlink("makefile.bak");
	if (rename(makefile, "makefile.bak") == -1) {
		error(0,0,"Can't rename %s as makefile.bak", makefile);
		return;
	}

	if (rename(tfname, makefile) == -1) {
		error(0,0,"Can't rename %s as %s", tfname, makefile);
		rename("makefile.bak", makefile);
	}
}

static void
set_parent(pos)
	file_pos_t pos;
{
	cur_parent = FILE_ORD(pos);
	parents[cur_parent] = 1;
}

static void
set_depend(pos)
	file_pos_t pos;
{
	int i;
	int file = FILE_ORD(pos);

	if (file == cur_parent) {
		return;
	}

	for (i = 0; i < MAX_DEPENDS; i++) {
		if (depend_graph[cur_parent][i] == file) {
			return;
		}
		else if (depend_graph[cur_parent][i] == NOT_A_FILE)  {
			depend_graph[cur_parent][i] = file;
			return;
		}
	}

	assert(0);
}

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
	fputs("\t\t-builtin\n", stderr);
	fputs("\t\t\tDisplay all predefined macros.\n", stderr);
	fputs("\t\t-Spath\n", stderr);
	fputs("\t\t\tSuppress dependencies for path.\n", stderr);

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

static int
scan_to_end(c)
	int c;
{
	for (;;) {
		if (c == 0 || c == '\n') break;
		c = cpp_getc();
	}
	return c;
}

int
skip_white(c)
	int c;
{
	while (is_white(c)) {
		c = cpp_getc();
	}
	return c;
}

static int
scan_directive(c)
	int c;
{
	char fname[256];
	int i;

	c = skip_white(c);
	if (! is_digit(c)) {
		return scan_to_end(c);
	}
	do {
		c = cpp_getc();
	} while (is_digit(c));
	c = skip_white(c);
	if (c != '"') {
		return scan_to_end(c);
	}
	for (i = 0; ; i++) {
		c = cpp_getc();
		if (c == '"' || c == 0 || c == '\n') break;
		fname[i] = c;
	}
	fname[i] = 0;
	set_depend(add_file(fname));
	return scan_to_end(c);
}

int
main(argc, argv)
	int argc;
	char *argv[];
{
	int i, c;
	int fstart = 0;
	int show = 0;
	char *makefile = NULL;

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
				fprintf(stderr, "%s not yet implemented\n", argv[i]);
				return 1;
			  case 'S':
				add_suppress(&argv[i][2]);
				break;
			  case 'M':
				if (argv[i][2] == 0) {
					return usage(argv[i]);
				}
				else {
					makefile = &argv[i][2];
				}
				break;
			  case 'b':
				if (strcmp(argv[i], "-builtin")) {
					return usage(argv[i]);
				}
				else {
					show = 1;
				}
				break;
			  default:
				return usage(argv[0]);
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

	init();

	for (i = fstart; i < argc; i++) {
		macro_init(1);

		if (cpp_open(argv[i]) != 0) {
			syserr(argv[i],0);
			continue;
		}

		set_parent(add_file(argv[i]));
		c = cpp_getc();
		for (;;) {
			switch (c) {
			  case 0:
				goto next_file;
			  case '\n':
				c = cpp_getc();
				break;
			  case '#':
				c = scan_directive(cpp_getc());
				break;
			  default:
				c = scan_to_end(c);
				break;
			}
		}

	  next_file:
		cpp_cleanup();
	}

	if (makefile != NULL) {
		patch_mf(makefile);
	}
	else {
		dump_depends(stdout);
	}

	return !!Num_Errors;
}
