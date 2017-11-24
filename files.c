#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "errors.h"
#include "hostinfo.h"
#include "files.h"


#ifdef HAS_MMAP
#include <sys/mman.h>
#else
#include <unistd.h>
#include <malloc.h>
#endif

#undef NULL
#define NULL	0

/* This module manages the correspondence between file pathnames and
 * an internal numbering of unique files, as represented by the
 * type <file_id_t>.
 * It implements the type <file_pos_t> which encodes a file number
 * and line number.
 *
 * Finally, the routines <sizeof_file>, <map_file>, and <unmap_file>
 * confusingly deal with Unix file descriptors, and don't interact
 * with the other routines in this module.
 */

/* <fnames> lists the unique pathnames in <file_id_t> order. <findex>
 * is the next available free <file_id_t>.
 */

static char *fnames[MAX_UNIQ_FNAMES];
static file_id_t findex;

#define FILE_ORD(x)		((x) >> LINE_NUMBER_BITS)

file_id_t
pos_file( file_pos_t pos )
{
    return FILE_ORD(pos);
}

line_nt
pos_line(file_pos_t pos)
{
    return line_number(pos);
}

static file_pos_t
file_line_pos( file_id_t f, int line)
{
    return (((long)f) << LINE_NUMBER_BITS) | line;
}


int
num_files()
{
    return findex;
}

char*
file_name_from_ord(file_id_t ord)
{
    return fnames[ord];
}

char *
file_name( file_pos_t pos )
{
    return fnames[FILE_ORD(pos)];
}

file_pos_t
add_file( char * path )
{
    file_pos_t i;
    file_id_t  n;

    if ((i = find_file(path)) != -1) {
	return i;
    }

    assert(findex < MAX_UNIQ_FNAMES);

    n = findex++;
    fnames[n] = path;

    return file_line_pos( n, 1 );
}

file_pos_t
set_file_pos(char * path, int line)
{
    file_pos_t pos;

    pos = add_file(path);
    return file_line_pos( pos_file(pos), line );
}

/* return 1 if strings not equal, 0 if equal */
int
compare_path(char * s1, char * s2)
{
    char c1, c2;

    /* compare paths but treat // the same as / */
    while (1) {
	c1 = *s1++;
	c2 = *s2++;
	if (c1 != c2)
	    return 1;
	if (c1 == '\0')
	    return (c2 != 0);
	if (c2 == '\0')
	    return 1;
	if (c1 == '/')
	    while (*s1 == '/')
		s1++;
	if (c2 == '/')
	    while (*s2 == '/')
		s2++;
    }
}

file_pos_t
find_file(char * path)
    /*
     * Search for <path> in the fnames list;
     * if found, return pos corresponding to file's first line;
     * otherwise return -1.
     */
{
    file_id_t i;

    for (i = 0; i < findex; i++) {
	if (!compare_path(fnames[i], path)) {
	    return file_line_pos( i, 1 );
	}
    }

    return -1;
}

size_t
sizeof_file(int fd)
{
    struct stat stat_buf;

    if (fstat(fd, &stat_buf) == -1) {
	return -1;
    }

    return stat_buf.st_size;
}

#ifdef HAS_MMAP			/* Use Unix mmap() and munmap() */
void *
map_file(int fd, size_t fsize)
{
    return mmap(0, fsize, PROT_READ, MAP_PRIVATE, fd, 0);
}

int
unmap_file(void * addr, size_t len)
{
    return munmap(addr, (int)len);
}

#else	/* This system doesn't support mmap() and munmap() */

void *
map_file(fd, fsize)
	int fd;
	size_t fsize;
{
	char *addr;
	int len;

	addr = (char*) malloc(fsize);
	assert(addr != NULL);

	len = read(fd, addr, (unsigned) fsize);
	assert(len == (int)fsize);

	return addr;
}

int
unmap_file(addr, len)
	void *addr;
	size_t len;
{
	free(addr);
}

#endif

/* convenience functions for source-specific warning messages */

void
error_at( file_pos_t pos, char * fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vreport(Report_error, file_name(pos), line_number(pos), fmt, ap);
    va_end(ap);
}

void
warning_at( file_pos_t pos, char * fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vreport(Report_warning, file_name(pos), line_number(pos), fmt, ap);
    va_end(ap);
}

void
inform_at ( file_pos_t pos, char * fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vreport(Report_inform, file_name(pos), line_number(pos), fmt, ap);
}
