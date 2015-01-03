/*
 * LEVEE, or Captain Video;  A vi clone
 *
 * Copyright (c) 1982-1997 David L Parsons
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by David L Parsons (orc@pell.chi.il.us).  My name may not be used
 * to endorse or promote products derived from this software without
 * specific prior written permission.  THIS SOFTWARE IS PROVIDED
 * AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "levee.h"
#include "extern.h"
#include <stdio.h>
#include <unistd.h>
/* read in a file -- return TRUE -- read file
			    FALSE-- file too big
*/

int PROC addfile(FILE *f, int start, int endd, int *size)
{
    register int chunk;

    chunk = read(fileno(f), core+start, (endd-start)-1);

    *size = chunk;
    return chunk < (endd-start)-1;
}


/* write out a file -- return TRUE if ok. */

bool PROC putfile(FILE *f, int start, int endd)
{
    write(fileno(f), core+start, endd-start);
}
