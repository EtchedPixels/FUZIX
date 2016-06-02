// debug.c - RPilot's interactive debugger

#include <string.h>

#include "rpilot.h"
#include "debug.h"

void debug(void)
{
  char inbuf[MAXLINE];
  char lastcmd[MAXLINE];

  do {
    strset( inbuf, 0 );
    printf( "debug> " );
    fgets( inbuf, MAXLINE-1, stdin );
    proc( inbuf, lastcmd );
  } while( strcasecmp(inbuf, "exit") );

}

void proc( char *inbuf, char *lastcmd )
{

  char *tok;

  if( !strcmp(inbuf, "" ) ) {
    strncpy( inbuf, lastcmd, MAXLINE-1 );
  }

  tok = parse( inbuf, 1 );
  
  if( !strcasecmp(tok, "step") ) {
    //    step( rpi );
  } else if( !strcasecmp(tok, "skip") ) {
    //    skip( rpi );
  } else if( !strcasecmp(tok, "print") ) {
    //    print( rpi, inbuf );
  } else if( !strcasecmp(tok, "set") ) {
    //    set( rpi, inbuf );
  } else if( !strcasecmp(tok, "list") ) {
    //    list( rpi, inbuf );
  } else if( !strcasecmp(tok, "run") ) {
    //    run( rpi, inbuf );
  } else if( !strcasecmp(tok, "stop") ) {
    //    stop( rpi );
  } else if( !strcasecmp(tok, "help") ) {
    //    help();
  } else if( !strcasecmp(tok, "jump") ) {
    //    jump( rpi, inbuf );
  } else if( !strcasecmp(tok, "exec" ) ) {
    //    exec( rpi, inbuf );
  } else if( !strcasecmp(tok, "use") ) {
    //    use( rpi, inbuf );
  } else if( !strcasecmp(tok, "restart") ) {
    //    restart( rpi );
  }

  free( tok );

}


void dump_numvars(void) 
{
  numvar *n = (numvar *)rpi->numhead;

  while( n != NULL ) {
    printf( "\"%s\" = %d\n", n->name, n->val );
    n = (numvar *)n->next;
  }

}
