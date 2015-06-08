/* scan.h - global variables for scanner for assembler */

#define EOLCHAR '\n'

EXTERN struct sym_s *gsymptr;	/* global symbol ptr */
EXTERN char lindirect;		/* left symbol for indirect addressing */
EXTERN char *lineptr;		/* current line position */
EXTERN offset_t number; 	/* constant number */
EXTERN char * rindexp;		/* error code for missing rindirect */
EXTERN char rindirect;		/* right symbol for indirect addressing */
EXTERN char sym;		/* current symbol */
EXTERN char *symname;		/* current symbol name */
