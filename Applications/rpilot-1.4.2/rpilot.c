/*
 * rpilot.c -- A simple PILOT interpretor in ANSI C
 * Copyright 1998, 2000 Rob Linwood (rob@auntfloyd.com)
 * Visit http://www.auntfloyd.com/ for more cool stuff
 *
 * See the file rpilot.txt for user documentation.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "rpilot.h"
#include "math.h"

rpinfo *rpi;
     
int readfile( char *filename )
{
  char inbuf[MAXLINE];  // 256-char limit on line width
  line *currline;
  line *templine = NULL;
  label *currlbl;
  label *templbl = NULL;
  int linenum = 0;
  char lastcmd = '!';
  FILE *f;


  if( (f = fopen(filename, "r")) == NULL ) {
    rpi = NULL;  /* kludge around the error printing */
    err( ERR_FILE, filename );
  }

  rpi->filename = new_string( filename );  

  currline = (line *)rpi->linehead;
  currlbl = (label *)rpi->lblhead;
  do {
    strset( inbuf, 0 );
    fgets( inbuf, MAXLINE-1, f );
    chop( inbuf );
    linenum++;
    trim( inbuf );
    if( strcmp(inbuf, "") ) {  // if the line isn't blank

#ifdef IGNORE_HASH
      if( (linenum == 1) && (inbuf[0] == '#') ) {
	break;
      }
#endif

      if( inbuf[0] == '*' ) { // label
	currlbl->next = (struct label *)new_label( inbuf+1, NULL, linenum );
	templbl = (label *)currlbl->next;
	currlbl = (label *)currlbl->next;
      } else if( (toupper(inbuf[0]) != 'R') && 
		 !((inbuf[0] == ':') && (lastcmd == 'R')) ) { 
	*(line **)&currline->next = new_line( inbuf, currline->cmd, linenum );
	currline = (line *)currline->next;
	if( templbl != NULL ) {
	  templbl->stmnt = (line *)currline;
	  templbl = NULL;
	}
	lastcmd = currline->cmd;  // next->cmd;
      } else {
	lastcmd = 'R';
      }
    }
  } while( !feof(f) );
} 


      

void handle( line *l )
{

  switch( toupper(l->cmd) ) {
  case 'A' : cmd_accept( l->args );
    break;
  case 'T' : cmd_type( l->args );
    break;
  case 'J' : cmd_jump( l->args );
    break;
  case 'U' : cmd_use( l->args );
    break;
  case 'E' : cmd_end( l->args );
    break;
  case 'M' : cmd_match( l->args );
    break;
  case 'C' : cmd_compute( l->args );
    break;
  case 'Y' : cmd_yes( l->args );
    break;
  case 'N' : cmd_no( l->args );
    break;
  case 'X' : cmd_execute( l->args );
    break;
  case 'S' : cmd_shell( l->args );
    break;
  case 'D' : cmd_debug( l->args );
    break;
  case 'G' : cmd_generate( l->args );
    break;
  }
}



int test( condex *cond )
{

  if( (cond->op == OP_YES) && (get_numvar("#MATCHED") == TRUE) ) {
    return TRUE;
  } else if( (cond->op == OP_NO) && (get_numvar("#MATCHED")==FALSE) ) {
    return TRUE;
  } else if( cond->op == OP_EQL ) {
    if( express(cond->lside) == express(cond->rside) ) {
      return TRUE;
    } else {
      return FALSE;
    }
  } else if( cond->op == OP_LT ) {
    if( express(cond->lside) < express(cond->rside) ) {
      return TRUE;
    } else {
      return FALSE;
    }
  } else if( cond->op == OP_GT ) {
    if( express(cond->lside) > express(cond->rside) ) {
      return TRUE;
    } else {
      return FALSE;
    }
  } else if( cond->op == OP_NEQL ) {
    if( express(cond->lside) != express(cond->rside) ) {
      return TRUE;
    } else {
      return FALSE;
    }
  } else if( cond->op == OP_LE ) {
    if( express(cond->lside) <= express(cond->rside) ) {
      return TRUE;
    } else {
      return FALSE;
    }
  } else if( cond->op == OP_GE ) {
    if( express(cond->lside) >= express(cond->rside) ) {
      return TRUE;
    } else {
      return FALSE;
    }
  }
  return FALSE;
}


void init()
{
  line *linehead;
  label *lblhead;
  strvar *svar;
  numvar *nvar;

  rpi = NULL;

  linehead = new_line( "", ' ', -1 );
  lblhead = new_label( "", NULL, -1 );
  rpi = (rpinfo *)xmalloc( sizeof(rpinfo) );

  rpi->linehead = (struct line *)linehead;
  rpi->lblhead = (struct label *)lblhead;
  rpi->numhead = (struct numvar *)new_numvar( "", -1 );
  rpi->strhead = (struct strvar *)new_strvar( "", "" );
  rpi->stk = (struct stack *)new_stack( NULL );
  rpi->status = STAT_RUN;
  rpi->error = -1;
  rpi->strict = TRUE;
  rpi->currline = rpi->linehead;
  
  svar = (strvar *)rpi->strhead;
  nvar = (numvar *)rpi->numhead;
  nvar->next = (struct numvar *)new_numvar( "#RPILOT", RP_VERSION );
  svar->next = (struct strvar *)new_strvar( "$PROMPT", ACCEPT_STR );
  
}

int interp()
{
  line *curr;

  while( rpi->currline != NULL ) {
    curr = (line *)rpi->currline;
    if( curr->cond != NULL ) {  // is there a conditional expression?
      if( test(curr->cond) == TRUE ) {  // if so, test it
	handle( curr );
      }
    } else {
      handle( curr );
    }
    // have any other functions changed the value of rpi->currline?
    if( (line *)rpi->currline == curr ) {
      rpi->currline = (struct line *)curr->next;
    } // Otherwise, keep the value of rpi->currline, cuz it's been modified
  }
}


int run( char *filename )
{

  init();
  readfile( filename );
  interp();
}


/*
 * the routines below are used in both commands (cmds.c) and the debugger
 * (debug.c), so are made as common procedures to both.
 */

void set_var( char *name, char *val )
{

  if( name[0] == '$' ) {  // string variable
    set_strvar( name, val );
  } else if( name[0] == '#' ) {  // numeric variable
    set_numvar( name, atoi(val) );
  } else {
    err( BAD_VAR, name  );
  }
    
}


int get_numval( char *str )
{
  if( str[0] == '#' ) {
    return( get_numvar(str) );
  } else {
    return( atoi( str ) );
  }
}

char *get_strval( char *str )
{
  if( str[0] == '$' ) {
    return get_strvar(str);
  } else {
    return str;
  }
}


void jump( char *str )
{
  label *l;

  l = get_label( get_strval(str) );

  rpi->currline = (struct line *)l->stmnt;
}


void use( char *str )
{
  label *l;
  line *curr;

  l = get_label( trim(get_strval(str)) );

  // push next command onto stack
  curr = (line *)rpi->currline;
  curr = (line *)curr->next;
  stk_push( (stack *)rpi->stk, curr ); 
  rpi->currline = (struct line *)l->stmnt;
}  


void execute( char *str )
{
  line *lne = new_line( trim(get_strval(str)), ' ', -1 );
  
  lne->cmd = toupper( lne->cmd );
  handle( lne );
}

