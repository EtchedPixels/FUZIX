/*
  err.c - Error handling code for RPilot
*/

#include "rpilot.h"
#include <string.h>

int yesno( char *msg )
{
  char buf[MAXLINE];

  printf( "%s (Y/N) ", msg );
  fgets( buf, MAXLINE-1, stdin );

  chop( buf );
  if( (!strcmp(strupr(buf), "Y")) || (!strcmp(strupr(buf), "YES")) ) {
    return YES;
  } else {
    return NO;
  }
}

int err( int errnum, char *msg )
{
  char *errbuf;
  int linenum;
  line *l;
  
  if( rpi != NULL ) {
    l = (line *)rpi->currline;
    linenum = l->linenum;
  } else {
    linenum = -1;
  }

  errbuf = (char *)xmalloc( strlen(errstr(errnum)) + strlen(msg) );
  sprintf( errbuf, errstr(errnum), msg );

  if( rpi != NULL ) {
    printf( "RPilot Error in %s, line %d\n   %s\n", 
	    rpi->filename, linenum, errbuf );
  } else {
    printf( "RPilot Error : %s\n", errbuf );
  }



  exit( errnum );
}

char *errstr( int errnum )
{

  char *errlist[] = {
    "Duplicate label `%s'",                                // DUP_LABEL
    "No file name specified",                              // NO_FILE
    "Can't open file `%s'",                                // ERR_FILE 
    "Unknown command `%s'",                                // UNKNWN_CMD
    "Out of memory!",                                      // NO_MEM
    "Duplicate variable `%s'",                             // DUP_VAR
    "Unknown variable `%s'",                               // BAD_VAR
    "Expected math symbol, not `%s'",                      // EXP_MATH
    "Missing relational operator",                         // NO_RELAT
    "Missing colon in statement",                          // NO_COLON
    "Unknown label: `%s'",                                 // BAD_LABEL
    "Unknown relational operator: `%s'",                   // BAD_RELAT
    "No equal sign in assignment",                         // NO_EQL
    "Missing right parentheses in conditional expression", // NO_RPAREN
    "Cannot assign value to constant `%s'",                // CONS_ASGN
    "Missing command line argument after `%s'"             // NO_CLARG
  };

  if( errnum >= 0 ) {
    return errlist[errnum];
  } else {
    return "";
  }
}

