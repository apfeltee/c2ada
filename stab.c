#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>

#include "errors.h"
#include "host.h"
#include "hash.h"
#include "files.h"
#include "il.h"
#include "allocate.h"
#include "units.h"
#include "stab.h"


#undef NULL
#define NULL			0L

#undef HASH_MAX
#define HASH_MAX		512

#ifndef MAX_SCOPE_LEVEL
#define MAX_SCOPE_LEVEL		64
#endif

extern file_pos_t yypos;

static symbol_t *hash_table[HASH_MAX];

typedef struct {
	int nparams;
	symbol_t *scope_thread;
	int scope_id;
} scope_stack_info_t;

typedef struct {
	scope_kind_t	kind;
	scope_id_t     	parent;
	short int       level;
	symbol_pt      	sym;
} scope_info_t, *scope_info_pt;

static scope_stack_info_t scope_info[MAX_SCOPE_LEVEL];
static int                cur_scope_level = 0;	      /* current scope depth */

/* scope_id assigned to most recent innermost scope */
static int                scope_id = 0;

static int current_scope_id;

#define SCOPE_INFO_BLOCKSIZE  64

static scope_info_pt * scope_tab;
static int scope_tab_size = 0;


scope_id_t
new_scope_id(scope_kind_t kind)
{
	static scope_info_t * free = NULL;
	static int            free_index;

	scope_info_t *        new_scope;
	scope_id_t            result = ++scope_id;

	if (!scope_tab_size) {
		scope_tab_size = 128;
		scope_tab = allocate(sizeof(scope_info_pt)*scope_tab_size);
	} else if (result==scope_tab_size) {
		scope_tab_size = 1.5 * scope_tab_size;
		scope_tab = realloc(scope_tab,
				    sizeof(scope_info_pt)*scope_tab_size);
	}

	if (!free || free_index > SCOPE_INFO_BLOCKSIZE-1) {
		free = (scope_info_t*) allocate(sizeof(scope_info_t) *
										SCOPE_INFO_BLOCKSIZE);
		free_index = 0;
	}
	new_scope = &free[free_index++];
	new_scope->kind = kind;

	scope_tab[result] = new_scope;
	current_scope_id = result;
	return result;
}

scope_id_t
new_block_scope( scope_id_t parent )
{
    scope_id_t result = new_scope_id(Block_scope);
    set_scope_parent(result, parent);
    scope_tab[result]->level = scope_level(parent) + 1;

    return result;
}

scope_id_t
current_scope(void)
{
    return current_scope_id;
}

void
set_current_scope(scope_id_t scope)
{
    current_scope_id = scope;
}

symbol_pt
new_sym(void)
{
    static symbol_t *free = NULL;
    static int free_index;

    symbol_t *decl;

    if (free == NULL || free_index > 63) {
	free = (symbol_t*) allocate(sizeof(symbol_t) * 64);
	free_index = 0;
    }

    decl = &free[free_index++];

    decl->sym_def	      = yypos;
    decl->sym_scope	      = cur_scope_level;
    decl->sym_scope_id	      = scope_info[cur_scope_level].scope_id;
    decl->traversal_unit      = -1;
    decl->_declared_in_header = current_unit_is_header;

    return decl;
}

symbol_t*
find_sym(name)
	char *name;
{
	hash_t hash;
	int index;
	symbol_t *decl;

	assert(name != NULL);

	hash = common_hash(name);
	index = hash & (HASH_MAX-1);

	for (decl = hash_table[index]; decl; decl = decl->sym_hash_list) {

		assert(decl->sym_hash_list != decl);
		if (decl->sym_hash == hash &&
			!strcmp(name,decl->sym_ident->node.id.name)) {

			return decl;
		}
	}

	return NULL;
}

void
store_sym(symbol_pt decl)
{

	int    index;
	int    level = decl->sym_scope;

	assert(decl != NULL);
	assert(decl->sym_ident != NULL);
	assert(decl->sym_ident->node_kind == _Ident);
	assert(decl->sym_ident->node.id.name != NULL);

	if (decl->stored) return;
	decl->stored = 1;

	decl->sym_hash = common_hash(decl->sym_ident->node.id.name);
	index = decl->sym_hash & (HASH_MAX-1);

	decl->sym_hash_list = hash_table[index];
	hash_table[index] = decl;

#if 0
	/* this should only occur on func defs */
	if (level != cur_scope_level) {
	    warning(__FILE__, __LINE__, "scope level mismatch for %s",
		    decl->sym_ident->node.id.name);
	}
#endif
	decl->sym_scope_list = scope_info[level].scope_thread;
	scope_info[level].scope_thread = decl;
}

void
scope_push(scope_kind_t kind)
{
	scope_id_t scope;
	if (++cur_scope_level >= MAX_SCOPE_LEVEL) {
		fatal(__FILE__,__LINE__,"Maximum scope depth exceeded");
	}
	scope = new_scope_id(kind);
	set_scope_parent(scope, scope_info[cur_scope_level-1].scope_id);
	scope_tab[scope]->level = cur_scope_level;

	scope_info[cur_scope_level].nparams		 = 0;
	scope_info[cur_scope_level].scope_thread = NULL;
	scope_info[cur_scope_level].scope_id	 = scope_id;
}

static void
pop_scope_thread()
{
    symbol_t *thread, *sym, *last, *next;
    int index;

    for (thread = scope_info[cur_scope_level].scope_thread;
	 thread;
	 thread = next) {

	next = thread->sym_scope_list;
	thread->sym_scope_list = NULL;

	/*
	 * Do we really want to pull this symbol out
	 * of the symbol table?
	 */
	switch (thread->sym_kind) {
	case type_symbol:
	case enum_literal:
	case func_symbol:
	    continue;
        default:
             break;
	}

	last = NULL;
	index = thread->sym_hash & (HASH_MAX-1);

	for (sym = hash_table[index];
	     sym && sym != thread;
	     sym = sym->sym_hash_list) {

	    last = sym;
	}

	if (sym != NULL) {
	    if (last != NULL) {
		last->sym_hash_list = sym->sym_hash_list;
	    } else {
		hash_table[index] = sym->sym_hash_list;
	    }
	}
    }
}

void
scope_pop()
{

	pop_scope_thread();

	if (--cur_scope_level < 0) {
		fatal(__FILE__,__LINE__,"Scope stack underflow");
	}
	current_scope_id = scope_info[cur_scope_level].scope_id;
}

int
next_param()
{
	return ++scope_info[cur_scope_level].nparams;
}



scope_kind_t
scope_kind(scope_id_t scope)
{
	return scope_tab[scope]->kind;
}

scope_id_t
scope_parent(scope_id_t scope)
{
	return scope_tab[scope]->parent;
}

symbol_pt
scope_symbol( scope_id_t scope)
{
	return scope_tab[scope]->sym;
}

int
scope_level( scope_id_t scope )
{
    return scope_tab[scope]->level;
}

symbol_pt
scope_parent_func( scope_id_t scope)
{
	symbol_pt sym;
	for (; scope; scope=scope_parent(scope)) {
		if ( (sym=scope_symbol(scope)) ) return sym;
	}
	return 0;
}

void
set_scope_kind( scope_id_t scope, scope_kind_t kind)
{
	scope_tab[scope]->kind = kind;
}

void
set_scope_parent( scope_id_t scope, scope_id_t parent)
{
	scope_tab[scope]->parent = parent;
}

void
set_scope_symbol( scope_id_t scope, symbol_pt sym)
{
    assert(scope!=0);
    scope_tab[scope]->sym = sym;
}

