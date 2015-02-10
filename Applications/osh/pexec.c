/*-
 * Copyright (c) 2005-2015
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
 *	@(#)$Id: 6551bee58cfc3e6c85292521c58776f50245eab7 $
 */
/*
 *	Derived from:
 *		- /usr/src/lib/libc/gen/execvp.c:
 *			$NetBSD: execvp.c,v 1.24 2003/08/07 16:42:47 agc Exp $
 */
/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)exec.c	8.1 (Berkeley) 6/4/93
 */

#include "defs.h"
#include "err.h"
#include "pexec.h"

extern char **environ;

/*
 * Execute a file or path name.
 */
int
pexec(const char *file, char *const *argv)
{
	const char **esh_argv;
	size_t dlen, flen;
	int cnt, eacces = 0;
	const char *d, *esh, *f, *pnp, *upp;
	char pnbuf[PATHMAX];

	/*
	 * Fail if the value of file, argv, or argv[0] is invalid.
	 */
	errno = 0;
	if (file == NULL || argv == NULL || argv[0] == NULL) {
		errno = EINVAL;
		goto fail;
	}
	if (*file == EOS || *argv[0] == EOS)
		goto fail;
	flen = strlen(file);

	/*
	 * If the name of the specified file contains one or more
	 * `/' characters, it is used as the path name to execute.
	 */
	for (f = file; *f != EOS; f++)
		if (*f == SLASH) {
			pnp = file;
			upp = "";
			goto exec_pathname;
		}
	*pnbuf = EOS;
	pnp = pnbuf;

	/*
	 * Get the user's PATH.  Fail if PATH is unset or is
	 * set to the empty string, as no PATH search shall be
	 * performed in such a case.
	 */
	upp = getenv("PATH");
	if (upp == NULL || *upp == EOS)
		goto fail;

	do {
		/* Find the end of this PATH element. */
		for (d = upp; *upp != COLON && *upp != EOS; upp++)
			;	/* nothing */
		/*
		 * Since this is a shell PATH, double, leading, and/or
		 * trailing colons indicate the current directory.
		 */
		if (d == upp) {
			d = ".";
			dlen = 1;
		} else
			dlen = (size_t)(upp - d);

		/*
		 * Complain if this path name for file would be too long.
		 * Otherwise, use this PATH element to build a possible
		 * path name for file.  Then, attempt to execve(2) it.
		 */
		if (dlen + flen + 1 >= sizeof(pnbuf)) {
			struct iovec msg[3];
			msg[0].iov_base = "pexec: ";
			msg[0].iov_len  = (size_t)7;
			msg[1].iov_base = (char *)d;
			msg[1].iov_len  = dlen;
			msg[2].iov_base = ": path too long\n";
			msg[2].iov_len  = (size_t)16;
			(void)writev(FD2, msg, 3);
			errno = ENAMETOOLONG;
			continue;
		}
		(void)memcpy(pnbuf, d, dlen);
		pnbuf[dlen] = SLASH;
		(void)memcpy(pnbuf + dlen + 1, file, flen);
		pnbuf[dlen + flen + 1] = EOS;

exec_pathname:
		(void)execve(pnp, argv, environ);
		switch (errno) {
		case EACCES:
#if defined(__OpenBSD__)
		case EISDIR:	/* Treat it as an EACCES error. */
#endif
			eacces = 1;
			/*FALLTHROUGH*/
		case ELOOP:
		case ENAMETOOLONG:
		case ENOENT:
		case ENOTDIR:
			break;
		case ENOEXEC:
			/*
			 * Get the user's EXECSHELL.
			 * Fail if it is unset or is set to an unusable value.
			 */
			esh = getenv("EXECSHELL");
			if (esh==NULL||*esh==EOS||strlen(esh)>=sizeof(pnbuf))
				goto fail;
			for (cnt = 0; argv[cnt] != NULL; cnt++)
				;	/* nothing */
			esh_argv = malloc((cnt + 2) * sizeof(char *));
			if (esh_argv == NULL)
				goto fail;
			esh_argv[0] = esh;
			esh_argv[1] = pnp;
			(void)memcpy(&esh_argv[2],&argv[1],cnt*sizeof(char *));
			(void)execve(esh, (char *const *)esh_argv, environ);
			free(esh_argv);
			errno = (errno == E2BIG) ? E2BIG : ENOEXEC;
			/*FALLTHROUGH*/
		default:
			goto fail;
		}
	} while (*upp++ == COLON);	/* Otherwise, *upp was NUL. */
	if (eacces != 0)
		errno = EACCES;

fail:
	if (errno == 0)
		errno = ENOENT;
	return -1;
}

extern const char *name;
extern bool no_lnum;

/*
 * Execute a file or path name w/ error handling.
 * This function never returns.
 */
void
err_pexec(const char *file, char *const *argv)
{
	long l;
	const char *f, *n;

	(void)pexec(file, argv);
#ifdef	DEBUG
	fd_print(FD2, "err_pexec: strerror(errno) == %s;\n", strerror(errno));
#endif

	f = (file == NULL) ? "(null)" : file;
	n = getmyname();

	if (EQUAL(n, "glob")) {
		if (errno == ENOEXEC)
			err(SH_ERR, FMT1S, ERR_NOSHELL);
		if (errno == E2BIG)
			err(SH_ERR, FMT1S, ERR_E2BIG);
		err(SH_ERR, FMT1S, ERR_GNOTFOUND);
	} else if (EQUAL(n, "sh6")) {
		if (errno == ENOEXEC)
			err(125, FMT1S, ERR_NOSHELL);
		if (errno == E2BIG)
			err(126, FMT2S, f, ERR_E2BIG);
		if (errno != ENOENT && errno != ENOTDIR)
			err(126, FMT2S, f, ERR_EXEC);
		err(127, FMT2S, f, ERR_NOTFOUND);
	} else {
		l = no_lnum ? -1 : get_lnum();
		if (name != NULL) {
			if (l != -1) {
				if (errno == ENOEXEC)
					err(125,FMT4LS,n,name,l,f, ERR_NOSHELL);
				if (errno == E2BIG)
					err(126,FMT4LS,n,name, l, f, ERR_E2BIG);
				if (errno != ENOENT && errno != ENOTDIR)
					err(126,FMT4LS,n, name, l, f, ERR_EXEC);
				err(127, FMT4LS, n, name, l, f, ERR_NOTFOUND);
			} else {
				if (errno == ENOEXEC)
					err(125,FMT4S, n, name, f, ERR_NOSHELL);
				if (errno == E2BIG)
					err(126, FMT4S, n, name, f, ERR_E2BIG);
				if (errno != ENOENT && errno != ENOTDIR)
					err(126, FMT4S, n, name, f, ERR_EXEC);
				err(127, FMT4S, n, name, f, ERR_NOTFOUND);
			}
		} else {
			if (l != -1) {
				if (errno == ENOEXEC)
					err(125, FMT3LFS, n, l, f, ERR_NOSHELL);
				if (errno == E2BIG)
					err(126, FMT3LFS, n, l, f, ERR_E2BIG);
				if (errno != ENOENT && errno != ENOTDIR)
					err(126, FMT3LFS, n, l, f, ERR_EXEC);
				err(127, FMT3LFS, n, l, f, ERR_NOTFOUND);
			} else {
				if (errno == ENOEXEC)
					err(125, FMT3S, n, f, ERR_NOSHELL);
				if (errno == E2BIG)
					err(126, FMT3S, n, f, ERR_E2BIG);
				if (errno != ENOENT && errno != ENOTDIR)
					err(126, FMT3S, n, f, ERR_EXEC);
				err(127, FMT3S, n, f, ERR_NOTFOUND);
			}
		}
	}
}
