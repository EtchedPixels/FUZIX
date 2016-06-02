/*
 * main.c - main() function for RPilot
 */

#include "rpilot.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int main( int argc, char *argv[] )
{
  int i;
  int showban = TRUE, inter = FALSE, bind = FALSE, mainfunc = FALSE, 
    showhelp = FALSE;
  char *filename=NULL, *outfile=NULL, *dataname=NULL, *funcname=NULL;

  for( i=1; i<argc; i++ ) {

    /*    printf("argv[%d] = %s\n", i, argv[i] ); */

    if( !strcasecmp(argv[i], "-b") ) {
      showban = FALSE;
    } else if( !strcasecmp(argv[i], "-i") ) {
      inter = TRUE;
    } else if( !strcasecmp(argv[i], "-?") ) {
      showhelp = TRUE;
    } else if( !strcasecmp(argv[i], "-c") ) {
      bind = TRUE;
    } else if( !strcasecmp(argv[i], "-o") ) {

      if( argc == i+1 ) {
	err( NO_CLARGS, "-o" );
      } else {
	outfile = new_string( argv[i+1] );
	i++;
      }
    } else if( !strcasecmp(argv[i], "-d") ) {
      if( argc == i+1 ) {
	err( NO_CLARGS, "-d" );
      } else {
	dataname = new_string( argv[i+1] );
	i++;
      }
    } else if( !strcasecmp(argv[i], "-f") ) {
      if( argc == i+1 ) {
	err( NO_CLARGS, "-f" );
      } else {
	funcname = new_string( argv[i+1] );
	i++;
      }
    } else if( !strcasecmp(argv[i], "-m") ) {
      mainfunc = TRUE;
    } else {
      filename = new_string( argv[i] );
    }
  }

/*    printf("filename=\"%s\", outfile=\"%s\", dataname=\"%s\", funcname=\"%s\"\n", */
/*  	 filename, outfile, dataname, funcname ); */
/*    printf("showban=%d, inter=%d, mainfunc=%d\n", showban, inter, mainfunc ); */


  if( argc == 1 ) {  // no arguments
    banner();
  } else {  // there were arguments
    if( showhelp == TRUE ) {
      if( showban == TRUE ) { banner(); }
      help(); 
    } else if( inter == TRUE ) {
      if( showban == TRUE ) { banner(); }
      interact();
    } else if( filename == NULL ) {
      if( showban == TRUE ) { banner(); }
      err( NO_FILE, "" );
    } else if( bind == TRUE ) {
      bindfile( filename, outfile, dataname, funcname, mainfunc );
    } else {
      run( filename );
    }
  }

  return 0;
}


void help() 
{
  printf( "Usage: rpilot [switch] filename\n" );
  printf( "Switch \t Action\n" );
  printf( "-b \t Suppress banner printing on startup\n" );
  printf( "-i \t Enter interactive mode when no file names are given\n");
  //  printf( "-c \t Bind a file as a C program\n" );
  printf( "-? \t Print this help message, then exit\n\n" );

}

void banner()
{
  printf( "RPilot: Rob's PILOT Interpreter, version %s\n", VERSION );
  printf( "Copyright 1998,2002 Rob Linwood (rcl211@nyu.edu)\n" );
  printf( "RPilot is Free Software. Please see http://rpilot.sf.net/\n" );
  printf( "For help, try `rpilot -?'\n\n" );
}
  
