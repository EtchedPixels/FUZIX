/*
 * goto.c - a port of the Sixth Edition (V6) UNIX transfer command
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
 *	@(#)$Id: a15db8d89eb988a587078882d69968fab7c015d8 $
 */
/*
 *	Derived from: Sixth Edition UNIX /usr/source/s1/goto.c
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

#ifndef	CONFIG_SUNOS
static	off_t	offset;
#endif

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
int
main(int argc, char **argv)
{
	size_t siz;
	char label[LABELMAX];

	setmyerrexit(&ut_errexit);
	setmyname(argv[0]);
	setmypid(getpid());

	if (argc < 2 || *argv[1] == EOS || isatty(FD0) != 0)
		err(FC_ERR, FMT2S, getmyname(), ERR_GENERIC);
	if ((siz = strlen(argv[1]) + 1) > sizeof(label))
		err(FC_ERR, FMT3S, getmyname(), argv[1], ERR_LABTOOLONG);
	if (lseek(FD0, (off_t)0, SEEK_SET) == -1)
		err(FC_ERR, FMT2S, getmyname(), ERR_SEEK);

	while (getlabel(label, *argv[1] & 0377, siz))
		if (strcmp(label, argv[1]) == 0) {
#ifndef	CONFIG_SUNOS
			(void)lseek(FD0, offset, SEEK_SET);
#endif
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
#ifndef	CONFIG_SUNOS
	int nc;

	offset++;
	nc = getchar();
	return (nc != EOF) ? nc & 0377 : EOF;
#else
	unsigned char nc;

	return (read(FD0, &nc, (size_t)1) == 1) ? nc : EOF;
#endif
}
