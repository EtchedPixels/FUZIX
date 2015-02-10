/*
 * err.c - shell and utility error-handling routines
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
 *	@(#)$Id: ff52cd09d9d131d45ad9ffbf0fa38ff28143f240 $
 */

#include "defs.h"
#include "err.h"

#define	FMTMAX		SBUFMM(2)
#define	MSGMAX		(FMTMAX + LINEMAX)

#define	MYNAME		"unknown"

static	void		(*myerrexit)(int)	= NULL;
/*@observer@*/
static	const char	*myname			= NULL;
static	pid_t		mypid			= -1;

static	void		wmsg(int, const char *, va_list);

/*
 * Handle all errors for the calling process.  This includes printing
 * any specified non-NULL message to the standard error and calling
 * the error exit function pointed to by the global myerrexit.
 * This function may or may not return.
 */
void
err(int es, const char *msgfmt, ...)
{
	va_list va;

	if (msgfmt != NULL) {
		va_start(va, msgfmt);
		wmsg(FD2, msgfmt, va);
		va_end(va);
	}
	if (myerrexit == NULL) {
		fd_print(FD2, FMT1S, "err: Invalid myerrexit function pointer");
		abort();
	}

	(*myerrexit)(es);
}

/*
 * Print any specified non-NULL message to the file descriptor pfd.
 */
void
fd_print(int pfd, const char *msgfmt, ...)
{
	va_list va;

	if (msgfmt != NULL) {
		va_start(va, msgfmt);
		wmsg(pfd, msgfmt, va);
		va_end(va);
	}
}

/*
 * Get the line number for the offset of the file descriptor FD0.
 * Return line number on success.
 * Return -1 on error.
 */
long
get_lnum(void)
{
	struct stat sb;
	off_t o, o1;
	long n;
	UChar c;
	bool b;

	if (fstat(FD0, &sb) == -1 || !S_ISREG(sb.st_mode))
		return -1;
	if ((o  = lseek(FD0, (off_t)0, SEEK_CUR)) == -1)
		return -1;
	if ((o1 = lseek(FD0, (off_t)0, SEEK_SET)) !=  0)
		return -1;
	for (b = false, n = 0; o1 != -1 && o1 < o; ) {
		if (read(FD0, &c, (size_t)1) != 1)
			break;
		o1 = lseek(FD0, (off_t)0, SEEK_CUR);
		if (c == EOS) {
			b = true;
			continue;
		}
		if (c == EOL)
			if (n > -1 && n < LONG_MAX)
				n += 1;
	}
	if (b)
		if (n > -1 && n < LONG_MAX)
			n += 1;
	n = (lseek(FD0, o, SEEK_SET) == o && n > -1 && n < LONG_MAX) ? n : -1;

#ifdef	DEBUG
	fd_print(FD2, "get_lnum: n == %ld;\n", n);
#endif

	return n;
}

/*
 * Return a pointer to the global myname on success.
 * Return a pointer to "(null)" on error.
 *
 * NOTE: Must call setmyname(argv[0]) first.
 */
const char *
getmyname(void)
{

	if (myname == NULL)
		return "(null)";

	return myname;
}

/*
 * Return the global mypid on success.
 * Return 0 on error.
 *
 * NOTE: Must call setmypid(getpid()) first.
 */
pid_t
getmypid(void)
{

	if (mypid == -1)
		return 0;

	return mypid;
}

/*
 * Set the global myerrexit to the function pointed to by func.
 */
void
setmyerrexit(void (*func)(int))
{

	if (func == NULL)
		return;

	myerrexit = func;
}

/*
 * Set the global myname to the basename of the string pointed to by s.
 */
void
setmyname(const char *s)
{
	const char *p;

	if (s != NULL && *s != EOS) {
		if ((p = strrchr(s, SLASH)) != NULL)
			p++;
		else
			p = s;
		if (*p == HYPHEN && *(p + 1) != EOS)
			p++;
		else if (*p == EOS)
			/* should never (but can) be true */
			p = MYNAME;
	} else
		/* should never (but can) be true */
		p = MYNAME;

	myname = p;
}

/*
 * Set the global mypid to the process ID p.
 */
void
setmypid(const pid_t p)
{

	if (mypid != -1)
		return;

	mypid = p;
}

/*
 * Cause all shell utility processes to exit appropriately on error.
 * This function is called by err() and never returns.
 */
void
ut_errexit(int es)
{

#ifdef	DEBUG
	fd_print(FD2, "ut_errexit: es == %d: Call %s(%d);\n",
	    es, (getpid() == getmypid()) ? "exit" : "_exit", es);
#endif

	EXIT(es);
}

/*
 * Write the specified message to the file descriptor wfd.
 * A diagnostic is written to FD2 on error.
 */
static void
wmsg(int wfd, const char *msgfmt, va_list va)
{
	char msg[MSGMAX];
	char fmt[FMTMAX];
	struct iovec ev[4];
	int i;

	i = snprintf(fmt, sizeof(fmt), "%s", msgfmt);
	if (i >= 1 && i < (int)sizeof(fmt)) {
		i = vsnprintf(msg, sizeof(msg), fmt, va);
		if (i >= 0 && i < (int)sizeof(msg)) {
			if (write(wfd, msg, strlen(msg)) == -1) {
				ev[0].iov_base = (char *)getmyname();
				ev[0].iov_len  = strlen(getmyname());
				ev[1].iov_base = ": ";
				ev[1].iov_len  = (size_t)2;
				ev[2].iov_base = ERR_WRITE;
				ev[2].iov_len  = strlen(ERR_WRITE);
				ev[3].iov_base = "\n";
				ev[3].iov_len  = (size_t)1;
				(void)writev(FD2, ev, 4);
			}
		}
	}
}
