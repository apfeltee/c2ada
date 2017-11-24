/* $Source: /home/CVSROOT/c2ada/configure.h,v $ */
/* $Revision: 1.1.1.1 $  $Date: 1999/02/02 12:01:51 $ */

#include "boolean.h"

extern void configure_project( char * filename );

extern char ** configured_reserved_ids(int * count_p);

extern boolean configured;

extern boolean configured_source_flag(char*   source,
				      char*   attribute,
				      boolean default_result  );


extern boolean configured_sym_info( symbol_pt sym, typeinfo_pt type );

extern void set_output_dir( char * pathname );

extern char * configured_output_dir(void);

extern char * configured_source_partner(char * fname);


extern char * configured_macro_replacement(file_id_t file,
					   char *    macro_name,
					   char *    macro_body,
					   int       body_len,
					   int       nformals,
					   char **   formals,
					   char *    eol_comment);

