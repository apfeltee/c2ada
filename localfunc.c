/* $Source: /home/CVSROOT/c2ada/localfunc.c,v $ */
/* $Revision: 1.1.1.1 $  $Date: 1999/02/02 12:01:51 $ */

#include "localfunc.h"
#include "types.h"
#include "nodeop.h"
#include "stab.h"

symbol_pt
new_local_func( typeinfo_pt return_type,
	        symbol_pt   decls,
	        stmt_pt     stmts,
	        file_pos_t  pos,
	        scope_id_t  scope)
{
    set_current_scope(scope);
    {
	stmt_pt body = new_stmt_Compound( pos, decls, stmts );
	node_pt func_name = new_pos_node(pos,_Ident, "test_condition"); /*TBD*/
	node_pt func_declarator = new_pos_node(pos, _Func_Call, func_name , 0);
	symbol_pt func_spec     = function_spec(return_type,
						func_declarator,
						scope_level(scope));
	symbol_pt func_def      = new_func(func_spec, body);

	func_spec->sym_def = pos;
	func_def->sym_def  = pos;

	/* TBD: mark the function specially as local? */

	return func_def;
    }
}

stmt_pt
return_bool_stmt( boolean value, file_pos_t pos )
{
    /* TBD: synthesize booleans somehow */
    node_pt retval = new_node(_Int_Number, value);
    retval->type = type_boolean();
    /* TBD: correct pos in retval */
    return new_stmt_Return(pos, retval);
}
