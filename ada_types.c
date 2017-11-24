/* $Source: /home/CVSROOT/c2ada/ada_types.c,v $ */
/* $Revision: 1.1.1.1 $ */
/* $Date: 1999/02/02 12:01:51 $ */

#include "boolean.h"
#include "il.h"
#include "types.h"
#include "ada_types.h"

boolean
same_ada_type( typeinfo_pt t1, typeinfo_pt t2)
{
    if (!equal_types(t1, t2)) return FALSE;

    if (t1->type_base && t2->type_base) {
	return (t1->type_base==t2->type_base ||
		t1->type_base->sym_ada_name == t2->type_base->sym_ada_name);
    } else if (t1->type_kind==pointer_to || t1->type_kind==array_of) {
	return same_ada_type(t1->type_next, t2->type_next);
    }
    return TRUE;
}
	
