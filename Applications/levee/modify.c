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
#include "grep.h"
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

/* modification commands that can be accessed by either editcore || execmode */

/* put stuff into the yank buffer */

bool PROC
doyank(int low, int high)
{
    HANDLE f;
    register sz;
    
    yank.size = high - low;
    moveleft(&core[low], yank.stuff, min(yank.size, SBUFSIZE));
    if (yank.size > SBUFSIZE) {
	if ((f=OPEN_NEW(yankbuf)) >= 0) {
	    low += SBUFSIZE;
	    sz = WRITE_TEXT(f, core+low, high-low);
	    CLOSE_FILE(f);
	    if (sz == high-low)
		return TRUE;
	}
	yank.size = -1;
	return FALSE;
    }
    return TRUE;
}

bool PROC
deletion(int low, int high)
{
    if (doyank(low, high))		/* fill yank buffer */
	return delete_to_undo(&undo, low, high-low);
    return FALSE;
}

/* move stuff from the yank buffer into core */

bool PROC
putback(int start, int *newend)
{
    int siz, st;
    HANDLE f;
    
    if (yank.size+bufmax < SIZE && yank.size > 0) {
	*newend = start + yank.size;
	if (start < bufmax)
	    moveright(&core[start], &core[start+yank.size], bufmax-start);
	moveleft(yank.stuff, &core[start], min(SBUFSIZE, yank.size));
	if (yank.size > SBUFSIZE) {
	    siz = yank.size - SBUFSIZE;
	    if ((f=OPEN_OLD(yankbuf)) >= 0) {
		st = READ_TEXT(f, &core[start+SBUFSIZE], siz);
		CLOSE_FILE(f);
		if (st == siz)
		    goto succeed;
	    }
	    moveleft(&core[start+yank.size], &core[start], bufmax-start);
	    *newend = -1;
	    return FALSE;
	}
    succeed:
	insert_to_undo(&undo, start, yank.size);
	return TRUE;
    }
    return FALSE;
}

#define DSIZE 1024

int PROC
makedest(char *str, int start, int ssize, int size)
/* makedest: make the replacement string for an regular expression */
{
    register char *fr = dst;
    register char *to = str;
    int as, asize, c;

    while (*fr && size >= 0) {
	if (*fr == AMPERSAND) {	/* & copies in the pattern that we matched */
	    if ((size -= ssize) < 0)
		return -1;
	    moveleft(&core[start],to,ssize);
	    to += ssize;
	    fr++;
	}
	else if (*fr == ESCAPE) {	/* \1 .. \9 do arguments */
	    c = fr[1];
	    fr += 2;
	    if (c >= '1' && c <= '9') {
		if ((as = RE_start[c-'1']) < 0)
		    continue;
		asize = RE_size [c-'1'];
		if ((size -= asize) < 0)
		    return -1;
		moveleft(&core[as],to,asize);
		to += asize;
	    }
	    else
		*to++ = c;
	}
	else {
	    *to++ = *fr++;
	    --size;
	}
    }
    return to-str;
}

int PROC
chop(int start, int *endd, bool visual, bool *query)
{
    int i,retval;
    char c;
/*>>>>
    bool ok;
  <<<<*/
    char dest[DSIZE];
    register int len, dlen;
    
    retval = -1;
    /*dlen = strlen(dst);*/
restart:
    count = 1;
    i = findfwd(pattern, start, *endd);
    if (i != ERR) {
	if (*query) {
	    /*>>>> don't delete -- keep for future use
	    if (visual) {
		mvcur(yp,setX(i));puts("?");
	    }
	    else {
	    <<<<*/
		println();
		writeline(-1,-1,bseekeol(i));
		println();
		mvcur(-1,setX(i));
		prints("^?");
	    /*>>>>
	    }
	    <<<<*/
	    do
		c = tolower(readchar());
	    while (c!='y'&&c!='n'&&c!='q'&&c!='a');
	    if (c == 'n') {
		start = i+1;
		goto restart;
	    }
	    else if (c == 'q')
		return retval;
	    else if (c == 'a')
		*query = FALSE;
	}
	len = lastp-i;
	dlen = makedest(dest, i, len, DSIZE);
	if (dlen >= 0 && bufmax-(int)(len+dlen) < SIZE
		      && delete_to_undo(&undo, i, len)) {
	    modified = TRUE;
	    if (dlen > 0) {
		moveright(&core[i], &core[i+dlen], bufmax-i);
		insert_to_undo(&undo,i,dlen);
		moveleft(dest,&core[i],dlen);
	    }
	    *endd += (dlen-len);
	    retval = i+dlen;
	}
    }
    return retval;
}
