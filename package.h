/* $Source: /home/CVSROOT/c2ada/package.h,v $ */
/* $Revision: 1.1.1.1 $  $Date: 1999/02/02 12:01:51 $ */

#include "context.h"

extern typeinfo_pt ptrs_type_for(typeinfo_pt ptr_type,
				 ctxt_pt     ctxt,
				 file_pos_t  pos);

extern void gen_pkg_def( symbol_pt sym, int indent );

extern void gen_unit_pkg_defs( int unit, int indent );

extern char * generic_ptrs_pkg_name(boolean const_ptr);
