/*
  err.h - prototypes and #defines for err.c
*/

#ifndef _err_h_
#define _err_h_

#include "rpilot.h"

/* Error message number definitions -- see err() for more */
#define ERR_NONE -1         // No error
#define DUP_LABEL 0	    // If there are two labels with the same name
#define NO_FILE 1	    // If no file was given on the command line
#define ERR_FILE 2	    // If the file given can't be opened
#define UNKWN_CMD 3	    // If there is an unknown command in the source
#define NO_MEM 4	    // If we run out of memory
#define DUP_VAR 5	    // If there are two variables that share a name
#define BAD_VAR 6	    // If a non-existantant variable is used
#define EXP_MATH 7	    // When a non-math symbol where it shouldn't
#define NO_RELAT 8	    // When a relational op is missing
#define NO_COLON 9          // statement is missing a colon
#define BAD_LABEL 10        // when an unknown label is called
#define BAD_RELAT 11        // a bad relational operator is used
#define NO_EQL 12           // no equal sign in an assignment
#define NO_RPAREN 13        // a condex is missing a right parenthese
#define CONS_ASGN 14        // attempt to assign a constant a value
#define NO_CLARGS 15

// Used to determine whether the program will halt on an error 
#define FATAL 1		    // Used when the error causes a call to exit() 
#define NONFATAL 2	    // Used when we can still go on 

#define YES TRUE
#define NO FALSE

// Displays a given error message and optionally halts execution
//int err( rpinfo *rpi, int errnum );
int err( int errnum, char *msg );
char *errstr( int errnum );

#endif
