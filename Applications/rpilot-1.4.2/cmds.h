/*
  cmds.h - header for cmds.c
*/

#ifndef _cmds_h_
#define _cmds_h_

#include "rpilot.h"

// use() implememnts PILOT's version of GOSUB
void cmd_use( char *str );
// Handles variable assignment
void cmd_compute( char *str );
// Handles user input
void cmd_accept( char *str );
// Displays data
void cmd_type( char *str );
// Marks the end of a subroutine
void cmd_end( char *str );
// Does string matching
void cmd_match( char *str );
// PILOT's version of GOTO
void cmd_jump( char *str );
// Displays text if #matched equals YES
void cmd_yes( char *str );
// Displays text if #matched equals NO
void cmd_no( char *str );


// The following are nonstandard functions available in rpilot programs 

// Executes a line of PILOT code
void cmd_execute( char *str );
// Allows access to the operating system
void cmd_shell( char *str );
// Gives debugging info from inside a PILOT programs
void cmd_debug( char *str );
// Puts a random number in a given variable
void cmd_generate( char *str );


#endif

