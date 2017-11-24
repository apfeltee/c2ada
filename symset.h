/* $Source: /home/CVSROOT/c2ada/symset.h,v $ */
/* $Revision: 1.1.1.1 $  $Date: 1999/02/02 12:01:51 $ */

#ifndef H_SYMSET
#define H_SYMSET

#include "boolean.h"

typedef struct symset  * symbols_t;

extern void symset_init(void);

extern symbols_t new_symbols_set(void);
extern void symset_add(symbols_t syms, symbol_pt sym);

void symset_filter_undone( symbols_t syms );

extern boolean symset_has(symbols_t syms, symbol_pt sym);
extern int     symset_size(symbols_t syms);

/* Symbol abstraction */
symbols_t get_undone_requisites(symbol_pt sym);
void      set_undone_requisites(symbol_pt sym, symbols_t syms);

/* symbol mapping */
typedef struct symmap * symmap_t;

symmap_t  new_symmap(char * mapname);
symbol_pt get_symmap(symmap_t map, symbol_pt key);
void      set_symmap(symmap_t map, symbol_pt key, symbol_pt value);

#endif
