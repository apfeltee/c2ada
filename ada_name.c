/*
 * The routines here generate legal Ada identifiers from
 * C identifiers.  All the klugy unique name munging is
 * done here as well
 */
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>

#include "errors.h"
#include "host.h"
#include "allocate.h"
#include "ada_name.h"
#include "hash.h"
#include "files.h"
#include "il.h"
#include "configure.h"

extern char *ada_keyword(char*, int);
extern int auto_package;

#undef NULL
#define NULL		0

#define HTAB		512	/* unique name hash table size */

/*
 * Unique name hash table node type
 */
typedef struct uniq_name_t {
    char		*uname;
    int			 uord;
    hash_t		 uhash;
    struct uniq_name_t	*ulink;
} uniq_name_t;

static uniq_name_t *hash_table[HTAB];

/*
 * allocator for unique name hash table nodes
 */
static uniq_name_t*
new_uniq_name()
{
    static uniq_name_t *free = NULL;
    static int free_index;

    if (free == NULL || free_index > 63) {
	free = (uniq_name_t*) allocate(sizeof(uniq_name_t) * 64);
	free_index = 0;
    }

    return &free[free_index++];
}

boolean
is_ada_keyword(char* name)
    /*
     * Determine if argument is an Ada keyword.
     */
{
    char buf[16];
    char *p;

    assert(name != NULL);

    if (strlen(name) > 9) {
	return 0;
    }

    for (p = buf; *name; name++) {
	*p++ = lcase(*name);
    }
    *p = 0;

    return (ada_keyword(buf, (int)(p-buf))) ? 1 : 0;
}

boolean
is_reserved_id(char* name)
    /*
     * Check if this is one of the configured reserved words.
     */
{
    static char ** reserved;
    static int    n_reserved = -1;
    if (n_reserved == -1) {
	/* initialize table */
	reserved = configured_reserved_ids(&n_reserved);
    }

    if (n_reserved == 0) {
	return 0;
    } else {
	int i;
	for (i=0; i<n_reserved; i++) {
	    if (!lcasecmp(reserved[i], name)) {
		return 1;
	    }
	}
	return 0;
    }
}

void
make_ada_identifier(char * name, char * buf)
    /*
     * Generate a legal Ada identifier from the argument 'name'.
     * Specifically, ignore '$', ignore leading, trailing, or multiple
     * underscores. Since hyphen may come from include directory names
     * convert it to underscore.
     */
{
    assert(name != NULL);
    assert(buf != NULL);

    while ((*name == '_') || (*name == '$') || (*name == '.')) {
	name++;
    }

    if (*name == 0) {
	strcpy(buf, "YIKES");
    }

    while (*name) {
	switch (*name) {
	  case '$':
	    name++;
	    break;
	  case '_':
	    if (name[1] != '_' && name[1] != 0) {
		*buf++ = *name;
	    }
	    name++;
	    break;
	  case '-':
	    *buf = '_';
	    buf++;
	    name++;
	    break;
	  default:
	    *buf++ = *name++;
	    break;
	}
    }

    *buf = 0;
}

static uniq_name_t*
find_uniq(char * name, int ord)
    /*
     * Determine if an identifier has already been used
     * in the argument (ord) module.
     */
{
    uniq_name_t *un;
    hash_t hash;
    int index;

    hash = lcase_hash(name);
    index = hash & (HTAB-1);

    if (!auto_package) {
	ord = 0;
    }

    for (un = hash_table[index]; un; un = un->ulink) {
	if (un->uord == ord && un->uhash == hash && !lcasecmp(un->uname, name)) {
	    return un;
	}
    }

    return NULL;
}

char*
uniq_name( char* name, int ord )
    /*
     * Find a name that's unique within the argument unit (ord).
     * If the argument name collides, generate suffixed names
     * until a unique one is generated.
     */
{
    char buf[2048];
    uniq_name_t *un;
    int index, i;

    strcpy(buf, name);

    for (i=0; find_uniq(buf,ord) ;i++) {
	sprintf(buf, "%s_c%d", name, i);
	if (!find_uniq(buf, ord)) break;
    }

    un = new_uniq_name();
    un->uname = new_string(buf);
    un->uhash = lcase_hash(buf);
    un->uord = (auto_package) ? ord : 0;

    index = un->uhash & (HTAB-1);

    un->ulink = hash_table[index];
    hash_table[index] = un;

    return un->uname;
}


ident_case_t
id_case(id)
    char *id;
{
    return Lower;   /* mg */

#if 0
    /* code that was here equivalent to */
    if (isupper(id[0]) {
	return isupper(id[1]) ? Upper : Cap;
    } else {
	return Lower;
    }
#endif
}

static void
uppercase( char * id )
{
    char c;

    for (; *id; id++) {
	c = *id;
	if (islower(c)) *id = toupper(c);
    }
}

static void
capitalize( char * id )
{
    char *prev;
    char c;

    for (prev = id; *id; id++) {
	if (*id == '_') {
	    c = *prev;
	    if (islower(c)) *prev = toupper(c);
	    prev = id + 1;
	}
    }
}

void
id_format(id, icase)
    char *id;
    ident_case_t icase;
{
    switch (icase) {
      case Upper:
	uppercase(id);
	break;
      case Cap:
	capitalize(id);
	break;
      default:
        break;
    }
}

char*
ada_name( char * name, int ord )
    /*
     * Transform an encoded string into a valid Ada identifier
     * that is unique within unit <ord>.
     * The "encoding" is that the first character of the argument
     * string may be a code indicating a prefix to add to
     * the generated name.
     */
{
    char buf[2048];
    char *p;
    ident_case_t icase;

    /* Ignore any leading underscores. */
    for (; *name == '_'; name++);

    /* Name was all underscores?? */
    if (*name == 0) {
	name = "YIKES";
    }

    if (is_ada_keyword(name) || is_reserved_id(name)) {
	/*
	 * NB: assume that none of the reserved ID's starts
	 * with "c_", so we don't have to test again.
	 */
	icase = id_case(name);
	sprintf(buf, "c_%s", name);

    } else {

	switch (name[0]) {
	  case ENUM_PREFIX:
	    strcpy(buf, "enum_");
	    name++;
	    break;
	  case STRUCT_PREFIX:
	    strcpy(buf, "struct_");
	    name++;
	    break;
	  case UNION_PREFIX:
	    strcpy(buf, "union_");
	    name++;
	    break;
	  default:
	    buf[0] = 0;
	    break;
	}

	icase = id_case(name);

	for (p = buf; *p; p++);

	make_ada_identifier(name, p);
    }

    if (ord == -1) {
	p = new_string(buf);
    } else {
	p = uniq_name(buf, ord);
    }

    id_format(p, icase);

    return p;
}


char*
tail(char *id)
    /*
     * Return the rightmost component name of an Ada name; i.e.
     * return the largest rightmost segment of id that
     * doesn't contain a '.'
     */
{
    char * p = id + strlen(id) - 1;
    for ( ; p>=id; p--){
    if (*p=='.') return p+1;
    }
    return id;
}

