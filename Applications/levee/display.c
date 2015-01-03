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

#include <string.h>
#include <unistd.h>
/* do a gotoXY -- allowing -1 for same row/column */

#if TERMCAP | ST

#define MAXCOLS 160

#if TERMCAP
#include "termcap.i"
#endif

#else /*!(TERMCAP | ST)*/

#define MAXCOLS COLS

#endif

PROC
mvcur(int y, int x)
{
    static char gt[30];
   
    if (y == -1)
	y = curpos.y;
    else
	curpos.y = y;
    if (y >= LINES)
	y = LINES-1;
    if (x == -1)
	x = curpos.x;
    else
	curpos.x = x;
    if (x >= COLS)
	x = COLS-1;

#if ZTERM
    zgoto(x,y);
#endif

#if ANSI
    {	register char *p = gt;		/* make a ansi gotoXY string */
	*p++ = 033;
	*p++ = '[';
	numtoa(p,1+y); p += strlen(p);
	*p++ = ';';
	numtoa(p,1+x); p += strlen(p);
	*p++ = 'H';
	WRITE_TEXT(1, gt, (p-gt));
    }
#endif

#if VT52
    CM[2] = y+32;
    CM[3] = x+32;
    strput(CM);
#endif

#if TERMCAP
    tgoto(gt,y,x);
    strput(gt);
#endif
}

PROC numtoa(char *str, int num)
{
    int i = 10;			/* I sure hope that str is 10 bytes long... */
    bool neg = (num < 0);

    if (neg)
	num = -num;

    str[--i] = 0;
    do{
	str[--i] = (num%10)+'0';
	num /= 10;
    }while (num > 0);
    if (neg)
	str[--i] = '-';
    moveleft(&str[i], str, 10-i);
}

PROC
printi(int num)
{
    char nb[10];
    register int size;
    
    numtoa(nb,num);
    size = min(strlen(nb),COLS-curpos.x);
    if (size > 0) {
	zwrite(nb, size);
	curpos.x += size;
    }
}

PROC
println(void)
{
    strput("\n\r");
    curpos.x = 0;
    curpos.y = min(curpos.y+1, LINES-1);
}

/* print a character out in a readable form --
 *    ^<x> for control-<x>
 *    spaces for <tab>
 *    normal for everything else
 */

static char hexdig[] = "0123456789ABCDEF";

int PROC
format(char *out, unsigned c)
/* format: put a displayable version of c into out */
{
    if (c >= ' ' && c < '') {
    	out[0] = c;
    	return 1;
    }
    else if (c == '\t' && !list) {
	register int i;
	int size;

	for (i = size = tabsize - (curpos.x % tabsize);i > 0;)
	    out[--i] = ' ';
	return size;
    }
    else if (c < 128) {
    	out[0] = '^';
    	out[1] = c^64;
    	return 2;
    }
    else {
#if MSDOS
	out[0] = c;
	return 1;
#else
	out[0] = '\\';
	out[1] = hexdig[(c>>4)&017];
	out[2] = hexdig[c&017];
	return 3;
#endif
    }
}

PROC
printch(char c)
{
    register int size;
    char buf[MAXCOLS];

    size = min(format(buf,c),COLS-curpos.x);
    if (size > 0) {
	zwrite(buf, size);
	curpos.x += size;
    }
}

PROC
prints(char *s)
{
    int size,oxp = curpos.x;
    char buf[MAXCOLS+1];
    register bi = 0;

    while (*s && curpos.x < COLS) {
    	size = format(&buf[bi],*s++);
    	bi += size;
    	curpos.x += size;
    }
    size = min(bi,COLS-oxp);
    if (size > 0)
	zwrite(buf, size);
}

PROC
writeline(int y,int x,int start)
{
    int endd,oxp;
    register size;
    char buf[MAXCOLS+1];
    register bi = 0;
    
    endd = fseekeol(start);
    if (start==0 || core[start-1] == EOL)
	mvcur(y, 0);
    else
	mvcur(y, x);
    oxp = curpos.x;

    while (start < endd && curpos.x < COLS) {
    	size = format(&buf[bi],core[start++]);
    	bi += size;
    	curpos.x += size;
    }
    if (list) {
    	buf[bi++] = '$';
    	curpos.x++;
    }
    size = min(bi,COLS-oxp);
    if (size > 0) {
	zwrite(buf, size);
    }
    if (curpos.x < COLS)
	strput(CE);
}

/* redraw && refresh the screen */

PROC
refresh(int y,int x,int start,int endd, bool rest)
{
    int sp;
    
#if ST
    /* turn the cursor off */
    asm(" clr.l  -(sp)     ");
    asm(" move.w #21,-(sp) ");
    asm(" trap   #14       ");
    asm(" addq.l #6,sp     ");
#endif
    sp = start;
    while (sp <= endd) {
	writeline(y, x, sp);
	sp = 1+fseekeol(sp);
	y++;
	x = 0;
    }
    if (rest && sp >= bufmax)
	while (y<LINES-1) { /* fill screen with ~ */
	    mvcur(y, 0);
	    printch('~'); strput(CE);
	    y++;
	}
#if ST
    /* turn the cursor back on */
    asm(" clr.w  -(sp)     ");
    asm(" move.w #1,-(sp)  ");
    asm(" move.w #21,-(sp) ");
    asm(" trap   #14       ");
    asm(" addq.l #6,sp     ");
#endif
}

/* redraw everything */

PROC
redisplay(bool flag)
{
    if (flag)
	clrprompt();
    refresh(0, 0, ptop, pend, TRUE);
}
    
PROC
scrollback(int curr)
{
    mvcur(0,0);		/* move to the top line */
    do {
	ptop = bseekeol(ptop-1);
	strput(UpS);
	writeline(0, 0, ptop);
    } while (ptop > curr);
    setend();
}

PROC
scrollforward(int curr)
{
    do {
	writeline(LINES-1, 0, pend+1);
	strput("\n");
	pend = fseekeol(pend+1);
	ptop = fseekeol(ptop)+1;
    } while (pend < curr);
}

/* find if the number of lines between top && bottom is less than dofscroll */

bool PROC
ok_to_scroll(int top, int bottom)
{
    int nl, i;
    
    nl = dofscroll;
    i = top;
    do
	i += 1+scan(bufmax-i,'=',EOL, &core[i]);
    while (--nl > 0 && i < bottom);
    return(nl>0);
}

PROC
clrprompt(void)
{
    mvcur(LINES-1,0);
    strput(CE);
}

PROC
prompt(bool toot, char *s)
{
    if (toot)
	error();
    clrprompt();
    prints(s);
}
