// condex.c - conditional expression handling

#include "rpilot.h"
#include <string.h>
#include <stdio.h>


condex *new_condex( char *str )
{
  int i = -1;
  condex *c;

  c = (condex *)malloc( sizeof(condex) );
  c->next = NULL;

  if( !strcmp(str, "") ) {
    c->lside = NULL;
    c->rside = NULL;
    c->op = OP_NULL;
    return c;
  }

  if( !strcmp(str, "Y") ) {
    c->op = OP_YES;
    return c;
  } else if( !strcmp(str, "N") ) {
    c->op = OP_NO;
    return c;
  }

  for(i=0; i<strlen(str); i++) {
    if( (str[i]=='=') || (str[i]=='>') || (str[i]=='<') || (str[i]=='!') ) {
      break;
    }
  }

  // we can't find a relational operator
  if( i == strlen(str) ) {
    printf("str=\"%s\"\n", str );
    err( NO_RELAT, str );
    return NULL;
  }

  if( str[i] == '=' ) {
    c->op = OP_EQL;
  } else if( (str[i] == '>') && (str[i+1] != '=') ) {
    c->op = OP_GT;
  } else if( (str[i] == '<') && (str[i+1] != '=') && (str[i+1] != '>') ) {
    c->op = OP_LT;
  } else if( (str[i] == '>') && (str[i+1] == '=') ) {
    c->op = OP_GE;
  } else if( (str[i] == '<') && (str[i+1] == '=') ) {
    c->op = OP_LE;
  } else if( (str[i] == '<') && (str[i+1] == '>') ) {
    c->op = OP_NEQL;
  } else { // Unknown operation
    err( BAD_RELAT, str );
  }

  if( (c->op==OP_EQL) || (c->op==OP_GT) || (c->op==OP_LT) ) {
    c->lside = new_string_from( str, 0, i );
    c->rside = new_string_from( str, i+1, strlen(str)-i );
  } else {
    c->lside = new_string_from( str, 0, i );
    c->rside = new_string_from( str, i+2, strlen(str)-i-1 );
  }

  return c;
}

/* hack to support binding */
void print_condex_to( condex *curr, FILE *stream )
{
  fprintf( stream, "%s ", curr->lside );
  switch( curr->op ) {
  case OP_EQL : fprintf( stream, "=" );
    break;
  case OP_NEQL : fprintf( stream, "<>" );
    break;
  case OP_GT : fprintf( stream, ">" );
    break;
  case OP_LT : fprintf( stream, "<" );
    break;
  case OP_GE : fprintf( stream, ">=" );
    break;
  case OP_LE : fprintf( stream, "<=" );
    break;
  case OP_YES : fprintf( stream, "Y" );
    break;
  case OP_NO : fprintf( stream, "N" );
    break;
  }
  fprintf( stream, " %s", curr->rside );
}


char *get_condex( condex *curr )
{
  char *buffer;

  buffer = (char *)malloc(1024);
  
  sprintf( buffer, "%s ", curr->lside );
  
  switch( curr->op ) {
  case OP_EQL : strcat( buffer, "=" );
    break;
  case OP_NEQL : strcat( buffer, "<>" );
    break;
  case OP_GT : strcat( buffer, ">" );
    break;
  case OP_LT : strcat( buffer, "<" );
    break;
  case OP_GE : strcat( buffer, ">=" );
    break;
  case OP_LE : strcat( buffer, "<=" );
    break;
  case OP_YES : strcat( buffer, "Y" );
    break;
  case OP_NO : strcat( buffer, "N" );
    break;
  }
  sprintf( buffer, " %s", curr->rside );

  return buffer;
}

void print_condex( condex *curr )
{
  print_condex_to( curr, stdout );
}


void print_condex_list( condex *head )
{
  condex *curr = head;
  
  while( curr ) {
    print_condex( curr );
    printf( "\n" );
    curr = (condex *)curr->next;
  }
}


#ifdef TEST

int main( int argc, char *argv[] )
{

  FILE *f;
  char buf[256];
  condex *head, *curr;

  if( argc < 2 ) {
    puts( "condex: Usage condex filename" );
    return 0;
  }

  f = fopen( argv[1], "r" );

  head = new_condex("");
  curr = head;

  do {
    strset( buf, 0 );
    fgets( buf, 255, f );
    chop( buf );
    if( strcmp(buf, "") ) {
      curr->next = (struct condex *)new_condex( buf );
      curr = (condex *)curr->next;
    }
  } while( !feof(f) );

  
  print_condex_list( head );

  return 0;
}

#endif

