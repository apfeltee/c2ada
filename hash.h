#ifndef _H_HASH_
#define _H_HASH_

#ifndef HASH_PRIME
#define HASH_PRIME		31741
#endif

typedef unsigned long hash_t;

extern hash_t common_hash(char*);
extern hash_t lcase_hash(char*);
extern int lcase(int);
extern int lcasecmp(char*,char*);

#endif /* _H_HASH_ */
