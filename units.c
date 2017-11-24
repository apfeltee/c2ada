#include <assert.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>

#include "errors.h"
#include "ada_name.h"
#include "files.h"
#include "units.h"
#include "allocate.h"
#include "vendor.h"
#include "cpp.h"
#include "configure.h"

extern int auto_package;
extern int ada_version;

#undef NULL
#define NULL			0

#ifndef PATH_MAX
#define PATH_MAX		1024
#endif

#ifndef MAX
#define MAX_NEST		64
#endif

/* unit_t is not externally visible */


#define BITS_PER_INT	(sizeof(int) * BITS_PER_BYTE)
#define UNIT_SET_SIZE	((MAX_UNIQ_FNAMES + BITS_PER_INT - 1) / BITS_PER_INT)

typedef int unit_ref_t[UNIT_SET_SIZE];

typedef struct {
    unsigned	        initialized:1;

    unsigned	        unit_has_ellipsis:1;
    unsigned	        unit_has_const_string:1;
    unsigned            unit_has_unchecked_conversion:1;
    unsigned            unchecked_conversions_in_spec:1;
    unsigned            unit_with_c_pointers:1;
    unsigned	        unit_with_c_const_pointers:1;
    unsigned		unit_has_private_part:1;

    unsigned            header_comment_set:1;

    char * 		src_path;		/* C source file name */
    char *              h_src_path;
    char *              c_src_path;
    file_id_t           h_file_id;
    file_id_t           c_file_id;

    char *		unit_name;		/* Ada package name */
    char *		unit_path;		/* Output spec file name */
    char *		unit_body_path;	        /* Output body file name */

    unit_ref_t   	unit_ref;   /* implicit dependencies */
    unit_ref_t		from_body;  /* implicit references from body */
    unit_ref_t 	        direct_ref; /* explicit includes */

    comment_block_pt    header_comment;         /* comment at beginning */
    comment_block_pt    trailer_comment;        /* comment at unit end  */

} unit_t;

/* OUTPUT FILE DESCRIPTORS */
FILE * cur_unit_fd;     /* set to cur_spec_fd or cur_body_fd as appropriate */
FILE * cur_spec_fd;     /* current spec output file */
FILE * cur_body_fd;     /* current body output file */

char *predef_pkg = "C";   /* This may be overwritten by a command line switch*/

static unit_t *table[MAX_UNIQ_FNAMES];

static unit_n cur_unit;

/* <file_unit_map> keeps track of file-unit
 * (file_id_t -> unit_n) correspondences.
 * The initialization is a special signal that the whole table
 * hasn't been initialized.
 * <unit_count> is the number of active entries in <unit_map>.
 * These values are maintained in <file_unit>.
 */
static unit_n file_unit_map[MAX_UNIQ_FNAMES] = {-2};
static int    unit_count                     = 0;

static int nesting_table[MAX_NEST];
static int nest_level;

unit_n
current_unit(void)
{
    if (! auto_package) return 0;

    assert(cur_unit < unit_count);
    return cur_unit;
}

boolean
is_current_unit (unit_n unit)
{
    return (cur_unit == unit);
}

static void
set_reference(unit_ref_t ref, unit_n ord)
{
    int index;
    int bit;

    index = ord / BITS_PER_INT;
    assert(index < UNIT_SET_SIZE);

    bit = 1 << (ord % BITS_PER_INT);

    ref[index] |= bit;
}

static void
clear_reference(unit_ref_t ref, unit_n ord)
{
    int index;
    int bit;

    index = ord / BITS_PER_INT;
    assert(index < UNIT_SET_SIZE);

    bit = 1 << (ord % BITS_PER_INT);

    ref[index] &= ~bit;
}

static boolean
valid_unit( unit_n unit )
{
    return unit>=0 && unit<MAX_UNIQ_FNAMES && table[unit]!=NULL;
}

void
set_ellipsis(unit_n unit)
{
    assert(valid_unit(unit));
    table[unit]->unit_has_ellipsis = TRUE;
}

boolean
has_ellipsis(unit_n unit)
{
    assert(valid_unit(unit));
    return table[unit]->unit_has_ellipsis;
}

void
set_unit_has_private_part( unit_n unit )
{
    assert(valid_unit(unit));
    table[unit]->unit_has_private_part = TRUE;
}

boolean
unit_has_private_part( unit_n unit )
{
    assert(valid_unit(unit));
    return table[unit]->unit_has_private_part;
}


void
set_unchecked_conversion(unit_n unit, boolean in_spec)
{
    assert(valid_unit(unit));
    table[unit]->unit_has_unchecked_conversion  = TRUE;
    table[unit]->unchecked_conversions_in_spec  = in_spec;
}

boolean
has_unchecked_conversion(unit_n unit)
{
    assert(valid_unit(unit));
    return table[unit]->unit_has_unchecked_conversion;
}

boolean
unchecked_conversions_to_spec(unit_n unit)
{
    assert(valid_unit(unit));
    return table[unit]->unchecked_conversions_in_spec ||
	configured_source_flag( table[unit]->src_path,
			       "unchecked_conversions_to_spec",
			       FALSE );
}

void
with_c_pointers( unit_n unit )
{
    assert( valid_unit(unit) );
    table[unit]->unit_with_c_pointers = TRUE;
}

void
with_c_const_pointers( unit_n unit )
{
    assert( valid_unit(unit) );
    table[unit]->unit_with_c_const_pointers = TRUE;
}

boolean
has_c_pointers( unit_n unit )
{
    assert( valid_unit(unit) );
    return table[unit]->unit_with_c_pointers;
}

boolean
has_c_const_pointers( unit_n unit )
{
    assert( valid_unit(unit) );
    return table[unit]->unit_with_c_const_pointers;
}

static boolean
is_referenced(unit_ref_t ref, unit_n ord)
    /* Is ord in the set ref? */
{
    int index;
    int bit;

    index = ord / BITS_PER_INT;
    assert(index < UNIT_SET_SIZE);

    bit = 1 << (ord % BITS_PER_INT);

    return (ref[index] & bit) != 0;
}

static void
ref_merge(unit_ref_t r1, unit_ref_t r2)
    /* r1 := union(r1, r2) */
{
    int i;
    for (i = 0; i < UNIT_SET_SIZE; i++) {
	r1[i] |= r2[i];
    }
}

static void
merge_direct_refs(void)
    /*
     * Calculates the closure of unit references for all
     * units.
     * TBD: Why would be want to do this??
     */
{
    unit_n last = unit_count;
    unit_n i, j;

    if (! auto_package) return;

    for (i = 0; i < last; i++) {
	unit_t * unit_i;
	unit_t * ref_j;

	unit_i = table[i];
	assert(unit_i != NULL);
	for (j = 0; j < last; j++) {
	    if (j != i && is_referenced(unit_i->direct_ref, j)) {
		ref_j = table[j];
		assert(ref_j != NULL);
		ref_merge(unit_i->direct_ref, ref_j->direct_ref);
	    }
	}
	/* A unit shouldn't "with" itself */
	clear_reference(unit_i->direct_ref, i);
	clear_reference(unit_i->unit_ref,   i);
    }
}

void
unit_start_gen(void)
{
    cur_unit_fd = cur_spec_fd = cur_body_fd = stdout;
    merge_direct_refs();
}

static char*
gen_unit_name(path)
    char *path;
{
    char buf[PATH_MAX];
    char res[PATH_MAX];
    char *p;
    int  prefix_len;

    if ( (prefix_len = in_system_search_path(path)))  {

	p = buf;
	path += prefix_len;
    } else {
	if (ada_version < 1995){
	    char *last = path;
	    char *s;
	    for (s = path; *s; s++) {
		if (*s == '/') {
		    last = s;
		}
	    }
	    path = last;
	}
	p = buf;
    }

    for (; *path; path++) {
	switch (*path) {
	case '.':
	    if((path[1] == 'h') && (path[2] == '\0')) {
		/* convert "header.h" into "header", not "header_h" */
		goto after_loop;
	    } else if((path[1] == 'c') && (path[2] == '\0')) {
		goto after_loop;
	    } else {
		*p++ = '_';
	    }
	    break;
	case '/':
	    while (path[1] == '/')
		path++;
	    if (ada_version >= 1995)
		/* Convert directory structure into child packages */
		*p++ = '.';
	    else
		*p++ = '_';
	    break;
	default:
	    *p++ = *path;
	    break;
	}
    }
 after_loop:

    *p = 0;

    make_ada_identifier(buf, res);
    return new_string(res);
}


/* An external file may be used to map the names of units, instead
 * of using the names that c2ada would otherwise automatically generate.
 */

typedef struct _unit_map {

    char * include_name;
    char * package_name;
    char * spec_name;
    char * body_name;

    struct _unit_map *next;
} unit_map;

static char * bindings_dir(void);

static unit_map *
decode_unit_map(void)
{
    /*
     * Map file cbind.map used to map unit names
     * This file is expected to be broken into lines
     *
     *	include_file_name package_name spec_file_name body_file_name
     */

    char buf1[100], buf2[100], buf3[100], buf4[100], buf[100];
    FILE *f = fopen("cbind.map", "r");
    unit_map *first, *current;

    if (f != NULL) {
	first = NULL;
	while (!feof(f)) {
	    if (first == NULL) {
		first = current = allocate(sizeof(unit_map));
	    } else {
		current->next = allocate(sizeof(unit_map));
		current = current->next;
	    }
	    fscanf(f, "%s %s %s %s\n", buf1, buf2, buf3, buf4);
	    current->include_name = new_string(buf1);
	    current->package_name = new_string(buf2);
	    sprintf(buf, "%s/%s", bindings_dir(), buf3);
	    current->spec_name    = new_string(buf);
	    sprintf(buf, "%s/%s", bindings_dir(), buf4);
	    current->body_name    = new_string(buf);
	}
    }
    return first;
}

static void
dump_unit_map(unit_map *first)
    /* for debugging */
{
    printf("include name\tpackage name\tspec name\tbody name\n");
    printf("------------\t------------\t---------\t---------\n");
    while(first) {
	printf("%s\t%s\t%s\t%s\n", first->include_name,
	       first->package_name, first->spec_name, first->body_name);
	first = first->next;
    }
}

static unit_map *
find_unit(first, iname)
    unit_map *first;
    char *iname;
{
    while(first) {
	if(!compare_path(first->include_name, iname))
	    return first;
	first = first->next;
    }
    return NULL;
}



static void
initialize_unit(unit_n ord, file_id_t file)
{
    static boolean    first_time = TRUE;
    static unit_map * umap = NULL;

    char       buf[PATH_MAX];
    char       buf2[PATH_MAX];
    unit_t *   unit;
    char *     start_unit;

    extern int map_files;
    unit_map * this_unit;

    assert(table[ord] == NULL);

    if (first_time) {
	if (map_files) umap = decode_unit_map();
	first_time = FALSE;
    }

    unit = NEW(unit_t);

    unit->src_path = file_name_from_ord(ord);
    assert(unit->src_path != NULL);

    if ((this_unit = find_unit(umap, unit->src_path)) == NULL) {
	unit->unit_name = gen_unit_name(unit->src_path);
	assert(unit->unit_name != NULL);

	sprintf(buf, "%s/", bindings_dir());
	start_unit = &buf[strlen(buf)];
	strcat(buf, unit->unit_name);

	switch (ada_compiler) {
	case VADS:
	    strcpy(buf2, buf);
	    strcat(buf, ".a");
	    strcat(buf2, ".bdy.a");
	    break;

	case GNAT:
	    {
		char *p;
		char  c;

		for( p = start_unit; (c = *p) != '\0'; p++ ) {
		    if (c == '.') {
			if (p-start_unit >= 2) {
			    *p = '-';
			} else {
			    *p = '+';
			}
		    }
		}
		for (p = buf; (c=*p) ; p++) {
		    if  (isupper(c)) *p = tolower(c) ;
		}
		strcpy(buf2, buf);
		strcat(buf, ".ads");
		strcat(buf2, ".adb");
		break;
	    }
	    break;

	case Rational:
	    strcpy(buf2, buf);
	    strcat(buf, ".1.ada");
	    strcat(buf2, ".2.ada");
	    break;

	default:
	    strcpy(buf2, buf);
	    strcat(buf, ".ada");
	    strcat(buf2, ".body.ada");
	    break;
	}
	unit->unit_path = new_string(buf);
	unit->unit_body_path = new_string(buf2);
    } else {
	unit->unit_name      = this_unit->package_name;
	unit->unit_path      = this_unit->spec_name;
	unit->unit_body_path = this_unit->body_name;
    }

    unit->initialized = TRUE;

    table[ord] = unit;
}

static void
dump_unit(unit_n ord)
    /* print out information about a unit from the unit table */
    /* for debugging */
{
    unit_t *unit;

    unit = table[ord];
    assert(unit != NULL);

    printf("unit %d\n", ord);
    printf("\tsrc = %s\n", unit->src_path);
    printf("\tname = %s\n", unit->unit_name);
    printf("\tpath = %s\n", unit->unit_path);
    fflush(stdout);
}

void
unit_included(file_pos_t pos, int nest)
    /*
     * <pos> indicates a unit <ord> that was found in an #include
     * directive. <nest> indicates the depth of the include.
     * This routine uses the nesting table to keep track of what's
     * included in what.  An inclusion results in a "with" clause.
     */
{
    unit_t *unit;
    int ord;
    int uord;

    if (! auto_package) return;
    if (nest < 1) return;
    if (nest >= MAX_NEST) return;

    ord = pos_unit(pos);

    if (nest > nest_level) {

	uord = nesting_table[nest_level];
	unit = table[uord];
	assert(unit != NULL);
	set_reference(unit->direct_ref, ord);
/*	set_reference(unit->unit_ref, ord); */
    }

    nest_level = nest;
    nesting_table[nest] = ord;
}

void
init_unit(file_pos_t pos)
{
    unit_n    unit;
    file_id_t file;

    if (! auto_package) return;

    unit = pos_unit(pos);
    file = pos_file(pos);
    if (table[unit] == NULL) {
	initialize_unit(unit, file);
    }
    cur_unit = unit;
}

static char *
bindings_dir()
{
    static char * result;
    if (!result) {
	result = configured_output_dir();
	if (!result) result = "bindings";
    }
    return result;
}


boolean
set_unit(unit_n ord)
    /* returns TRUE only if output file can't be opened */
{
    unit_t *unit;
    static int first_time = 1;

    if (! auto_package) return 0;

    assert(ord >= 0);
    assert(ord < unit_count);

    unit = table[ord];
    assert(unit != NULL);

    if (first_time) {
	mkdir(bindings_dir(), 0777);
	first_time = 0;
    }

    cur_unit = ord;

    cur_spec_fd = fopen(unit->unit_path, "w");
    if (cur_spec_fd == NULL) {
	syserr(unit->unit_path, 0);
	return 1;
    }
    cur_body_fd = fopen(unit->unit_body_path, "w");
    if (cur_body_fd == NULL) {
	syserr(unit->unit_body_path, 0);
	return 1;
    }

    output_to_spec(); /* start out with spec in new unit */

    inform(0, 0, "Generating %s %s", unit->unit_path, unit->unit_body_path);
    return 0;
}

void
unit_completed()
{
    if (auto_package) {
	fclose(cur_spec_fd);
	fclose(cur_body_fd);
	cur_unit = MAX_UNIQ_FNAMES + 1;
    }
}

void
unit_dependency(unit_n ord, unit_n dep, boolean from_body)
    /* unit <ord> depends on unit <dep> */
{
    unit_t *ord_unit;
    unit_n  last;

    if (! auto_package) return;

    last = unit_count;

    assert(ord >= 0 && ord < last);
    assert(dep >= 0 && dep < last);

    ord_unit = table[ord];
    assert(ord_unit != NULL);

    set_reference(from_body? ord_unit->from_body : ord_unit->unit_ref,
		  dep);
}

char*
unit_name(unit_n ord)
{
    unit_t *unit;

    if (! auto_package) return "y";

    assert(ord < unit_count);

    unit = table[ord];
    assert(unit != NULL);

    assert(unit->unit_name != NULL);
    return unit->unit_name;
}

char*
cur_unit_name()
{
    return unit_name(current_unit());
}

char*
cur_unit_source()
{
    unit_t *unit;
    unit_n  ord;

    if (! auto_package) return NULL;

    ord = current_unit();

    unit = table[ord];
    assert(unit != NULL);

    assert(unit->src_path != NULL);
    return unit->src_path;
}

char*
cur_unit_path()
{
    unit_t *unit;
    int ord;

    if (! auto_package) return NULL;

    ord = current_unit();

    unit = table[ord];
    assert(unit != NULL);

    assert(unit->unit_path != NULL);
    return unit->unit_path;
}

static boolean program_has_const_string = FALSE;

void
set_cur_unit_has_const_string(void)
{
    unit_t *unit;
    unit_n  ord;

    if (auto_package) {
	ord = cur_unit;
	if (ord < unit_count) {
	    unit = table[ord];
	    assert(unit != NULL);
	    unit->unit_has_const_string = TRUE;
	}
    } else {
	program_has_const_string = TRUE;
    }
}

int
cur_unit_has_const_string(void)
{
    unit_t *unit;
    int ord;

    if (auto_package) {
	ord = current_unit();
	unit = table[ord];
	assert(unit != NULL);
	return unit->unit_has_const_string;
    } else {
	return program_has_const_string;
    }
}

static int is_child_of_predef;

void
set_cur_unit_is_child_of_predef()
{
    char *unit = cur_unit_name();
    int   i    = strlen(predef_pkg);
    /*
     * If we are not a child of the predefined package, "with" it,
     * so we get the built-in types
     */
    is_child_of_predef =
	((!strncmp(predef_pkg, unit, i)) && (unit[i] == '.'));
}

int
cur_unit_is_child_of_predef()
{
    return is_child_of_predef;
}

static unit_n
nth_element(unit_ref_t set1, unit_ref_t set2, int n)
    /* returns the <n>th element of (<set1>-<set2>) */
    /* set2 may be NULL */
{
    int    count;
    unit_n i;
    unit_n last = unit_count;

    for (count = 0, i = 0; i < last; i++) {
	if (is_referenced(set1,i) &&
	    !(set2 && is_referenced(set2,i))) {

	    if (count == n) {
		return i;
	    }
	    count++;
	}
    }
    return -1;
}

int
nth_ref_unit_ord(int n)
    /* Returns the <n>th dependency of the current unit, or -1. */
{
    unit_t *unit;

    if (!auto_package) return -1;
    unit = table[current_unit()];
    assert(unit != NULL);

    clear_reference(unit->unit_ref, current_unit());
    return nth_element(unit->unit_ref, 0, n);
}

int
nth_direct_ref_unit_ord(int n)
    /* Returns the <n>th "direct reference"  of the current unit, or -1 */
{
    unit_t *unit;

    if (!auto_package) return -1;
    unit = table[current_unit()];
    assert(unit != NULL);

    return nth_element(unit->direct_ref, 0, n);
}

int
nth_body_ref_unit_ord(int n)
{
    unit_t * unit;
    if (!auto_package) return -1;
    unit = table[current_unit()];
    assert(unit!=NULL);

    clear_reference(unit->from_body, current_unit());
    return nth_element(unit->from_body, unit->unit_ref, n);
}



/* Switch output to the current spec or body */

void
output_to_spec(void)
{
    extern void format_to_spec();

    cur_unit_fd = cur_spec_fd;
    format_to_spec();
}

void
output_to_body()
{
    extern void format_to_body();

    cur_unit_fd = cur_body_fd;
    format_to_body();
}

void
output_to(boolean to_spec)
{
    if (to_spec) {
	output_to_spec();
    } else {
	output_to_body();
    }
}

boolean
output_is_spec()
{
    return (cur_unit_fd == cur_spec_fd);
};



/* Set and get unit information fields */

comment_block_pt
cur_unit_header_comment( void )
{
    return table[cur_unit]->header_comment;
}

boolean
cur_unit_header_comment_set(void)
{
    return table[cur_unit]->header_comment_set;
}


void
set_cur_unit_header_comment( comment_block_pt comment )
{
    unit_t * cu = table[cur_unit];
    cu->header_comment = comment;
    cu->header_comment_set = TRUE;
}


comment_block_pt
cur_unit_trailer_comment( void )
{
    return table[cur_unit]->trailer_comment;
}

void
set_cur_unit_trailer_comment( comment_block_pt comment )
{
    table[cur_unit]->trailer_comment = comment;
}

file_id_t
file_partner(file_id_t file)
{
    char * file_id      = file_name_from_ord(file);
    char * partner_name = configured_source_partner(file_id);
    if (partner_name) {
	file_pos_t pos = add_file(partner_name);
	return pos_file(pos);
    } else {
	return -1;
    }
}

int
num_units()
{
    return unit_count;
}

unit_n
file_unit( file_id_t file )
{
    unit_n        result;
    file_id_t     partner;

    if (file_unit_map[0]== -2) {
	/* This is a special signal that <file_unit_map>
	 * hasn't been initialized */
	int i;
	for (i=0; i<MAX_UNIQ_FNAMES; i++) file_unit_map[i] = -1;
    }

    result = file_unit_map[file];
    if (result>=0) return result;

    result = unit_count++;
    file_unit_map[file] = result;
    partner = file_partner(file);
    if (partner >= 0) {
	file_unit_map[partner] = result;
    }
    return result;
}

unit_n
pos_unit(file_pos_t pos)
{
    unit_n result;
    if (auto_package) {
	result = file_unit( pos_file(pos) );
    } else {
	result = 0;
    }
    return result;
}

boolean
pos_in_current_unit(file_pos_t pos)
{
    return pos_unit(pos)==current_unit();
}

