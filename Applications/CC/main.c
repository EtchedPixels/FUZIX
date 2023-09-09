/*
 *	Compiler pass main loop
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "compiler.h"

FILE *debug;

unsigned deffunctype;		/* The type of an undeclared function */
unsigned funcbody;		/* Parser global for function body */
unsigned voltrack;		/* Track possible volatiles */
unsigned in_sizeof;		/* Set if we are in sizeof() */
/*
 *	A C program consists of a series of declarations that by default
 *	are external definitions.
 */
static void toplevel(void)
{
	if (token == T_TYPEDEF) {
		next_token();
		dotypedef();
	} else {
		funcbody = 0;
		voltrack = 0;
		target_reginit();
		declaration(S_EXTDEF);
	}
}

/* A function defined by use is taken to be int f(); */
static unsigned functype[2] = {
	1, ELLIPSIS
};

int main(int argc, char *argv[])
{
	next_token();
	init_nodes();
	/* A function with no type info returning INT */
	deffunctype = make_function(CINT, functype);
#ifdef DEBUG
	if (argv[1]) {
		debug = fopen(argv[1], "w");
		if (debug == NULL) {
			perror(argv[1]);
			return 255;
		}
	}
#endif
	while (token != T_EOF)
		toplevel();
	/* No write out any uninitialized variables */
	write_bss();
	out_write();
	return errors;
}
