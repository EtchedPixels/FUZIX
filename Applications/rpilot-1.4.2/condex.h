// condex.h - condex type and functions

#ifndef _condex_h_
#define _condex_h_

#include "rpilot.h"
#include <stdio.h>

#define OP_NULL 0
#define OP_EQL 1
#define OP_LT 2
#define OP_GT 3
#define OP_NEQL 4
#define OP_LE 5
#define OP_GE 6
#define OP_YES 7
#define OP_NO 8

typedef struct {
  int op;
  char *rside;
  char *lside;
  struct condex *next;
} condex;

condex *new_condex( char *str );

void print_condex_to( condex *curr, FILE *stream );
void print_condex( condex *curr );
void print_condex_list( condex *head );
char *get_condex( condex *curr );

#endif
