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
 * flexos interface for levee (Metaware C)
 */
#include "levee.h"

#if FLEXOS
#include <flexos.h>
#include <ctype.h>

static int oldkmode;
static int oldsmode;

int
min(a,b)
{
    return (a>b) ? b : a;
}

int
max(a,b)
{
    return (a<b) ? b : a;
}

strput(s)
char *s;
{
    s_write(0x01, 1L, s, (long)strlen(s), 0L);
}

char *
basename(s)
 char *s;
{
    char *strrchr();
    register char *p;

    if ((p=strrchr(s,'/')) || (p=strrchr(s,'\\')) || (p = strrchr(s,':')))
	return 1+p;
    return s;
}

getKey()
{
    char c;

    s_read(0x0101, 0L, &c, 1L, 0L);
    return c;
}

initcon()
{
    CONSOLE tty;

    s_get(T_CON, 1L, &tty, SSIZE(tty));
    oldkmode = tty.con_kmode;
    oldsmode = tty.con_smode;
    tty.con_kmode = 0x0667;
    tty.con_smode = 0;
    s_set(T_CON, 1L, &tty, SSIZE(tty));
}

fixcon()
{
    CONSOLE tty;

    s_get(T_CON, 1L, &tty, SSIZE(tty));
    tty.con_kmode = oldkmode;
    tty.con_smode = oldsmode;
    s_set(T_CON, 1L, &tty, SSIZE(tty));
}

char *
strdup(s)
char *s;
{
    char *malloc();
    char *p;

    if (p=malloc(strlen(s)+1))
	strcpy(p, s);
    return p;
}

getpid()
{
    PROCESS myself;

    s_get(T_PDEF, 0L, &myself, SSIZE(myself));

    return myself.ps_pid;
}

strlwr(s)
char *s;
{
    while (*s) {
	if (isupper(*s))
	    *s += 32;
	s++;
    }
}
#endif
