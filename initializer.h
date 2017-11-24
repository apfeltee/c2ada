/* $Source: /home/CVSROOT/c2ada/initializer.h,v $ */
/* $Revision: 1.1.1.1 $  $Date: 1999/02/02 12:01:51 $ */

node_pt gen_initializer( typeinfo_pt t, node_pt e);
void gen_zero(typeinfo_pt t);
void fix_sym_initializer( symbol_pt sym );
