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
/*
 * iRMX interface for levee (Intel C)
 */
#include "levee.h"
#if RMX

extern char FkL, CurRT, CurLT, CurUP, CurDN;

extern alien rq$s$write$move();

strput(s)
/* strput: write a string to stdout */
char *s;
{
    int dummy;

    if (s)
	rq$s$write$move(fileno(stdout), s, strlen(s), &dummy);
}

char
getKey()
/* getKey: read a character from stdin */
{
    char c,sw;
    unsigned dummy;

    read(0,&c,1);

    if (c == FkL) {	/* (single character) function key lead-in */
	dq$special(3,&fileno(stdin),&dummy);	/* grab a raw-mode character */
	if (read(0,&sw,1) == 1)
	    if (sw == CurLT)
		c = LTARROW;
	    else if (sw == CurRT)
		c = RTARROW;
	    else if (sw == CurUP)
		c = UPARROW;
	    else if (sw == CurDN)
		c = DNARROW;
	    else
		c = sw | 0x80;
	dq$special(1,&fileno(stdin),&dummy);	/* back into transparent mode */
    }
#if 0
    else if (c == 0x7f)	/* good old dos kludge... */
	return erase;
#endif
    return c;
}

int max(a,b)
int a,b;
{
    return (a>b)?a:b;
}

int min(a,b)
int a,b;
{
    return (a>b)?b:a;
}

extern alien token rq$c$create$command$connection(),
		   rq$c$delete$command$connection(),
		   rq$c$send$command();

int system(s)
/* system: do a shell escape */
char *s;
{
    char *string();
    unsigned cp, error, status, dummy;

    cp = rq$c$create$command$connection(fileno(stdin),fileno(stdout),0,&error);
    if (!error) {
	rq$c$send$command(cp,string(s),&status,&error);
	rq$c$delete$command$connection(cp,&dummy);
    }
    return error?(error|0x8000):(status&0x7fff);
}
#endif
