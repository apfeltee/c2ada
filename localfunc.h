/* $Source: /home/CVSROOT/c2ada/localfunc.h,v $ */
/* $Revision: 1.1.1.1 $   $Date: 1999/02/02 12:01:51 $ */

#ifndef _H_LOCALFUNC_
#define _H_LOCALFUNC_

#include "boolean.h"
#include "il.h"
#include "stmt.h"

extern stmt_pt return_bool_stmt(boolean value, file_pos_t pos );

extern symbol_pt new_local_func(typeinfo_pt return_type,
				symbol_pt   decls,
				stmt_pt     stmts,
				file_pos_t  pos,
				scope_id_t  scope);


#endif
