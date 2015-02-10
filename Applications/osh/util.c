/*
 * util.c - special built-in shell utilities for osh
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
 *	@(#)$Id: a2c541b8c9cade1c4faa1f606fe0e6c1555827ba $
 */
/*
 *	Derived from: osh-20080629
 *			fd2.c	(r404 2008-05-09 16:52:15Z jneitzel)
 *			goto.c	(r465 2008-06-25 22:11:58Z jneitzel)
 *			if.c	(r465 2008-06-25 22:11:58Z jneitzel)
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

#define	OSH_SHELL

#include "defs.h"
#include "err.h"
#include "pexec.h"
#include "sh.h"
#include "strtoint.h"

#define	IS_SBI(k)	\
	((k) == SBI_ECHO || (k) == SBI_FD2 || (k) == SBI_GOTO || (k) == SBI_IF)

static	int	sbi_echo(int, char **);
/*@maynotreturn@*/
static	int	sbi_fd2(int, char **);
/*@maynotreturn@*/
static	int	sbi_goto(int, char **);
/*@maynotreturn@*/
static	int	sbi_if(int, char **);

/*
 * Execute the shell utility specified by key w/ the argument
 * count ac and the argument vector pointed to by av.  Return
 * status s of the last call in the chain on success.  Do not
 * return on invalid-utility error.
 */
int
uexec(enum sbikey key, int ac, char **av)
{
	int (*util)(int, char **);
	int r;
	static int cnt, cnt1, s;

	if (cnt == 0) setmyerrexit(&ut_errexit);

	switch (key) {
	case SBI_ECHO:	util = &sbi_echo;	break;
	case SBI_FD2:	util = &sbi_fd2;	break;
	case SBI_GOTO:	util = &sbi_goto;	break;
	case SBI_IF:	util = &sbi_if;		break;
	default:
		err(FC_ERR, FMT2S, getmyname(), "uexec: Invalid utility");
		/*NOTREACHED*/
		return FC_ERR;
	}

	cnt1 = cnt++;

	r = (*util)(ac, av);

	if (cnt-- > cnt1)
		s = r;

#ifdef	DEBUG
#ifdef	DEBUG_PROC
	if (cnt == 0) fd_print(FD2, "uexec: return %d;\n", s);
#endif
#endif

	return s;
}

/*
 * NAME
 *	echo - write arguments to standard output
 *
 * SYNOPSIS
 *	echo [-n] [string ...]
 *
 * DESCRIPTION
 *	Echo writes its string arguments (if any) separated by
 *	blanks and terminated by a newline to the standard output.
 *	If `-n' is specified, the terminating newline is not written.
 */
static int
sbi_echo(int argc, char **argv)
{
	bool nopt;
	char **avp, **ave;

	argc--, argv++;
	if (*argv != NULL && EQUAL(*argv, "-n")) {
		argc--, argv++;
		nopt = true;
	} else
		nopt = false;

	for (avp = argv, ave = &argv[argc]; avp < ave; avp++)
		fd_print(FD1, "%s%s", *avp, (avp + 1 < ave) ? " " : "");
	if (!nopt)
		fd_print(FD1, FMT1S, "");

	return SH_TRUE;
}

/*@noreturn@*/
static	void	fd2_usage(void);

/*
 * NAME
 *	fd2 - redirect from/to file descriptor 2
 *
 * SYNOPSIS
 *	fd2 [-e] [-f file] [--] command [arg ...]
 *
 * DESCRIPTION
 *	See the fd2(1) manual page for full details.
 */
static int
sbi_fd2(int argc, char **argv)
{
	bool eopt;
	enum sbikey key;
	int efd, nfd, ofd, opt;
	char *file;

	setmyname(argv[0]);

	ofd = FD1, efd = FD2;
	eopt = false, file = NULL;
	while ((opt = getopt(argc, argv, ":ef:")) != -1)
		switch (opt) {
		case 'e':
			eopt = true;
			break;
		case 'f':
			file = optarg;
			break;
		default:
			fd2_usage();
		}
	argc -= optind;
	argv += optind;
	if (argc < 1)
		fd2_usage();

	if (file != NULL) {
		if ((nfd = open(file, O_WRONLY|O_APPEND|O_CREAT, 0666)) == -1)
			err(FC_ERR, FMT3S, getmyname(), file, ERR_CREATE);
		if (dup2(nfd, efd) == -1)
			err(FC_ERR, FMT2S, getmyname(), strerror(errno));
		if (eopt && dup2(efd, ofd) == -1)
			err(FC_ERR, FMT2S, getmyname(), strerror(errno));
		(void)close(nfd);
	} else {
		if (eopt)
			ofd = FD2, efd = FD1;
		if (dup2(ofd, efd) == -1)
			err(FC_ERR, FMT2S, getmyname(), strerror(errno));
	}

	/*
	 * Try to execute the specified command.
	 */
	key = cmd_lookup(argv[0]);
	if (IS_SBI(key))
		return uexec(key, argc, argv);

	(void)err_pexec(argv[0], argv);
	/*NOTREACHED*/
	return FC_ERR;
}

static void
fd2_usage(void)
{

	err(FC_ERR, FD2_USAGE, getmyname());
}

static	off_t	offset;

static	bool	getlabel(/*@out@*/ char *, int, size_t);
static	int	xgetc(void);

/*
 * NAME
 *	goto - transfer command
 *
 * SYNOPSIS
 *	goto label [...]
 *
 * DESCRIPTION
 *	See the goto(1) manual page for full details.
 */
static int
sbi_goto(int argc, char **argv)
{
	size_t siz;
	char label[LABELMAX];

	setmyname(argv[0]);

	if (argc < 2 || *argv[1] == EOS || isatty(FD0) != 0)
		err(FC_ERR, FMT2S, getmyname(), ERR_GENERIC);
	if ((siz = strlen(argv[1]) + 1) > sizeof(label))
		err(FC_ERR, FMT3S, getmyname(), argv[1], ERR_LABTOOLONG);
	if (lseek(FD0, (off_t)0, SEEK_SET) == -1)
		err(FC_ERR, FMT2S, getmyname(), ERR_SEEK);

	while (getlabel(label, *argv[1] & 0377, siz))
		if (strcmp(label, argv[1]) == 0) {
			(void)lseek(FD0, offset, SEEK_SET);
			return SH_TRUE;
		}

	fd_print(FD2, FMT3S, getmyname(), argv[1], ERR_LABNOTFOUND);
	return SH_FALSE;
}

/*
 * Search for the first occurrence of a possible label with both
 * the same first character (fc) and the same length (siz - 1)
 * as argv[1], and copy this possible label to buf.
 * Return true  (1) if possible label found.
 * Return false (0) at end-of-file.
 */
static bool
getlabel(char *buf, int fc, size_t siz)
{
	int c;
	char *b;

	while ((c = xgetc()) != EOF) {
		/* `:' may be preceded by blanks. */
		while (c == SPACE || c == TAB)
			c = xgetc();
		if (c != COLON) {
			while (c != EOL && c != EOF)
				c = xgetc();
			continue;
		}

		/* Prepare for possible label. */
		while ((c = xgetc()) == SPACE || c == TAB)
			;	/* nothing   */
		if (c != fc)	/* not label */
			continue;

		/*
		 * Try to copy possible label (first word only)
		 * to buf, ignoring it if it becomes too long.
		 */
		b = buf;
		do {
			if (c == EOL || c == SPACE || c == TAB || c == EOF) {
				*b = EOS;
				break;
			}
			*b = c;
			c = xgetc();
		} while (++b < &buf[siz]);

		/* Ignore any remaining characters on labelled line. */
		while (c != EOL && c != EOF)
			c = xgetc();
		if (c == EOF)
			break;

		if ((size_t)(b - buf) != siz - 1)	/* not label */
			continue;
		return true;
	}

	*buf = EOS;
	return false;
}

/*
 * If not at end-of-file, return the next character from the standard
 * input as an unsigned char converted to an int while incrementing
 * the global offset.  Otherwise, return EOF at end-of-file.
 */
static int
xgetc(void)
{
	int nc;

	offset++;
	nc = getchar();
	return (nc != EOF) ? nc & 0377 : EOF;
}

static	int	iac;
static	int	iap;
static	char	**iav;

/*@maynotreturn@*/
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
static int
sbi_if(int argc, char **argv)
{
	bool re;		/* return value of expr() */

	setmyname(argv[0]);

	if (argc > 1) {
		iac = argc;
		iav = argv;
		iap = 1;
		re  = expr();
		if (re && iap < iac)
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
	iap--;
	return re;
}

static bool
e1(void)
{
	bool re;

	re = e2();
	if (equal(nxtarg(RETERR), "-a"))
		return re & e1();
	iap--;
	return re;
}

static bool
e2(void)
{

	if (equal(nxtarg(RETERR), "!"))
		return !e3();
	iap--;
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
		err(FC_ERR, FMT3S, getmyname(), iav[iap - 2], ERR_EXPR);

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
				iap--;
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
	enum sbikey xak;
	char **xap, **xav;

	if (iap < 2 || iap > iac)	/* should never (but can) be true */
		err(FC_ERR, FMT2S, getmyname(), ERR_AVIINVAL);

	xav = xap = &iav[iap];
	while (*xap != NULL) {
		if (forked && equal(*xap, "}"))
			break;
		xap++;
	}
	if (forked && xap - xav > 0 && !equal(*xap, "}"))
		err(FC_ERR, FMT3S, getmyname(), iav[iap - 1], ERR_BRACE);
	*xap = NULL;
	if (xav[0] == NULL) {
		if (forked)
			err(FC_ERR, FMT3S, getmyname(), iav[iap - 1], ERR_COMMAND);
		else
			/* Currently unreachable; do not remove. */
			err(FC_ERR, FMT2S, getmyname(), ERR_COMMAND);
	}

	/* Invoke the ":" or "exit" special command. */
	if (equal(xav[0], ":"))
		_exit(SH_TRUE);
	if (equal(xav[0], "exit")) {
		(void)lseek(FD0, (off_t)0, SEEK_END);
		_exit(SH_TRUE);
	}

	xak = cmd_lookup(xav[0]);
	if (IS_SBI(xak)) {
		if (forked)
			_exit(uexec(xak, (int)(xap - xav), xav));
		else
			(void)uexec(xak, (int)(xap - xav), xav);
		return;
	}

	(void)err_pexec(xav[0], xav);
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

	if (ra && mode == X_OK && sheuid == 0) {
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

	if (iap < 1 || iap > iac)	/* should never (but can) be true */
		err(FC_ERR, FMT2S, getmyname(), ERR_AVIINVAL);

	if (iap == iac) {
		if (reterr) {
			iap++;
			return NULL;
		}
		err(FC_ERR, FMT3S, getmyname(), iav[iap - 1], ERR_ARGUMENT);
	}
	nap = iav[iap];
	iap++;
	return nap;
}

static bool
equal(const char *a, const char *b)
{

	if (a == NULL || b == NULL)
		return false;
	return EQUAL(a, b);
}
