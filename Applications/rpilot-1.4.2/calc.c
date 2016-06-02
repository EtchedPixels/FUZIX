/*
 * calc.c - handle simple mathematical expressions
 * rob - started july.25.2000
 * 
 * updates:
 *   - got around to finishing it - aug.11.2000
 *   - RPilot special code - aug.11.2000
 */

#include "rpilot.h"
#include "rstring.h"
#include "calc.h"
#include "var.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

int next_num( char *str, int *pos, int *status );
char next_tok( char *str, int *pos );
int find_match( char *str, int pos, char what, char match );
int read_num( char *str, int *pos );
int read_var( char *str, int *pos );

int calc( char *expr, int *status )
{
  int pos = 0;
  int num = 0, result = 0;
  char op = 0; 

  total_trim( expr );

  result = next_num( expr, &pos, status );

  while( pos < strlen(expr) ) {


    op = next_tok( expr, &pos );
    num = next_num( expr, &pos, status );


    switch( op ) {
    case 0 :  // invalid operand
      *status = CALC_NO_OP;
      return 0;
      break;
    case '+' :
      result += num;
      break;
    case '-' :
      result -= num;
      break;
    case '/' :
      result /= num;
      break;
    case '*' :
      result *= num;
      break;
    case '%' :
      result %= num;
      break;
    case '&' : 
      result &= num;
      break;
    case '|' :
      result |= num;
      break;
    case '^' :
      result ^= num;
      break;
    default:
      *status = CALC_BAD_OP;
      return 0;
      break;
    }
  }

  *status = CALC_SUCCESS;
  return result;
}




int next_num( char *str, int *pos, int *status )
{
  char *inparen, *tempstr;
  int result, rparen;
  int mult = 1;

  *pos = wspace( str, *pos );
  
  if( str[*pos] == '-' ) {
    mult = -1;
    *pos += 1;
  }
  if( str[*pos] == '(' ) {
    rparen = find_match( str, *pos+1, ')', '(' );
    inparen = new_string_from( str, *pos+1, rparen-*pos-1 );
    
    *pos = rparen+1;

    result = calc( inparen, status );
    free( inparen );
  } else if( str[*pos] == '#' ) {  // variable
    result = read_var( str, pos );
  } else {
    result = read_num( str, pos );
  }

  return result * mult;
}

/*
 * find_match()
 * Returns the position in the string `str' of the matching character.
 * Example: find_match( "((8*8)+9)/2", 1, ')', '(' ) => 8
 */
int find_match( char *str, int pos, char what, char match )
{
  int levels = 1;
  int i = pos;

  do {
    if( str[i] == what ) {
      levels--;
    } else if( str[i] == match ) {
      levels++;
    }
    i++;
  } while( levels != 0 );

  return i-1;
}


int read_num( char *str, int *pos )
{
  int start;
  int numchars;
  char *num;
  int result;

  start = wspace( str, *pos );
  numchars = start;

  while( isdigit(str[numchars]) ) {
    numchars++;
  }

  num = new_string_from( str, start, numchars - start );

  *pos = numchars;

  result = atoi( num );
  free( num );

  return result;
}
  

char next_tok( char *str, int *pos )
{
  int nows = wspace( str, *pos );
  *pos = nows;

  *pos += 1;  // increment position counter
  return str[nows];
}



int read_var( char *str, int *pos )
{
  int start;
  int numchars;
  char *var;
  int result;

  start = *pos;
  numchars = start + 1;

  while( isalpha(str[numchars]) ) {
    numchars++;
  }

  var = new_string_from( str, start, numchars - start );

  //  printf( "*** read_var(): var = \"%s\"\n", var );

  *pos = numchars;

  result = get_numvar( var );
  free( var );

  return result;
}
