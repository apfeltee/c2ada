/*
 * Ridiculous heap routines
 */

#include <assert.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

#include "errors.h"
#include "allocate.h"

#undef NULL
#define NULL	0

char*
new_string(str)
	char *str;
{
	char *result;
	int len;

	assert(str != NULL);
	len = strlen(str);
	assert(len >= 0);
	result = (char*)malloc(len + 1);
	assert(result != NULL);
	strcpy(result, str);
	return result;
}

#define BUFSIZE 1024

char*
new_strf(char * str, ... )
{
    va_list   ap;
    int       n;
    char      buf[BUFSIZE];

    va_start(ap, str);
    n = vsprintf( buf, str, ap );
    va_end(ap);
    if (n>=BUFSIZE) {
	fatal(__FILE__,__LINE__,"buffer overrun in new_strf");
    }
    return new_string(buf);
}


void*
allocate(size)
	size_t size;
{
	void *ptr;

	assert(size > 0);
	ptr = malloc(size);
	assert(ptr != 0);
	memset(ptr, 0, size);
	return ptr;
}

void
deallocate(ptr)
	void *ptr;
{
	free(ptr);
}
