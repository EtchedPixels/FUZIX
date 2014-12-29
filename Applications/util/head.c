/*
  Copyright (c) 1987,1997, Prentice Hall
  All rights reserved.
  
  Redistribution and use of the MINIX operating system in source and
  binary forms, with or without modification, are permitted provided
  that the following conditions are met:
  
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
  
     * Redistributions in binary form must reproduce the above
       copyright notice, this list of conditions and the following
       disclaimer in the documentation and/or other materials provided
       with the distribution.
  
     * Neither the name of Prentice Hall nor the names of the software
       authors or contributors may be used to endorse or promote
       products derived from this software without specific prior
       written permission.
  
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS, AUTHORS, AND
  CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL PRENTICE HALL OR ANY AUTHORS OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/* head - print the first few lines of a file	Author: Andy Tanenbaum */

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT 10

void do_file(int n, FILE *f);
void usage(void);

int main(int argc, char *argv[])
{
  FILE *f;
  int n, k, nfiles;
  char *ptr;

  /* Check for flag.  Only flag is -n, to say how many lines to print. */
  k = 1;
  ptr = argv[1];
  n = DEFAULT;
  if (argc > 1 && *ptr++ == '-') {
	k++;
	n = atoi(ptr);
	if (n <= 0) usage();
  }
  nfiles = argc - k;

  if (nfiles == 0) {
	/* Print standard input only. */
	do_file(n, stdin);
	exit(0);
  }

  /* One or more files have been listed explicitly. */
  while (k < argc) {
	if (nfiles > 1) printf("==> %s <==\n", argv[k]);
	if ((f = fopen(argv[k], "r")) == NULL)
		fprintf(stderr, "%s: cannot open %s: %s\n",
			argv[0], argv[k], strerror(errno));
	else {
		do_file(n, f);
		fclose(f);
	}
	k++;
	if (k < argc) printf("\n");
  }
  return(0);
}


void do_file(int n, FILE *f)
{
  int c;

  /* Print the first 'n' lines of a file. */
  while (n) switch (c = getc(f)) {
	    case EOF:
		return;
	    case '\n':
		--n;
	    default:	putc((char) c, stdout);
	}
}


void usage(void)
{
  fprintf(stderr, "Usage: head [-n] [file ...]\n");
  exit(1);
}
