/* $Source: /home/CVSROOT/c2ada/aux_decls.h,v $ */
/* $Revision: 1.1.1.1 $  $Date: 1999/02/02 12:01:51 $ */

#ifndef _AUX_DECLS
#define _AUX_DECLS

#include "boolean.h"
#include "il.h"
#include "types.h"
#include "units.h"

/* Unchecked conversions */

symbol_pt unchecked_conversion_func( typeinfo_pt from_type, 
				     typeinfo_pt to_type,
				     file_pos_t  pos,
				     boolean     in_spec   );

typedef void (*gen_unchecked_conversion_func_pt) (symbol_pt sym,
						  typeinfo_pt from_type,
						  typeinfo_pt to_type);
    
void gen_unchecked_conversion_funcs( unit_n unit, 
				     gen_unchecked_conversion_func_pt f);


/* Stdarg usage */
node_pt stdarg_empty_node( file_pos_t pos);

void use_stdarg_concat( unit_n unit, typeinfo_pt type );

typedef void (*gen_stdarg_concat_func_pt) ( typeinfo_pt type );

void gen_stdarg_concat_funcs( unit_n unit, gen_stdarg_concat_func_pt f);


void use_type( unit_n unit, typeinfo_pt type );

typedef void (*gen_use_type_decl_pt) (typeinfo_pt type );
void gen_use_type_decls(unit_n unit, gen_use_type_decl_pt f);



#endif
