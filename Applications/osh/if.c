/*
 * if.c - a port of the Sixth Edition (V6) UNIX conditional command
 */
/*-
 * Copyright (c) 2004-2015
 *	Jeffrey Allen Neitzel <jan (at) v6shell (dot) org>.
 *	All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY JEFFREY ALLEN NEITZEL ``AS IS'', AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL JEFFREY ALLEN NEITZEL BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	@(#)$Id: 416ec8fc211a8802e16c66a3369278cf4ea27b05 $
 */
/*
 *	Derived from:
 *		- Sixth Edition UNIX	/usr/source/s1/if.c
 *		- Seventh Edition UNIX	/usr/src/cmd/test.c
 */
/*-
 * Copyright (C) Caldera International Inc.  2001-2002.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code and documentation must retain the above
 *    copyright notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed or owned by Caldera
 *      International, Inc.
 * 4. Neither the name of Caldera International, Inc. nor the names of other
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * USE OF THE SOFTWARE PROVIDED FOR UNDER THIS LICENSE BY CALDERA
 * INTERNATIONAL, INC. AND CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL CALDERA INTERNATIONAL, INC. BE LIABLE FOR ANY DIRECT,
 * INDIRECT INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "defs.h"
#include "err.h"
#include "pexec.h"
#include "sasignal.h"
#include "strtoint.h"

const char	*name;
bool		no_lnum	= true;	/* no line number flag */

static	int	ac;
static	int	ap;
static	char	**av;
static	uid_t	ifeuid;

/*@noreturn@*/
static	void	doex(bool);
static	bool	e1(void);
static	bool	e2(void);
static	bool	e3(void);
static	bool	equal(/*@null@*/ const char *, /*@null@*/ const char *);
static	bool	expr(void);
static	bool	ifaccess(/*@null@*/ const char *, int);
static	bool	ifstat1(/*@null@*/ const char *, mode_t);
static	bool	ifstat2(/*@null@*/ const char *, /*@null@*/ const char *, int);
/*@null@*/
static	char	*nxtarg(bool);

/*
 * NAME
 *	if - conditional command
 *
 * SYNOPSIS
 *	if [expression [command [arg ...]]]
 *
 * DESCRIPTION
 *	See the if(1) manual page for full details.
 */
int
main(int argc, char **argv)
{
	bool re;		/* return value of expr() */

	setmyerrexit(&ut_errexit);
	setmyname(argv[0]);
	setmypid(getpid());

	ifeuid = geteuid();

	/*
	 * Set-ID execution is not supported.
	 */
	if (ifeuid != getuid() || getegid() != getgid())
		err(FC_ERR, FMT2S, getmyname(), ERR_SETID);

	(void)sasignal(SIGCHLD, SIG_DFL);

	if (argc > 1) {
		ac = argc;
		av = argv;
		ap = 1;
		re = expr();
		if (re && ap < ac)
			doex(!FORKED);
	} else
		re = false;

	return re ? SH_TRUE : SH_FALSE;
}

/*
 * Evaluate the expression.
 * Return true (1) or false (0).
 */
static bool
expr(void)
{
	bool re;

	re = e1();
	if (equal(nxtarg(RETERR), "-o"))
		return re | expr();
	ap--;
	return re;
}

static bool
e1(void)
{
	bool re;

	re = e2();
	if (equal(nxtarg(RETERR), "-a"))
		return re & e1();
	ap--;
	return re;
}

static bool
e2(void)
{

	if (equal(nxtarg(RETERR), "!"))
		return !e3();
	ap--;
	return e3();
}

static bool
e3(void)
{
	bool re;
	pid_t cpid, tpid;
	long ai, ci;
	int cstat, d;
	char *a, *b, *c;

	if ((a = nxtarg(RETERR)) == NULL)
		err(FC_ERR, FMT3S, getmyname(), av[ap - 2], ERR_EXPR);

	/*
	 * Deal w/ parentheses for grouping.
	 */
	if (equal(a, "(")) {
		re = expr();
		if (!equal(nxtarg(RETERR), ")"))
			err(FC_ERR, FMT3S, getmyname(), a, ERR_PAREN);
		return re;
	}

	/*
	 * Execute { command [arg ...] } to obtain its exit status.
	 */
	if (equal(a, "{")) {
		if ((cpid = fork()) == -1)
			err(FC_ERR, FMT2S, getmyname(), ERR_FORK);
		if (cpid == 0)
			/**** Child! ****/
			doex(FORKED);
		else {
			/**** Parent! ****/
			tpid = wait(&cstat);
			while ((a = nxtarg(RETERR)) != NULL && !equal(a, "}"))
				;	/* nothing */
			if (a == NULL)
				ap--;
			return (tpid == cpid && cstat == 0) ? true : false;
		}
	}

	/*
	 * file existence/permission tests
	 */
	if (equal(a, "-e"))
		return ifaccess(nxtarg(!RETERR), F_OK);
	if (equal(a, "-r"))
		return ifaccess(nxtarg(!RETERR), R_OK);
	if (equal(a, "-w"))
		return ifaccess(nxtarg(!RETERR), W_OK);
	if (equal(a, "-x"))
		return ifaccess(nxtarg(!RETERR), X_OK);

	/*
	 * file existence/type tests
	 */
	if (equal(a, "-d"))
		return ifstat1(nxtarg(!RETERR), S_IFDIR);
	if (equal(a, "-f"))
		return ifstat1(nxtarg(!RETERR), S_IFREG);
	if (equal(a, "-h"))
		return ifstat1(nxtarg(!RETERR), S_IFLNK);
	if (equal(a, "-s"))
		return ifstat1(nxtarg(!RETERR), F_GZ);
	if (equal(a, "-t")) {
		/* Does the descriptor refer to a terminal device? */
		if ((b = nxtarg(RETERR)) == NULL || *b == EOS) {
			err(FC_ERR, FMT3S, getmyname(), a, ERR_DIGIT);
			/*NOTREACHED*/
		}
		if (*b >= '0' && *b <= '9' && *(b + 1) == EOS) {
			d = *b - '0';
			if (IS_DIGIT(d, *b))
				return isatty(d) != 0;
		}
		err(FC_ERR, FMT3S, getmyname(), b, ERR_BADDIGIT);
	}

	/*
	 * binary comparisons
	 */
	if ((b = nxtarg(RETERR)) == NULL)
		err(FC_ERR, FMT3S, getmyname(), a, ERR_OPERATOR);
	if (equal(b,  "="))
		return  equal(a, nxtarg(!RETERR));
	if (equal(b, "!="))
		return !equal(a, nxtarg(!RETERR));
	if (equal(b, "<"))
		return strcmp(a, nxtarg(!RETERR)) < 0;
	if (equal(b, ">"))
		return strcmp(a, nxtarg(!RETERR)) > 0;
	if (equal(b, "-ef"))
		return ifstat2(a, nxtarg(!RETERR), F_EF);
	if (equal(b, "-nt"))
		return ifstat2(a, nxtarg(!RETERR), F_NT);
	if (equal(b, "-ot"))
		return ifstat2(a, nxtarg(!RETERR), F_OT);
	if (equal(b, "-eq") || equal(b, "-ne") || equal(b, "-gt") ||
	    equal(b, "-ge") || equal(b, "-lt") || equal(b, "-le")) {
		if ((c = nxtarg(RETERR)) == NULL)
			err(FC_ERR, FMT3S, getmyname(), b, ERR_INTEGER);
		if (strtoint(a, &ai) && strtoint(c, &ci)) {
			if (equal(b, "-eq"))
				return ai == ci;
			if (equal(b, "-ne"))
				return ai != ci;
			if (equal(b, "-gt"))
				return ai >  ci;
			if (equal(b, "-ge"))
				return ai >= ci;
			if (equal(b, "-lt"))
				return ai <  ci;
			if (equal(b, "-le"))
				return ai <= ci;
		} else
			err(FC_ERR, NULL);
	}
	err(FC_ERR, FMT3S, getmyname(), b, ERR_OPUNKNOWN);
	/*NOTREACHED*/
	return false;
}

static void
doex(bool forked)
{
	char **xap, **xav;
	int i;
	const char **tav;
	const char *cmd;

	if (ap < 2 || ap > ac)	/* should never (but can) be true */
		err(FC_ERR, FMT2S, getmyname(), ERR_AVIINVAL);

	xav = xap = &av[ap];
	while (*xap != NULL) {
		if (forked && equal(*xap, "}"))
			break;
		xap++;
	}
	if (forked && xap - xav > 0 && !equal(*xap, "}"))
		err(FC_ERR, FMT3S, getmyname(), av[ap - 1], ERR_BRACE);
	*xap = NULL;
	if (xav[0] == NULL) {
		if (forked)
			err(FC_ERR, FMT3S, getmyname(),av[ap - 1],ERR_COMMAND);
		else
			/* Currently unreachable; do not remove. */
			err(FC_ERR, FMT2S, getmyname(), ERR_COMMAND);
	}

	/* Invoke the ":" or "exit" special command. */
	if (equal(xav[0], ":"))
		EXIT(SH_TRUE);
	if (equal(xav[0], "exit")) {
		(void)lseek(FD0, (off_t)0, SEEK_END);
		EXIT(SH_TRUE);
	}

	cmd = xav[0];
	if (IS_LIBEXEC(equal)) {
		for (i = 0; xav[i] != NULL; i++)
			;	/* nothing */
		if ((tav = malloc((i + 1) * sizeof(char *))) == NULL) {
			err(FC_ERR, FMT2S, getmyname(), ERR_NOMEM);
			/*NOTREACHED*/
		}
		if (equal(cmd, "fd2"))
			tav[0] = FD2_PATH;
		else if (equal(cmd, "goto"))
			tav[0] = GOTO_PATH;
		else
			tav[0] = IF_PATH;
		cmd = tav[0];
		(void)memcpy(&tav[1], &xav[1], i * sizeof(char *));
		(void)err_pexec(cmd, (char *const *)tav);
	} else
		(void)err_pexec(cmd, xav);
}

/*
 * Check access permission for file according to mode while
 * dealing w/ special cases for the superuser and `-x':
 *	- Always grant search access on directories.
 *	- If not a directory, require at least one execute bit.
 * Return true  (1) if access is granted.
 * Return false (0) if access is denied.
 */
static bool
ifaccess(const char *file, int mode)
{
	struct stat sb;
	bool ra;

	if (file == NULL || *file == EOS)
		return false;

	ra = access(file, mode) == 0;

	if (ra && mode == X_OK && ifeuid == 0) {
		if (stat(file, &sb) < 0)
			ra = false;
		else if (S_ISDIR(sb.st_mode))
			ra = true;
		else
			ra = (sb.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH)) != 0;
	}

	return ra;
}

static bool
ifstat1(const char *file, mode_t type)
{
	struct stat sb;
	bool rs;

	if (file == NULL || *file == EOS)
		return false;

	if (type == S_IFLNK) {
		if (lstat(file, &sb) < 0)
			rs = false;
		else
			rs = (sb.st_mode & S_IFMT) == type;
	} else if (stat(file, &sb) < 0)
		rs = false;
	else if (type == S_IFDIR || type == S_IFREG)
		rs = (sb.st_mode & S_IFMT) == type;
	else if (type == F_GZ)
		rs = sb.st_size > (off_t)0;
	else
		rs = false;

	return rs;
}

static bool
ifstat2(const char *file1, const char *file2, int act)
{
	struct stat sb1, sb2;
	bool rs;

	if (file1 == NULL || *file1 == EOS)
		return false;
	if (file2 == NULL || *file2 == EOS)
		return false;

	if (stat(file1, &sb1) < 0)
		return false;
	if (stat(file2, &sb2) < 0)
		return false;

	if (act == F_OT)
		rs = sb1.st_mtime < sb2.st_mtime;
	else if (act == F_NT)
		rs = sb1.st_mtime > sb2.st_mtime;
	else if (act == F_EF)
		rs = sb1.st_dev == sb2.st_dev && sb1.st_ino == sb2.st_ino;
	else
		rs = false;

	return rs;
}

static char *
nxtarg(bool reterr)
{
	char *nap;

	if (ap < 1 || ap > ac)	/* should never (but can) be true */
		err(FC_ERR, FMT2S, getmyname(), ERR_AVIINVAL);

	if (ap == ac) {
		if (reterr) {
			ap++;
			return NULL;
		}
		err(FC_ERR, FMT3S, getmyname(), av[ap - 1], ERR_ARGUMENT);
	}
	nap = av[ap];
	ap++;
	return nap;
}

static bool
equal(const char *a, const char *b)
{

	if (a == NULL || b == NULL)
		return false;
	return EQUAL(a, b);
}
