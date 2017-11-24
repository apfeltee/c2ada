#ifndef _H_ALLOCATE_
#define _H_ALLOCATE_

#ifdef NO_DEALLOC
#	undef free
#	define free(x)
#endif

extern char *new_string(char*);
extern char *new_strf(char *, ... ); /* uses printf style formatting */
extern void *allocate(size_t size);
extern void deallocate(void *ptr);

#define NEW(type) ((type*)allocate(sizeof(type)))

#endif /* _H_ALLOCATE_ */
