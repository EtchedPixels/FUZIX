/*
  cmds.c - RPilot commands functions
*/


#include "rpilot.h"
#include "math.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef isblank
#define isblank(ch) (ch==' ' || ch == '\t')
#endif

/* FIXME: move this into the C headers where it belongs */
#ifndef RAND_MAX
#define RAND_MAX 32767
#endif

/*
 * internal_type takes a string and formats it like it would be for a T:
 * statement, but returning a string rather than printing it to the screen 
 */
char *internal_type( char *str )
{
  int i;
  int nextsp;
  char *varname;
  char output[500], *ret;


 if( str == NULL ) {
   return new_string("");
 }


 // output = (char *)malloc(5000);
 strcpy( output, "" );
 
  for( i=0; i<strlen(str); i++ ) {
    if( (str[i] == '$') && (!isblank(str[i+1])) ) {
      if( str[i+1] == '$' ) {
	i+=2;
      } else {
	nextsp = find( str, " \t", i );  // find next space
	if( nextsp == -1 ) {  // no more spaces, use the rest of the line
	  varname = new_string_from( str, i, strlen(str)-i );
	} else {
	  varname = new_string_from( str, i, nextsp-i );
	}
	sprintf( output, "%s%s ", output, get_strvar( varname) );
	i+=strlen( varname );
	free( varname );
      }
    } else if( (str[i] == '#') && (!isblank(str[i+1])) ) {
      if( str[i+1] == '#' ) {
	i+=2;
      } else {
	nextsp = find( str, " \t", i );  // find next space;
	if( nextsp == -1 ) {  // no more spaces, use the rest of the line
	  varname = new_string_from( str, i, strlen(str)-i );
	} else {
	  varname = new_string_from( str, i, nextsp-i );
	}
	sprintf( output, "%s%d ", output, get_numvar( varname) );
	i+=strlen( varname );
	free( varname );
      }
    } else {
      sprintf( output, "%s%c", output, str[i] );
    }
    //    printf( "output=\"%s\"\n", output );

  }

  ret = new_string( output );
  //  free( output );
  //  printf("!!output=\"%s\", len=%d\n", output, strlen(output) );
  //  printf("!!ret=\"%s\", len=%d\n", ret, strlen(ret) );
  
  return ret;
}


/*
 * the cmd_ functions implement the PILOT statements that they're named after
 */

void cmd_use( char *str )
{
  use( str );  
}

void cmd_compute( char *str )
{
  char *lside, *rside;
  int eqlpos;
  int i;

  if( str == NULL ) {
    return;
  }

  trim( str );

  for( eqlpos=0; eqlpos<strlen(str); eqlpos++ ) {
    if( str[eqlpos] == '=' ) {
      break;
    }
  }

  if( eqlpos == strlen(str) ) {
    err( NO_EQL, str );
  }

  lside = new_string_from( str, 0, eqlpos );
  rside = new_string_from( str, eqlpos+1, strlen(str)-eqlpos-1 );
  trim( lside );
  trim( rside );
  
  if( lside[0] == '#' ) { // numeric variable, so a simple assignment
    set_numvar( lside, express(rside) );
  } else if( lside[0] == '$' ) { // string var, copy all strings on the right
    set_strvar( lside, internal_type(rside) );
  } else {
    err( CONS_ASGN, lside );
  }
      

  free( lside );
  free( rside );
}



void cmd_accept( char *str )
{
  char inbuf[MAXLINE];
  int i;

  fflush( stdin );
  printf( "%s ", get_strvar("$PROMPT") );  // print the prompt

  trim( str );

  if( !strcmp(str, "") ) { // blank args, so we put it in $accept
    fgets( inbuf, MAXLINE-1, stdin );
    chop( inbuf );  // remove trailing LF
    set_strvar( "$ACCEPT", inbuf );
    free( rpi->lastacc );
    rpi->lastacc = new_string( "$ACCEPT" );
  } else if( str[0] == '$' ) { // we need to read a string variable
    fgets( inbuf, MAXLINE-1, stdin );
    chop( inbuf );
    if( !strcmp(inbuf, "") ) {  // did the user enter a blank line?
      strcpy( inbuf, "[BLANK]" ); // FIXME ....maybe
    }
    set_strvar( str, inbuf );
    free( rpi->lastacc );
    rpi->lastacc = new_string( str );
  } else if( str[0] == '#' ) { // read a numeric var
    fgets( inbuf, MAXLINE-1, stdin );
    chop( inbuf );
    i = atoi( inbuf );
    //    scanf( "%d", &i );
    set_numvar( str, i );
    free( rpi->lastacc );
    rpi->lastacc = new_string( str );
  }

  fflush(stdin);
}


void cmd_type( char *str )
{
/*   int i; */
/*   int nextsp; */
/*   char *varname; */

/*  if( str == NULL ) { */
/* //puts( "T: empty arguments (str == NULL)" ); */
/*    return; */
/*  } */

/*   for( i=0; i<strlen(str); i++ ) { */
/*     if( (str[i] == '$') && (!isblank(str[i+1])) ) { */
/*       if( str[i+1] == '$' ) { */
/* 	i+=2; */
/*       } else { */
/* 	nextsp = find( str, " \t", i );  // find next space */
/* 	if( nextsp == -1 ) {  // no more spaces, use the rest of the line */
/* 	  varname = new_string_from( str, i, strlen(str)-i ); */
/* 	} else { */
/* 	  varname = new_string_from( str, i, nextsp-i ); */
/* 	} */
/* 	printf( "%s ", get_strvar( varname) ); */
/* 	i+=strlen( varname ); */
/* 	free( varname ); */
/*       } */
/*     } else if( (str[i] == '#') && (!isblank(str[i+1])) ) { */
/*       if( str[i+1] == '#' ) { */
/* 	i+=2; */
/*       } else { */
/* 	nextsp = find( str, " \t", i );  // find next space; */
/* 	if( nextsp == -1 ) {  // no more spaces, use the rest of the line */
/* 	  varname = new_string_from( str, i, strlen(str)-i ); */
/* 	} else { */
/* 	  varname = new_string_from( str, i, nextsp-i ); */
/* 	} */
/* 	printf( "%d ", get_numvar( varname) ); */
/* 	i+=strlen( varname ); */
/* 	free( varname ); */
/*       } */
/*     } else { */
/*       putchar( str[i] ); */
/*     } */
/*   } */
  
  char *output;

  output = internal_type( str );
  printf( "%s\n", output );
  free( output );
  
}


void cmd_end( char *str )
{
  
  rpi->currline = (struct line *)stk_pop( (stack *)rpi->stk );
  
}


void cmd_match( char *str )
{
  int count = numstr( str );
  int i;
  //  char *temp = (char *)malloc( strlen(str)+1 );
  char *temp = NULL;

  strupr( str );
  for( i=1; i<count+1; i++ ) { // count;
    //    strset( temp, 0 );
    temp = parse( str, i );
    if( !strcmp(strupr(get_strvar(rpi->lastacc)), temp) ) {
      set_numvar( "#MATCHED", 1 );
      set_numvar( "#WHICH", i );
      return;
    }
  }
  
  free( temp );

  set_numvar( "#MATCHED", 0 );
  set_numvar( "#WHICH", 0 );  
}



void cmd_jump( char *str  )
{
  jump( str );
}


void cmd_execute( char *str )
{
  execute( str );
}

// Types a line if #matched == TRUE
void cmd_yes( char *str )
{

  if( get_numvar( "#MATCHED") == TRUE ) {
    cmd_type( str );
  }
}


// Types a line if #matched == FALSE
void cmd_no( char *str )
{

  if( get_numvar("#MATCHED") == FALSE ) {
    cmd_type( str );
  }
}



// call a shell
void cmd_shell( char *str )
{
  int retcode;

  retcode = system( get_strval(str) );
  set_numvar( "#RETCODE", retcode );
}



void cmd_debug( char *str )
{
  debug();
}


void cmd_generate( char *str )
{
  int upper, lower, rnd;
  char *exp, *var;

  if( !strcmp(str, "") ) {
    // do what?
  } else {
    trim( str );
  }

  var = parse( str, 1 );
  exp = parse( str, 2 );
  lower = get_numval( exp );
  free( exp );
  exp = parse( str, 3 );
  upper = get_numval( exp );

  srand( (unsigned)time(NULL) );

  //  rnd = lower + (int)( (upper * rand()) / RAND_MAX );
  rnd = lower+(int) ((float)upper*rand()/(RAND_MAX+1.0));

  set_numvar(  var, rnd );
  free( exp );
  free( var );
}
