/*
  rpilot.h - universal macros and the like.
*/

#ifndef _rpilot_h_
#define _rpilot_h_

#include "line.h"
#include "stack.h"
#include "label.h"
#include "var.h"
#include "cmds.h"
#include "condex.h"
#include "debug.h"
#include "err.h"
#include "rstring.h"
#include "rpinfo.h"
#include "bind.h"
#include "interact.h"

#include <stdio.h>
#include <stdlib.h>

#define MAXLINE 256         // Max length of a source line

// Status indicators
#define STAT_RUN 1
#define STAT_HALT 2
#define STAT_END 3


// Used to check conditional values 
#define FALSE 0
#define TRUE 1

// What string is displayed for an accept command?
#define ACCEPT_STR ">"


// Remove a terminating newline
#define chop( str )   if(str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0'

// Initialize randon number based on time
//#define srandom() srand( (unsigned)time(NULL) );


// The version number, of course
#define VERSION "1.4.2"
// The version number which can be gotten with the #RPILOT variable
#define RP_VERSION 14

// exported functions
int get_numval( char *str );
void set_var( char *name, char *val );
int run( char *filename );
void jump( char *str );
void use( char *str );
void execute( char *str );
char *get_strval( char *str );
void init(); 
int readfile( char *filename );
extern rpinfo *rpi;

#endif
