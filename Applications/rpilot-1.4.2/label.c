/*
 * label.c - label handling code
 */

#include "rpilot.h"

#include <string.h>

label *new_label( char *name, line *lne, int linenum )
{
  label *l;

  l = (label *)malloc( sizeof(label) );
  l->linenum = linenum;
  l->stmnt = lne;
  l->name = new_string( name );
  l->next = NULL;

  return l;
}


label *get_label( char *name )
{
  label *l = (label *)rpi->lblhead;

  while( l != NULL ) {
    if( !strcasecmp(l->name, name) ) {
      return l; 
    }
    l = (label *)l->next;
  }
  
  err( BAD_LABEL, name );
}


void print_label( label *l )
{
    printf( "[line %d] %s: ", l->linenum, l->name );
    print_line( l->stmnt );
    printf( "\n" );
}

void print_label_list()
{
  label *l = (label *)rpi->lblhead;

  while( l != NULL ) {
    print_label( l );
    l = (label *)l->next;
  }

}



