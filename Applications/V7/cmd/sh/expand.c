/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 1999 Robert Nordier. All rights reserved. */

#
/*
 *	UNIX shell
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */

#include 	<dirent.h>
#include	<sys/types.h>
#define DIRSIZ 31
#include	<sys/stat.h>
#include	"defs.h"



/* globals (file name generation)
 *
 * "*" in params matches r.e ".*"
 * "?" in params matches r.e. "."
 * "[...]" in params matches character class
 * "[...a-z...]" in params matches a through z.
 *
 */

static void	addg();


INT	expand(as,rflg)
	STRING		as;
{
	INT		count, dirf;
	BOOL		dir=0;
	STRING		rescan = 0;
	REG STRING	s, cs;
	ARGPTR		schain = gchain;
	struct dirent	entry;
	STATBUF		statb;

	if (trapnote&SIGSET ) { return(0); ;}

	s=cs=as; entry.d_name[DIRSIZ-1]=0; /* to end the string */

	/* check for meta chars */
	{
	   REG BOOL slash; slash=0;
	   WHILE !fngchar(*cs)
	   DO	if (*cs++==0
		) {	if (rflg && slash ) { break; } else { return(0) ;}
		} else if ( *cs=='/'
		) {	slash++;
		;}
	   OD
	}

	for(;;) {	if (cs==s
		) {	s=nullstr;
			break;
		} else if ( *--cs == '/'
		) {	*cs=0;
			if (s==cs ) { s="/" ;}
			break;
		;}
	}
	if (stat(s,&statb)>=0
	    && (statb.st_mode&S_IFMT)==S_IFDIR
	    && (dirf=open(s,0))>0
	) {	dir++;
	;}
	count=0;
	if (*cs==0 ) { *cs++=0200 ;}
	if (dir
	) {	/* check for rescan */
		REG STRING rs; rs=cs;

		REP	if (*rs=='/' ) { rescan=rs; *rs=0; gchain=0 ;}
		PER	*rs++ DONE

		// FIXME: readdir
		WHILE read(dirf, (void *)&entry, 32) == 32 && (trapnote&SIGSET) == 0
		DO	if (entry.d_ino==0 ||
			    (*entry.d_name=='.' && *cs!='.')
			) {	continue;
			;}
			if (gmatch(entry.d_name, cs)
			) {	addg(s,entry.d_name,rescan); count++;
			;}
		OD
		close(dirf);

		if (rescan
		) {	REG ARGPTR	rchain;
			rchain=gchain; gchain=schain;
			if (count
			) {	count=0;
				WHILE rchain
				DO	count += expand(rchain->argval,1);
					rchain=rchain->argnxt;
				OD
			;}
			*rescan='/';
		;}
	;}

	{
	   REG CHAR	c;
	   s=as;
	   WHILE c = *s
	   DO	*s++=(c&STRIP?c:'/') OD
	}
	return(count);
}

gmatch(s, p)
	REG STRING	s, p;
{
	REG INT		scc;
	CHAR		c;

	if (scc = *s++
	) {	if ((scc &= STRIP)==0
		) {	scc=0200;
		;}
	;}
	switch(c = *p++) {
	    case '[':
		{BOOL ok; INT lc;
		ok=0; lc=077777;
		WHILE c = *p++
		DO	if (c==']'
			) {	return(ok?gmatch(s,p):0);
			} else if ( c==MINUS
			) {	if (lc<=scc && scc<=(*p++) ) { ok++ ;}
			} else {	if (scc==(lc=(c&STRIP)) ) { ok++ ;}
			;}
		OD
		return(0);
		}

	    default:
		if ((c&STRIP)!=scc ) { return(0) ;}

	    case '?':
		return(scc?gmatch(s,p):0);

	    case '*':
		if (*p==0 ) { return(1) ;}
		--s;
		WHILE *s
		DO  if (gmatch(s++,p) ) { return(1) ;} OD
		return(0);

	    case 0:
		return(scc==0);
	}
}

static void	addg(as1,as2,as3)
	STRING		as1, as2, as3;
{
	REG STRING	s1, s2;
	REG INT		c;

	s2 = locstak()+BYTESPERWORD;

	s1=as1;
	WHILE c = *s1++
	DO	if ((c &= STRIP)==0
		) {	*s2++='/';
			break;
		;}
		*s2++=c;
	OD
	s1=as2;
	WHILE *s2 = *s1++ DO s2++ OD
	if (s1=as3
	) {	*s2++='/';
		WHILE *s2++ = *++s1 DONE
	;}
	makearg(endstak(s2));
}

makearg(args)
	REG STRING	args;
{
	((ARGPTR)args)->argnxt=gchain;
	gchain=(ARGPTR)args;
}

