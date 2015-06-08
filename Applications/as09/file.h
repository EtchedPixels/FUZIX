/* file.h - global variables involving files for assembler */

EXTERN char *filnamptr;		/* file name pointer */
EXTERN char *truefilename;	/* in case actual source name is a tmpname */

EXTERN fd_t infil;		/* current input file (stacked, 0 = memory) */

/* Output fds */
EXTERN unsigned char outfd;	/* output fd for writer fns */
EXTERN fd_t binfil;		/* binary output file (0 = memory) */
EXTERN fd_t lstfil;		/* list output file (0 = standard) */
EXTERN fd_t objfil;		/* object output file */
EXTERN fd_t symfil;		/* symbol table output file */

/* readsrc internals */
EXTERN unsigned infil0;		/* Number of first input area */
EXTERN unsigned infiln;		/* Number of current input area */
