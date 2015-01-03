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
#include <ctype.h>
#include <string.h>

/* driver for movement commands */

findstates PROC
findCP(int curp, int *newpos, cmdtype cmd)
{
    static char chars[2] = {'/','?'};
    char tsearch;

    *newpos = ERR;
    switch (cmd) {			/* move around */
    
    case GO_LEFT:
	*newpos = max(lstart, curp-max(count,1));
	break;

    case GO_RIGHT:
	*newpos = min(lend, curp+max(count,1));
	break;

    case GO_UP:
    case GO_DOWN:
	*newpos = nextline(cmd==GO_DOWN, curp, count);
	if (*newpos >= 0 && *newpos < bufmax)
	    *newpos = findcol(*newpos,xp);
	break;

    case FORWD:
    case TO_WD:
    case BACK_WD:
    case BTO_WD:
	*newpos = moveword(curp,(cmd <= TO_WD),(cmd==TO_WD || cmd==BTO_WD));
	break;

    case NOTWHITE:
	*newpos = skipws(bseekeol(curp));
	break;

    case TO_COL:
	*newpos = findcol(curp, count);
	break;

    case TO_EOL:
	while ( (count-- > 1) && (curp < bufmax) )
	    curp = 1+fseekeol(curp);
	*newpos = fseekeol(curp);
	break;

    case PARA_FWD:
	do
	    curp = findfwd("^*[ \t$",curp+1,bufmax-1);
	while (curp != ERR && --count > 0);
	*newpos = (curp==ERR)?bufmax:curp;
	break;

    case PARA_BACK:
	do
	    curp = findback("^*[ \t$",curp-1,0);
	while (curp != ERR && --count > 0);
	*newpos = (curp==ERR)?0:curp;
	break;

    case SENT_FWD:
    case SENT_BACK:
	*newpos = sentence(curp, cmd==SENT_FWD);
	break;

    case MATCHEXPR:
	*newpos = match(curp);
	break;

    case TO_CHAR:
    case UPTO_CHAR:
    case BACK_CHAR:
    case BACKTO_CHAR:
	ch = readchar();
	if (ch == ESC)
	    return ESCAPED;
	if (cmd<=UPTO_CHAR) {
	    *newpos = fchar(curp,*newpos);
	    if (cmd==UPTO_CHAR && *newpos>=0)
		*newpos = max(curp, *newpos-1);
	}
	else {
	    *newpos = bchar(curp,*newpos);
	    if (cmd==BACKTO_CHAR && *newpos>=0)
		*newpos = min(curp, *newpos+1);
	}
	break;

    case PAGE_BEGIN:
	*newpos = ptop;
	break;

    case PAGE_END:
	*newpos = pend;
	break;

    case PAGE_MIDDLE:
	curp = ptop;
	count = 12;
	while (count-- > 0 && curp < bufmax)
	   curp = 1+fseekeol(curp);
	*newpos = skipws(curp);
	break;

    case GLOBAL_LINE:
	if (count <= 0)
	    *newpos = bufmax-1;
	else
	    *newpos = to_index(count);
	break;

    case TO_MARK:
    case TO_MARK_LINE:
	*newpos = getcontext((char)tolower(readchar()), cmd==TO_MARK_LINE);
	break;

    case CR_FWD:
    case CR_BACK:
	curp = nextline(cmd==CR_FWD, curp, count);
	if (cmd==CR_BACK && curp > 0)
	    curp = bseekeol(curp);
	*newpos = skipws(curp);
	break;

    case PATT_FWD:
    case PATT_BACK:			/* search for pattern */
    case FSEARCH:
    case BSEARCH:
	clrprompt();
	if (cmd == PATT_FWD || cmd == PATT_BACK) {
	    printch(tsearch = instring[0] = chars[cmd-PATT_FWD]);
	    if (!getline(&instring[1]))
		return ESCAPED;	/* needs to skip later tests */
	}
	else {
	    if (!nlsearch)
		return BADMOVE;
	    tsearch = nlsearch;
	    printch(instring[0] = (cmd==FSEARCH)?nlsearch:((nlsearch=='?')?'/':'?') );
	    prints(lastpatt);
	    instring[1] = 0;
	}
	if (*findparse(instring, newpos, curp)) { /* croaked patt */
	    *newpos = ERR;
	    prompt(TRUE,"bad pattern");
	}
	else if (*newpos == ERR)
	    prompt(FALSE,"no match");
	nlsearch = tsearch;	/* fixup for N, n */
	break;
    }
    if ( ((*newpos) >= 0) && ((*newpos) <= bufmax) )
	return LEGALMOVE;
    return BADMOVE;
}

/* this procedure handles all movement in visual mode */

PROC
movearound(cmdtype cmd)
{
    int cp;
    
    switch (findCP(curr, &cp, cmd)) {
	case LEGALMOVE:
	    if (cp < bufmax) {
		if (cmd >= PATT_FWD)		/* absolute move */
		    contexts[0] = curr;	/* so save old position... */
		curr = cp;			/* goto new position */
		if (cmd==GO_UP || cmd==GO_DOWN) /* reset Xpos */
		    deranged = TRUE;
		else
		    xp = setX(cp);		/* just reset XP */
		if (cp < lstart || cp > lend) {
		    lstart = bseekeol(curr);
		    lend   = fseekeol(curr);
		    if (curr < ptop) {
			if (canUPSCROLL && ok_to_scroll(curr, ptop)) {
			    scrollback(curr);
			    yp = 0;
			}
			else {
			    yp = settop(LINES / 2);
			    redisplay(TRUE);
			}
		    }
		    else if (curr > pend) {
			if (ok_to_scroll(pend, curr)) {
			    scrollforward(curr);
			    yp = LINES-2;
			}
			else {
			    yp = settop(LINES / 2);
			    redisplay(TRUE);
			}
		    }
		    else
			yp = setY(curr);
		}
	    }
	    else
		error();
	break;
	case BADMOVE:
	    error();
	break;
    }
    mvcur(yp, xp);
}
    
int PROC
findcol(int ip, int col)
{
    int tcol, endd;
    
    ip = bseekeol(ip);			/* start search here */
    endd = fseekeol(ip);		/* end search here */

    tcol = 0;
    while (tcol < col && ip < endd)
	switch (cclass(core[ip++])) {
	    case 0: tcol++;    break;
	    case 1: tcol += 2; break;
	    case 3: tcol += 3; break;
	    case 2: tcol = tabsize*(1+(tcol/tabsize)); break;
	}
    return(ip);
}

char dstpatt[]="[](){}", srcpatt[]="][)(}{";

/* find matching [], (), {} */

int PROC
match(int p)
{
    char srcchar, dstchar;
    int lev, step;
    
    while((lev = scan(6,'=',core[p],srcpatt)) >= 6 && core[p] != EOL)
	p++;
    if (lev < 6) {
	srcchar = srcpatt[lev];
	dstchar = dstpatt[lev];
	step = setstep[lev&1];
	lev = 0;
	while (p >= 0 && p < bufmax) {
	    p += step;
	    if (core[p] == srcchar)
		lev++;
	    else if (core[p] == dstchar)
		if(--lev < 0)
		    return p;
	}
    }
    return (-1);
}

char * PROC
class(char c)
/* find the character class of a char -- for word movement */
{
    if (strchr(wordset,c))
	return wordset;
    else if (strchr(spaces,c))
	return spaces;
    else
	return (char*)NULL;
}

int PROC
skip(char *chars, int cp, int step)
/* skip past characters in a character class */
{
    while (cp >= 0 && cp < bufmax && strchr(chars,core[cp]))
	cp += step;
    return cp;
}

int PROC
tow(int cp, int step)
/* skip to the start of the next word */
{
    while (cp >= 0 && cp < bufmax
		 && !(strchr(wordset,core[cp]) || strchr(spaces,core[cp])))
	cp += step;
    return cp;
}

int PROC
moveword(int cp, bool forwd, bool toword)
/* word movement */
{
    int step;
    register char *ccl;
    
    step = setstep[forwd];	/* set direction to move.. */
    if (!toword)
	cp += step;		/* advance one character */
    count = max(1,count);
    ccl = class(core[cp]);
    if (toword && ccl == spaces) {	/* skip to start of word */
	count--;
	cp = skip(spaces,cp,step);
	ccl = class(core[cp]);
    }

    while (cp >= 0 && cp < bufmax && count-- > 0) {
	if (ccl == spaces) {
	    cp = skip(spaces,cp,step);
	    ccl = class(core[cp]);
	}
	cp = (ccl)?skip(ccl,cp,step):tow(cp,step);
	ccl = class(core[cp]);
    }
    if (toword) {				/* past whitespace? */
	if (ccl == spaces)
	    cp = skip(spaces,cp,step);
	return cp;
    }
    return cp-step;				/* sit on last character. */
}

/* find a character forward on current line */

int PROC
fchar(int pos, int npos)
{
    do
	pos += scan(lend-pos-1,'=',ch, &core[pos+1]) + 1;
    while (--count>0 && pos<lend);
    if (pos<lend)
	return(pos);
    return(npos);
}

/* find a character backward on the current line */

int PROC
bchar(int pos, int npos)
{
    do
	pos += scan(-pos+lstart+1,'=',ch, &core[pos-1]) - 1;
    while (--count>0 && pos>=lstart);
    if (pos>=lstart)
	return(pos);
    return(npos);
}

/* look for the end of a sentence forward */

int PROC
ahead(int i)
{
    char c;

    do {
	if ((c=core[i]) == '.' || c == '?' || c == '!')
	    if (i == bufmax-1 || (c=core[1+i]) == TAB || c == EOL || c == ' ')
		return i;

    } while (++i < bufmax);
    return -1;
}

/* look for the end of a sentence backwards. */

int PROC
back(int i)
{
    char c;

    do {
	if ((c=core[i]) == '.' || c == '?' || c == '!')
	    if ((c=core[1+i]) == TAB || c == EOL || c == ' ')
		return i;

    } while (--i >= 0);
    return -1;
}

/* find the end of the next/last sentence.
    Sentences are delimited by ., !, or ? followed by a space.
*/

int PROC
sentence(int start, bool forwd)
{
    do {
	if (forwd)
	    start = ahead(start+1);
	else
	    start = back(start-1);
    } while (--count > 0 && start >= 0);
    return start;
}
