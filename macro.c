#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <sys/types.h>
#include "c2ada.h"

/* from scan.c */
extern comment_block_pt fetch_comment_block(void);

#undef NULL
#define NULL 0

#ifndef MACRO_TABLE_SIZE /* must be a power of 2 */
    #define MACRO_TABLE_SIZE 512
#endif

#define STRMAC(a) STRMAC__NEXT(a)
#define STRMAC__NEXT(a) #a

macro_t* macro_list_head;
static macro_t* macro_list_tail;
static macro_t* hash_table[MACRO_TABLE_SIZE];

struct autodefs
{
    char* name;
    char* value;
} deftab[] = {
    {"__ANSI_CPP__", "1"},
    {"_LANGUAGE_C", "1"},
    {"LANGUAGE_C", "1"},
    {"__STDC__", "1"},

    #ifdef __IEEE_BIG_ENDIAN
        {"__IEEE_BIG_ENDIAN", "1"},
    #else
        {"__IEEE_LITTLE_ENDIAN", "1"},
    #endif

#ifdef _IBMR2
    {"_IBMR2", "1"},
#endif
#ifdef _AIX
    {"_AIX", "1"},
#endif
#ifdef _AIX32
    {"_AIX32", "1"},
#endif
#ifdef host_mips
    {"host_mips", "1"},
#endif
#ifdef mips
    {"mips", "1"},
#endif
#ifdef MIPSEB
    {"MIPSEB", "1"},
#endif
#ifdef sgi
    {"sgi", "1"},
#endif
#ifdef sun
    {"sun", "1"},
#endif
#ifdef __sun
    {"__sun", "1"},
#endif
#ifdef sparc
    {"sparc", "1"},
#endif
#ifdef __sparc
    {"__sparc", "1"},
#endif
#ifdef SVR3
    {"SVR3", "1"},
#endif
#ifndef __SVR4
    {"__SVR4", "1"},
#endif
#ifdef SYSTYPE_SYSV
    {"SYSTYPE_SYSV", "1"},
#endif
#ifdef _MIPSEB
    {"_MIPSEB", "1"},
#endif
#ifdef _POSIX_SOURCE
    {"_POSIX_SOURCE", "1"},
#endif
#ifdef _SYSTYPE_SYSV
    {"_SYSTYPE_SYSV", "1"},
#endif
#ifdef __EXTENSIONS__
    {"__EXTENSIONS__", "1"},
#endif
#ifdef __host_mips
    {"__host_mips", "1"},
#endif
#ifdef __mips
    {"__mips", "1"},
#endif
#ifdef __sgi
    {"__sgi", "1"},
#endif
#ifdef __SVR3
    {"__SVR3", "1"},
#endif
#ifdef unix
    {"unix", "1"},
#endif
#ifdef __unix
    {"__unix", "1"},
#endif
#ifdef __unix__
    {"__unix__", "1"},
#endif
#ifdef __linux
    {"__linux", "1"},
#endif
#ifdef __linux__
    {"__linux__", "1"},
#endif
#ifdef __CYGWIN__
    {"__CYGWIN__", STRMAC(__CYGWIN__)},
#endif
#ifdef __CYGWIN32__
    {"__CYGWIN32__", STRMAC(__CYGWIN32__)},
#endif
#ifdef __CYGWIN64__
    {"__CYGWIN64__", STRMAC(__CYGWIN64__)},
#endif
#ifdef _MSC_VER
    {"_MSC_VER", STRMAC(_MSC_VER)},
#endif

    #include "defaultmacros.h"

    { NULL, NULL }
};

static void macro_add_to_list(macro_t* m)
{
    if(macro_list_head)
    {
        macro_list_tail->macro_next = m;
    }
    else
    {
        macro_list_head = m;
    }
    macro_list_tail = m;
}

static void macro_add_to_table(macro_t* m)
{
    int index;

    m->macro_hash = common_hash(m->macro_name);

    index = m->macro_hash & (MACRO_TABLE_SIZE - 1);

    m->macro_hash_link = hash_table[index];
    hash_table[index] = m;
}

void macro_undef(char* name)
{
    macro_t *m, *last;
    hash_t hash;
    int index;

    assert(name != NULL);

    hash = common_hash(name);
    index = hash & (MACRO_TABLE_SIZE - 1);

    last = NULL;

    for(m = hash_table[index]; m; m = m->macro_hash_link)
    {
        if(m->macro_hash == hash && !strcmp(m->macro_name, name))
        {
            if(last == NULL)
            {
                hash_table[index] = m->macro_hash_link;
            }
            else
            {
                last->macro_hash_link = m->macro_hash_link;
            }
        }
        else
        {
            last = m;
        }
    }
}

void macro_def(
    const char* name,
    const char* body,
    int len,
    int params,
    char** formals,
    file_pos_t definition,
    char* eol_comment)
{
    macro_t* m;

    assert(name != NULL);

    m = (macro_t*)allocate(sizeof(macro_t));

    m->macro_name = name;
    m->macro_body = body;
    m->macro_body_len = len;
    m->macro_params = params;
    m->macro_definition = definition;
    m->macro_param_vec = formals;
    m->macro_func = NULL;
    m->macro_declared_in_header = current_unit_is_header;
    m->comment = fetch_comment_block();
    m->eol_comment = eol_comment;

    if(definition != 0)
    {
        macro_add_to_list(m);
    }

    macro_add_to_table(m);
}

macro_t* macro_find(char* name)
{
    macro_t* m;
    hash_t hash;
    int index;

    hash = common_hash(name);
    index = hash & (MACRO_TABLE_SIZE - 1);

    for(m = hash_table[index]; m; m = m->macro_hash_link)
    {
        if(m->macro_hash == hash && !strcmp(m->macro_name, name))
        {
            return m;
        }
    }

    return NULL;
}

void macro_init(int force)
{
    static int macro_initialized = 0;
    macro_t* m;
    int i;

    if(macro_initialized)
    {
        if(!force)
            return;
        memset(&hash_table, 0, sizeof(hash_table));
        macro_list_head = NULL;
        macro_list_tail = NULL;
    }

    macro_initialized = 1;

    m = (macro_t*)allocate(sizeof(macro_t));
    m->macro_name = "__FILE__";
    m->macro_params = BUILTIN_FILE;
    macro_add_to_table(m);

    m = (macro_t*)allocate(sizeof(macro_t));
    m->macro_name = "__LINE__";
    m->macro_params = BUILTIN_LINE;
    macro_add_to_table(m);

    for(i = 0; deftab[i].name != NULL; i++)
    {
        macro_def(deftab[i].name, deftab[i].value, strlen(deftab[i].value), -1,
                  NULL, 0, 0);
    }
}

void cpp_show_predefines()
{
    int i;

    puts("__FILE__\n__LINE__");

    for(i = 0; deftab[i].name != NULL; i++)
    {
        printf("%s\t%s\n", deftab[i].name, (deftab[i].value) ? deftab[i].value : "");
    }
}

#if 0
macro_t* nth_macro(int n)
{
	static int lastn = -10;
	static macro_t *lastm = NULL;

	int i;

	assert(n >= 0);

	if (lastn == (n - 1)) {
		i = n;
		assert(lastm != NULL);
		lastm = lastm->macro_next;
	}
	else {
		for (lastm = macro_list_head, i = 0; i < n; i++) {
			if (lastm == NULL) break;
			lastm = lastm->macro_next;
		}
	}

	if (i == n && lastm != NULL) {
		lastn = n;
		return lastm;
	}

	lastn = -10;
	lastm = NULL;

	return NULL;
}
#endif
