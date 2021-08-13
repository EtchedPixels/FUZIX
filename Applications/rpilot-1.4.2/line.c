/*
 * line.c - RPilot syntax handling routines
 */

#include "rpilot.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


// the input str should already be trim()ed

line *new_line( char *str, char lastcmd, int linenum )
{
  line *l = (line *)xmalloc( sizeof(line) );
  int colon; 
  int rparen, lparen;
  int i;
  char *contemp = NULL;  // temp variable for condex string


  colon = findchar( str, ':' );  // get the position of the colon
  l->next = NULL;
  l->linenum = linenum;

  // is it a blank string?
  if( !strcmp(str, "") ) {
    l->cmd = lastcmd;
    l->args = NULL;
    l->cond = NULL;
    return l;
  }
  
  if( colon == -1 ) {  // if there is no colon, it's an error
    err( NO_COLON, str );
  }


  /* Notes for the 3/3/00 rewrite
   * There are 3 different possiblities for what will be the first character
   * in the line:
   *
   * 1) A letter signifing the command name (this includes 'Y' and 'N')
   * 2) A colon, meaning that we should use the last command for this one, too
   * 3) A left parentheses, like above, but with a conditional expression
   *
   */

  if( colon != strlen(str)-1 ) {
    l->args = new_string_from( str, colon+1, strlen(str)-colon+1 );
    ltrim( l->args );
  } else {
    l->args = new_string( "" );
  }


  /* 
   * Here, we'll handle the second case first, because it is the easiest.
   * The command is the same as the last command, and everything after the
   * first character constitutes the arguments.
   */

  if( str[0] == ':' ) {
    l->cmd = lastcmd;
    l->cond = NULL;
    return l;
  }


  /*
   * Now the third case.  Like the second case, everything after the colon
   * forms the arguments.  But we have to find the right parentheses in order
   * to create a conditional expression.  The command is the same as the
   * previous one.
   */

  if( str[0] == '(' ) {
    l->cmd = lastcmd;

    rparen = findchar( str, ')' );
    if( rparen == -1 ) {
      // If there is no right parentheses, we signal an error
      err( NO_RPAREN, str );
    }
    // otherwise, create a temporary string, and make a new condex
    //    contemp = new_string_from( str, 1, rparen-1 );
    contemp = new_string_from( str, 1, rparen-1 );
    l->cond = new_condex( contemp );
    free( contemp );

    return l;
  }


  /*
   * Finally, the first case.  This itself has four possibilities:
   *
   * 1) There is no conditional expression
   * 2) There is a conditional expression in parentheses
   * 3) There is a 'Y' as the conditional
   * 4) There is an 'N' as the conditional
   */
  
  l->cmd = str[0];
  //  l->args = new_string_from( str, colon+1, strlen(str)-colon+1 );

  lparen = findchar( str, '(' );
  if( (lparen == -1)|| (lparen > colon) ) {
    // Now, we can see if there is a Y or N somehwere.. 
    for( i=1; i<colon; i++ ) {
      if( toupper(str[i]) == 'Y' ) {
	l->cond = new_condex( "Y" );
	return l;
      } else if( toupper(str[i]) == 'N' ) {
	l->cond = new_condex( "N" );
	return l;
      }
    }
    // If we didn't find Y or N, there is no condex, so just return
    l->cond = NULL;
    return l;
    
  }

  // Now we have a conditional expression, so create a new condex and return
  rparen = findchar( str, ')' );
  if( rparen == -1 ) {
    err( NO_RPAREN, str );
  }
  //  contemp = new_string_from( str, lparen+1, rparen-1 );
  contemp = new_string_from( str, lparen+1, rparen-2 );
  l->cond = new_condex( contemp );
  free( contemp );

  return l;
}

/* support binding */
void print_line_to( line *curr, FILE *stream )
{
  if( curr == NULL ) {
    fprintf( stream, "[NULL]" );
    return;
  }
  fprintf( stream, "%c", curr->cmd );
  if( curr->cond != NULL ) {
    if( (curr->cond->op == OP_YES) || (curr->cond->op == OP_NO) ) {
      print_condex_to( curr->cond, stream );
    } else {
      fprintf( stream, "(" );
      print_condex_to( curr->cond, stream );
      fprintf( stream, ")" );
    }
  }
  fprintf( stream, ": %s", curr->args );
}

char *get_line( line *curr )
{
  char *buffer;
  char *cond;

  buffer = (char *)malloc(1024);
  
  if( curr == NULL ) {
    return "(null)";
  }

  sprintf( buffer, "%c", curr->cmd );
  if( curr->cond != NULL ) {
    if( (curr->cond->op == OP_YES) || (curr->cond->op == OP_NO) ) {
      sprintf( buffer, "%c", curr->cond->op );
    } else {
      cond = get_condex( curr->cond );
      sprintf( buffer, "(%s)", cond );
      free( cond );
    }
  }

  sprintf( buffer, ": %s", curr->args );

  return buffer;
}
      

void print_line( line *curr )
{
  print_line_to( curr, stdout );
}


void print_line_list( line *head ) 
{
  line *curr = head;
  
  while( curr ) {
    printf( "#%d ", curr->linenum );
    print_line( curr );
    printf( "\n" );
    curr = (line *)curr->next;
  }
}


#ifdef TEST

int main( int argc, char *argv[] )
{

  FILE *f;
  char buf[256];
  line *head, *curr;
  int linenum = 0;

  if( argc < 2 ) {
    puts( "line: Usage line filename" );
    return 0;
  }

  f = fopen( argv[1], "r" );

  head = new_line("", ' ', 0 );
  curr = head;

  do {
    strset( buf, 0 );
    fgets( buf, 255, f );
    chop( buf );
    trim( buf );
puts( buf );
    if( strcmp(buf, "") ) {
      curr->next = (struct line *)new_line( buf, 'Q', ++linenum );
      curr = (line *)curr->next;
    }
  } while( !feof(f) );

  
  print_line_list( head );

  return 0;
}


#endif
