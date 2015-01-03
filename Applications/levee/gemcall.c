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
 * Gemdos (Atari ST) bindings for levee (Alcyon/Sozobon C)
 */
#include "levee.h"

#if ST
#include <atari\osbind.h>

strput(s)
register char *s;
{
    write(1, s, strlen(s));
}


zwrite(s,len)
char *s;
{
    write(1, s, len);
}


min(a,b)
register int a, b;
{
    return (a<b)?a:b;
}


max(a,b)
register int a, b;
{
    return (a>b)?a:b;
}


unsigned
getKey()
/* get input from the keyboard. All odd keys (function keys, et al) that
 * do not produce a character have their scancode orred with $80 and returned.
 */
{
    unsigned c;
    long key;

    c = (key = Crawcin()) & 0xff;
    if (!c)
	c = (((unsigned)(key>>16))|0x80) & 0xff;
    return c;
} /* getKey */
#endif /*ST*/
