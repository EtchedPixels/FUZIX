// header for bind.c

#ifndef _bind_h_
#define _bind_h_

#define BIND_HEADER "bind.h"

void bindfile( char *filename, char *outfile, char *dataname, char *funcname, 
	       int manifunc );

void run_bound( char *code[] );


#endif
