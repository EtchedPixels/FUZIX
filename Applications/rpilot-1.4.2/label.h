/*
  label.h - header for label.c
*/

#ifndef _label_h_
#define _label_h_

/* #include "line.h" */
#include "rpilot.h"

typedef struct {
  char *name;
  line *stmnt;
  int linenum;
  struct label *next;
} label;

label *get_label( char *name );
label *new_label( char *name, line *lne, int linenum );
void print_label( label *l );
void print_label_list();

#endif
