/* $Source: /home/CVSROOT/c2ada/order.h,v $ */
/* $Revision: 1.1.1.1 $  $Date: 1999/02/02 12:01:51 $ */

#include "symset.h"
extern boolean has_undone_requisites( symbol_pt sym );

extern void postpone_doing( symbol_pt sym );

extern void set_symbol_done( symbol_pt sym );

extern boolean sym_done(symbol_pt sym);









