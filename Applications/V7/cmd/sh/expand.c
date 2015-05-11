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

static void addg(const char *as1, char *as2, const char *as3);


int expand(char *as, int rflg)
{
	int count, dirf;
	BOOL dir = 0;
	STRING rescan = 0;
	register char *s, *cs;
	ARGPTR schain = gchain;
	STATBUF statb;
	/* Use the internal API to avoid sucking in readdir and thus malloc */
	struct __dirent entry;

	if (trapnote & SIGSET)
		return (0);

	s = cs = as;
	entry.d_name[DIRSIZ - 1] = 0;	/* to end the string */

	/* check for meta chars */
	{
		register BOOL slash;
		slash = 0;
		while (!fngchar(*cs)) {
			if (*cs++ == 0) {
				if (rflg && slash) {
					break;
				} else {
					return (0);
				}
			} else if (*cs == '/') {
				slash++;
			}
		}
	}

	for (;;) {
		if (cs == s) {
			s = (char *)nullstr;
			break;
		} else if (*--cs == '/') {
			*cs = 0;
			if (s == cs) {
				s = "/";
			}
			break;
		}
	}
	if (stat(s, &statb) >= 0
	    && (statb.st_mode & S_IFMT) == S_IFDIR
	    && (dirf = open(s, 0)) > 0) {
		dir++;
	}
	count = 0;
	if (*cs == 0) {
		*cs++ = 0200;
	}
	if (dir) {		/* check for rescan */
		register char *rs;
		rs = cs;

		do {
			if (*rs == '/') {
				rescan = rs;
				*rs = 0;
				gchain = 0;
			}
		} while (*rs++);

		/* We don't want to use opendir as it uses calloc and sucks in malloc so we get
		   down and dirty */
		while (_getdirent(dirf, (void *) &entry, 32) == 32 && (trapnote & SIGSET) == 0) {
			if (entry.d_ino == 0 || (*entry.d_name == '.' && *cs != '.'))
				continue;
			if (gmatch(entry.d_name, cs)) {
				addg(s, entry.d_name, rescan);
				count++;
			}
		}
		close(dirf);

		if (rescan) {
			register ARGPTR rchain;
			rchain = gchain;
			gchain = schain;
			if (count) {
				count = 0;
				while (rchain) {
					count += expand(rchain->argval, 1);
					rchain = rchain->argnxt;
				}
			}
			*rescan = '/';
		}
	}

	{
		register char c;
		s = as;
		while (c = *s)
			*s++ = (c & STRIP ? c : '/');
	}
	return count;
}

int gmatch(register char *s, register char *p)
{
	register int scc;
	char c;

	if (scc = *s++) {
		if ((scc &= STRIP) == 0) {
			scc = 0200;
		}
	}
	switch (c = *p++) {
	case '[':
		{
			BOOL ok;
			int lc;
			ok = 0;
			lc = 077777;
			while (c = *p++) {
				if (c == ']') {
					return (ok ? gmatch(s, p) : 0);
				} else if (c == MINUS) {
					if (lc <= scc && scc <= (*p++)) {
						ok++;
					}
				} else {
					if (scc == (lc = (c & STRIP))) {
						ok++;
					}
				}
			}
			return (0);
		}

	default:
		if ((c & STRIP) != scc)
			return (0);

	case '?':
		return (scc ? gmatch(s, p) : 0);

	case '*':
		if (*p == 0)
			return 1;
		--s;
		while (*s) {
			if (gmatch(s++, p))
				return 1;
		}
		return 0;

	case 0:
		return (scc == 0);
	}
}

static void addg(const char *as1, char *as2, const char *as3)
{
	register const char *s1;
	register char *s2;
	register int c;

	s2 = locstak() + BYTESPERWORD;

	s1 = as1;
	while (c = *s1++) {
		if ((c &= STRIP) == 0) {
			*s2++ = '/';
			break;
			;
		}
		*s2++ = c;
	}
	s1 = as2;
	while (*s2 = *s1++) {
		s2++;
	}
	if (s1 = as3) {
		*s2++ = '/';
		while (*s2++ = *++s1);
	}
	makearg(endstak(s2));
}

void makearg(register char *args)
{
	((ARGPTR) args)->argnxt = gchain;
	gchain = (ARGPTR) args;
}
