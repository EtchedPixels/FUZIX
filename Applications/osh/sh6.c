/*
 * sh6.c - a port of the Sixth Edition (V6) UNIX Thompson shell
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
 *	@(#)$Id: 7440110f83fb9f97939c121296d4028ee89b13ba $
 */
/*
 *	Derived from: Sixth Edition UNIX /usr/source/s2/sh.c
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

#define	SH6_SHELL

#include "defs.h"
#include "err.h"
#include "pexec.h"
#include "sasignal.h"
#include "sh.h"

/*
 * **** Global Variables ****
 */
const char	*name;		/* $0 - shell command name */
bool		no_lnum	= true;	/* no line number flag     */

/*
 * shell signal messages
 */
static	const char *const sigmsg[] = {
	" -- Core dumped",
	"Hangup",
	"",
	"Quit",
	"Illegal instruction",
	"Trace/BPT trap",
	"IOT trap",
	"EMT trap",
	"Floating exception",
	"Killed",
	"Bus error",
	"Memory fault",
	"Bad system call",
	""
};
#define	NSIGMSG		((int)(sizeof(sigmsg) / sizeof(sigmsg[0])))

static	char		apid[DOLMAX];	/* $$ - ASCII shell process ID      */
/*@null@*/
static	const char	*argv2p;	/* string for `-c' option           */
static	int		dolc;		/* $N dollar-argument count         */
/*@null@*/
static	const char	*dolp;		/* $N and $$ dollar-value pointer   */
static	char	*const	*dolv;		/* $N dollar-argument value array   */
/*@null@*/ /*@observer@*/
static	const char	*error_message;	/* error msg for read/parse errors  */
static	bool		glob_flag;	/* glob flag for `*', `?', `['      */
static	char		line[LINEMAX];	/* command-line buffer              */
static	char		*linep;		/* line pointer                     */
static	int		nul_count;	/* `\0'-character count (per line)  */
static	int		one_line_flag;	/* one-line flag for `-t' option    */
static	char		peekc;		/* just-read, pushed-back character */
/*@null@*/ /*@observer@*/
static	const char	*prompt;	/* interactive-shell prompt pointer */
static	enum sigflags	sig_child;	/* SIG(INT|QUIT|TERM) child flags   */
static	int		status;		/* shell exit status                */
static	int		tree_count;	/* talloc() call count (per line)   */
static	char		*word[WORDMAX];	/* argument/word pointer array      */
/*@null@*/
static	char		**wordp;	/* word pointer                     */

/*
 * **** Function Prototypes ****
 */
static	void		rpx_line(void);
static	void		get_word(void);
static	char		xgetc(bool);
static	char		readc(void);
/*@maynotreturn@*/ /*@null@*/
static	struct tnode	*talloc(void);
static	void		tfree(/*@null@*/ /*@only@*/ struct tnode *);
/*@null@*/
static	struct tnode	*syntax(char **, char **);
/*@null@*/
static	struct tnode	*syn1(char **, char **);
/*@null@*/
static	struct tnode	*syn2(char **, char **);
/*@null@*/
static	struct tnode	*syn3(char **, char **);
static	void		vscan(/*@null@*/ char **, int (*)(int));
static	void		ascan(/*@null@*/ char *, int (*)(int));
static	int		tglob(int);
static	int		trim(int);
static	bool		any(int, const char *);
static	void		execute(/*@null@*/ struct tnode *,
				/*@null@*/ int *, /*@null@*/ int *);
static	void		pwait(pid_t);
static	int		prsig(int, pid_t, pid_t);
/*@maynotreturn@*/
static	void		sh_errexit(int);
static	void		sh_init(/*@dependent@*/ /*@null@*/ /*@temp@*/
				const char *);
static	void		sh_magic(void);
static	void		fd_free(void);
static	bool		fd_isdir(int);
static	void		xfree(/*@null@*/ /*@only@*/ void *);
/*@maynotreturn@*/ /*@out@*/
static	void		*xmalloc(size_t);

/*
 * NAME
 *	sh6 - shell (command interpreter)
 *
 * SYNOPSIS
 *	sh6 [- | -c [string] | -t | file [arg1 ...]]
 *
 * DESCRIPTION
 *	See the sh6(1) manual page for full details.
 */
int
main(int argc, char **argv)
{
	bool dosigs = false;

	sh_init(argv[0]);
	if (argv[0] == NULL || *argv[0] == EOS)
		err(SH_ERR, FMT1S, ERR_ALINVAL);
	if (fd_isdir(FD0))
		goto done;

	if (argc > 1) {
		name = argv[1];
		dolv = &argv[1];
		dolc = argc - 1;
		if (*argv[1] == HYPHEN) {
			dosigs = true;
			if (argv[1][1] == 'c' && argc > 2) {
				dolv  += 1;
				dolc  -= 1;
				argv2p = argv[2];
			} else if (argv[1][1] == 't')
				one_line_flag = 2;
		} else {
			(void)close(FD0);
			if (open(argv[1], O_RDONLY) != FD0)
				err(SH_ERR, FMT2S, argv[1], ERR_OPEN);
			if (fd_isdir(FD0))
				goto done;
		}
	} else {
		dosigs = true;
		if (isatty(FD0) != 0 && isatty(FD2) != 0)
			prompt = (geteuid() != 0) ? "% " : "# ";
	}
	if (dosigs) {
		if (sasignal(SIGINT, SIG_IGN) == SIG_DFL)
			sig_child |= S_SIGINT;
		if (sasignal(SIGQUIT, SIG_IGN) == SIG_DFL)
			sig_child |= S_SIGQUIT;
		if (prompt != NULL)
			if (sasignal(SIGTERM, SIG_IGN) == SIG_DFL)
				sig_child |= S_SIGTERM;
	}
	fd_free();
	sh_magic();

loop:
	if (prompt != NULL)
		fd_print(FD2, "%s", prompt);
	rpx_line();
	goto loop;

done:
	return status;
}

/*
 * Read, parse, and execute a command line.
 */
static void
rpx_line(void)
{
	struct tnode *t;
	sigset_t nmask, omask;
	char *wp;

	linep = line;
	wordp = word;
	error_message = NULL;
	nul_count = 0;
	tree_count = 0;
	do {
		wp = linep;
		get_word();
	} while (*wp != EOL);
	*wordp = NULL;

	if (error_message != NULL) {
		err(SH_ERR, FMT1S, error_message);
		return;
	}

	if (wordp - word > 1) {
		(void)sigfillset(&nmask);
		(void)sigprocmask(SIG_SETMASK, &nmask, &omask);
		t = NULL;
		t = syntax(word, wordp);
		(void)sigprocmask(SIG_SETMASK, &omask, NULL);
		if (error_message != NULL)
			err(SH_ERR, FMT1S, error_message);
		else
			execute(t, NULL, NULL);
		tfree(t);
		t = NULL;
	}
}

/*
 * Copy a word from the standard input into the line buffer,
 * and point to it w/ *wordp.  Each copied word is represented
 * in line as an individual `\0'-terminated string.
 */
static void
get_word(void)
{
	char c, c1;

	*wordp++ = linep;

loop:
	switch (c = xgetc(DOLSUB)) {
	case SPACE:
	case TAB:
		goto loop;

	case DQUOT:/* "..." == multi-char literal */
	case SQUOT:/* '...' == ...                */
		c1 = c;
		while ((c = xgetc(!DOLSUB)) != c1) {
			if (c == EOL) {
				if (error_message == NULL)
					error_message = ERR_SYNTAX;
				peekc = c;
				*linep++ = EOS;
				return;
			}
			if (c == BQUOT) {
				if ((c = xgetc(!DOLSUB)) == EOL)
					c = SPACE;/* continue line */
				else {
					peekc = c;
					c = BQUOT;
				}
			}
			*linep++ = c | QUOTE;
		}
		break;

	case BQUOT:/* \. == one-char literal */
		if ((c = xgetc(!DOLSUB)) == EOL)
			goto loop;
		c |= QUOTE;
		peekc = c;
		break;

	case LPARENTHESIS: case RPARENTHESIS:
	case SEMICOLON:    case AMPERSAND:
	case VERTICALBAR:  case CARET:
	case LESSTHAN:     case GREATERTHAN:
	case EOL:
		*linep++ = c;
		*linep++ = EOS;
		return;

	default:
		peekc = c;
	}

	for (;;) {
		if ((c = xgetc(DOLSUB)) == BQUOT) {
			if ((c = xgetc(!DOLSUB)) == EOL)
				c = SPACE;/* continue line */
			else
				c |= QUOTE;
		}
		if (any(c, WORDPACK)) {
			peekc = c;
			if (any(c, QUOTPACK))
				goto loop;
			*linep++ = EOS;
			return;
		}
		*linep++ = c;
	}
	/*NOTREACHED*/
}

/*
 * If dolsub is true, get either the next literal character from the
 * standard input or substitute the current $ dollar w/ the next
 * character of its value, which is pointed to by dolp.  Otherwise,
 * get only the next literal character from the standard input.
 */
static char
xgetc(bool dolsub)
{
	int n;
	char c;

	if (peekc != EOS) {
		c = peekc;
		peekc = EOS;
		return c;
	}

	if (wordp >= &word[WORDMAX - 5]) {
		wordp -= 10;
		while (xgetc(!DOLSUB) != EOL)
			;	/* nothing */
		wordp += 10;
		error_message = ERR_TMARGS;
		goto geterr;
	}
	if (linep >= &line[LINEMAX - 5]) {
		linep -= 10;
		while (xgetc(!DOLSUB) != EOL)
			;	/* nothing */
		linep += 10;
		error_message = ERR_TMCHARS;
		goto geterr;
	}

getd:
	if (dolp != NULL) {
		c = *dolp++ & ASCII;
		if (c != EOS)
			return c;
		dolp = NULL;
	}
	c = readc();
	if (c == DOLLAR && dolsub) {
		c = readc();
		if (c >= '0' && c <= '9') {
			n = c - '0';
			if (IS_DIGIT(n, c) && n < dolc)
				dolp = (n > 0) ? dolv[n] : name;
			goto getd;
		}
		if (c == DOLLAR) {
			dolp = apid;
			goto getd;
		}
	}
	/* Ignore all EOS/NUL characters. */
	if (c == EOS) do {
		if (++nul_count >= LINEMAX) {
			error_message = ERR_TMCHARS;
			goto geterr;
		}
		c = readc();
	} while (c == EOS);
	return c;

geterr:
	return EOL;
}

/*
 * Read and return an ASCII-equivalent character from the string
 * pointed to by argv2p or from the standard input.  When reading
 * from argv2p, return the character or `\n'.  When reading from
 * the standard input, return the character.  Otherwise, exit w/
 * the value of the global variable status when appropriate.
 */
static char
readc(void)
{
	char c;

	if (argv2p != NULL) {
		if (argv2p == (char *)-1)
			exit(status);
		if ((c = (*argv2p++ & ASCII)) == EOS) {
			argv2p = (char *)-1;
			c = EOL;
		}
		return c;
	}
	if (one_line_flag == 1)
		exit(status);
	if (read(FD0, &c, (size_t)1) != 1)
		exit(status);
	if ((c &= ASCII) == EOL && one_line_flag == 2)
		one_line_flag = 1;

	return c;
}

/*
 * Allocate and initialize memory for a new tree node.
 * Return a pointer to the new node on success.
 * Return a pointer to NULL if >= TREEMAX.
 * Do not return on ENOMEM error.
 */
static struct tnode *
talloc(void)
{
	struct tnode *t;

	if (tree_count >= TREEMAX) {
		error_message = ERR_CLOVERFLOW;
		return NULL;
	}
	t = xmalloc(sizeof(struct tnode));
	t->nleft  = NULL;
	t->nright = NULL;
	t->nsub   = NULL;
	t->nfin   = NULL;
	t->nfout  = NULL;
	t->nav    = NULL;
	t->nflags = 0;
	t->ntype  = 0;
	tree_count++;
	return t;
}

/*
 * Deallocate memory for the shell command tree pointed to by t.
 */
static void
tfree(struct tnode *t)
{

	if (t == NULL)
		return;
	switch (t->ntype) {
	case TLIST:
	case TPIPE:
		tfree(t->nleft);
		tfree(t->nright);
		break;
	case TCOMMAND:
		xfree(t->nav);
		break;
	case TSUBSHELL:
		tfree(t->nsub);
		break;
	}
	xfree(t);
}

/*
 * syntax:
 *	empty
 *	syn1
 */
static struct tnode *
syntax(char **p1, char **p2)
{

	while (p1 < p2)
		if (any(**p1, EOC))
			p1++;
		else
			return syn1(p1, p2);
	return NULL;
}

/*
 * syn1:
 *	syn2
 *	syn2 ; syntax
 *	syn2 & syntax
 */
static struct tnode *
syn1(char **p1, char **p2)
{
	struct tnode *t;
	int c, subcnt;
	char **p;

	subcnt = 0;
	for (p = p1; p < p2; p++)
		switch (**p) {
		case LPARENTHESIS:
			subcnt++;
			continue;

		case RPARENTHESIS:
			subcnt--;
			if (subcnt < 0)
				goto syn1err;
			continue;

		case SEMICOLON:
		case AMPERSAND:
		case EOL:
			if (subcnt == 0) {
				c = **p;
				if ((t = talloc()) == NULL)
					goto syn1err;
				t->ntype  = TLIST;
				t->nleft  = syn2(p1, p);
				if (c == AMPERSAND && t->nleft != NULL)
					t->nleft->nflags |= FAND | FINTR | FPRS;
				t->nright = syntax(p + 1, p2);
				t->nflags = 0;
				return t;
			}
			break;
		}

	if (subcnt == 0)
		return syn2(p1, p2);

syn1err:
	if (error_message == NULL)
		error_message = ERR_SYNTAX;
	return NULL;
}

/*
 * syn2:
 *	syn3
 *	syn3 | syn2
 *	syn3 ^ syn2
 */
static struct tnode *
syn2(char **p1, char **p2)
{
	struct tnode *t;
	int subcnt;
	char **p;

	subcnt = 0;
	for (p = p1; p < p2; p++)
		switch (**p) {
		case LPARENTHESIS:
			subcnt++;
			continue;

		case RPARENTHESIS:
			subcnt--;
			continue;

		case VERTICALBAR:
		case CARET:
			if (subcnt == 0) {
				if ((t = talloc()) == NULL)
					return NULL;
				t->ntype  = TPIPE;
				t->nleft  = syn3(p1, p);
				t->nright = syn2(p + 1, p2);
				t->nflags = 0;
				return t;
			}
			break;
		}

	return syn3(p1, p2);
}

/*
 * syn3:
 *	command  [arg ...]  [< in]  [> [>] out]
 *	( syn1 )            [< in]  [> [>] out]
 */
static struct tnode *
syn3(char **p1, char **p2)
{
	struct tnode *t;
	enum tnflags flags;
	int ac, c, n, subcnt;
	char **p, **lp, **rp;
	char *fin, *fout;

	flags = 0;
	if (**p2 == RPARENTHESIS)
		flags |= FNOFORK;

	fin    = NULL;
	fout   = NULL;
	lp     = NULL;
	rp     = NULL;
	n      = 0;
	subcnt = 0;
	for (p = p1; p < p2; p++)
		switch (c = **p) {
		case LPARENTHESIS:
			if (subcnt == 0) {
				if (lp != NULL)
					goto syn3err;
				lp = p + 1;
			}
			subcnt++;
			continue;

		case RPARENTHESIS:
			subcnt--;
			if (subcnt == 0)
				rp = p;
			continue;

		case GREATERTHAN:
			p++;
			if (p < p2 && **p == GREATERTHAN)
				flags |= FCAT;
			else
				p--;
			/*FALLTHROUGH*/

		case LESSTHAN:
			if (subcnt == 0) {
				p++;
				if (p == p2 || any(**p, REDIRERR))
					goto syn3err;
				if (c == LESSTHAN) {
					if (fin != NULL)
						goto syn3err;
					fin = *p;
				} else {
					if (fout != NULL)
						goto syn3err;
					fout = *p;
				}
			}
			continue;

		default:
			if (subcnt == 0)
				p1[n++] = *p;
		}

	if (lp == NULL) {
		if (n == 0)
			goto syn3err;
		if ((t = talloc()) == NULL)
			goto syn3err;
		t->ntype = TCOMMAND;
		t->nav   = xmalloc((n + 1) * sizeof(char *));
		for (ac = 0; ac < n; ac++)
			t->nav[ac] = p1[ac];
		t->nav[ac] = NULL;
	} else {
		if (n != 0)
			goto syn3err;
		if ((t = talloc()) == NULL)
			goto syn3err;
		t->ntype = TSUBSHELL;
		t->nsub  = syn1(lp, rp);
	}
	t->nfin   = fin;
	t->nfout  = fout;
	t->nflags = flags;
	return t;

syn3err:
	if (error_message == NULL)
		error_message = ERR_SYNTAX;
	return NULL;
}

/*
 * Scan the entire argument vector pointed to by vp.
 * Each character of each argument is scanned w/ the function
 * pointed to by func.  Do nothing if vp is a NULL pointer.
 */
static void
vscan(char **vp, int (*func)(int))
{
	char *ap, c;

	if (vp == NULL)
		return;
	while ((ap = *vp++) != NULL)
		while ((c = *ap) != EOS)
			*ap++ = (*func)(c);
}

/*
 * Scan the argument pointed to by ap, a `\0'-terminated string.
 * Each character of the string is scanned w/ the function pointed
 * to by func.  Do nothing if ap is a NULL pointer.
 */
static void
ascan(char *ap, int (*func)(int))
{
	char c;

	if (ap == NULL)
		return;
	while ((c = *ap) != EOS)
		*ap++ = (*func)(c);
}

/*
 * Set the global variable glob_flag to true (1) if the character
 * c is a glob character.  Return the character in all cases.
 */
static int
tglob(int c)
{

	if (any(c, GLOBCHARS))
		glob_flag = true;
	return c;
}

/*
 * Return the character c converted back to its ASCII form.
 */
static int
trim(int c)
{

	return c & ASCII;
}

/*
 * Return true (1) if the character c matches any character in
 * the string pointed to by as.  Otherwise, return false (0).
 */
static bool
any(int c, const char *as)
{
	const char *s;

	s = as;
	while (*s != EOS)
		if (*s++ == c)
			return true;
	return false;
}

/*
 * Execute the shell command tree pointed to by t.
 */
static void
execute(struct tnode *t, int *pin, int *pout)
{
	struct tnode *t1;
	enum tnflags f;
	pid_t cpid;
	int i, pfd[2];
	const char **tav;
	const char *cmd, *p;

	if (t == NULL)
		return;
	switch (t->ntype) {
	case TLIST:
		f = t->nflags & (FFIN | FPIN | FINTR);
		if ((t1 = t->nleft) != NULL)
			t1->nflags |= f;
		execute(t1, NULL, NULL);
		if ((t1 = t->nright) != NULL)
			t1->nflags |= f;
		execute(t1, NULL, NULL);
		return;

	case TPIPE:
		if (pipe(pfd) == -1) {
			err(SH_ERR, FMT1S, ERR_PIPE);
			if (pin != NULL) {
				(void)close(pin[0]);
				(void)close(pin[1]);
			}
			return;
		}
		f = t->nflags;
		if ((t1 = t->nleft) != NULL)
			t1->nflags |= FPOUT | (f & (FFIN|FPIN|FINTR|FPRS));
		execute(t1, pin, pfd);
		if ((t1 = t->nright) != NULL)
			t1->nflags |= FPIN | (f & (FAND|FPOUT|FINTR|FPRS));
		execute(t1, pfd, pout);
		(void)close(pfd[0]);
		(void)close(pfd[1]);
		return;

	case TCOMMAND:
		if (t->nav == NULL || t->nav[0] == NULL) {
			/* should never (but can) be true */
			err(SH_ERR, FMT1S, "execute: Invalid command");
			return;
		}
		cmd = t->nav[0];
		if (EQUAL(cmd, ":")) {
			status = SH_TRUE;
			return;
		}
		if (EQUAL(cmd, "chdir")) {
			ascan(t->nav[1], &trim);
			if (t->nav[1] == NULL)
				err(SH_ERR, FMT2S, cmd, ERR_ARGCOUNT);
			else if (chdir(t->nav[1]) == -1)
				err(SH_ERR, FMT2S, cmd, ERR_BADDIR);
			else
				status = SH_TRUE;
			return;
		}
		if (EQUAL(cmd, "exit")) {
			if (prompt == NULL) {
				(void)lseek(FD0, (off_t)0, SEEK_END);
				EXIT(status);
			}
			return;
		}
		if (EQUAL(cmd, "login") || EQUAL(cmd, "newgrp")) {
			if (prompt != NULL) {
				p = (*cmd == 'l') ? PATH_LOGIN : PATH_NEWGRP;
				vscan(t->nav, &trim);
				(void)sasignal(SIGINT, SIG_DFL);
				(void)sasignal(SIGQUIT, SIG_DFL);
				(void)pexec(p, (char *const *)t->nav);
				(void)sasignal(SIGINT, SIG_IGN);
				(void)sasignal(SIGQUIT, SIG_IGN);
			}
			err(SH_ERR, FMT2S, cmd, ERR_EXEC);
			return;
		}
		if (EQUAL(cmd, "shift")) {
			if (dolc > 1) {
				dolv = &dolv[1];
				dolc--;
				status = SH_TRUE;
				return;
			}
			err(SH_ERR, FMT2S, cmd, ERR_NOARGS);
			return;
		}
		if (EQUAL(cmd, "wait")) {
			pwait(-1);
			return;
		}
		/*FALLTHROUGH*/

	case TSUBSHELL:
		f = t->nflags;
		if ((cpid = ((f & FNOFORK) != 0) ? 0 : fork()) == -1) {
			err(SH_ERR, FMT1S, ERR_FORK);
			return;
		}
		/**** Parent! ****/
		if (cpid != 0) {
			if (pin != NULL && (f & FPIN) != 0) {
				(void)close(pin[0]);
				(void)close(pin[1]);
			}
			if ((f & FPRS) != 0)
				fd_print(FD2, "%u\n", (unsigned)cpid);
			if ((f & FAND) != 0)
				return;
			if ((f & FPOUT) == 0)
				pwait(cpid);
			return;
		}
		/**** Child! ****/
		/*
		 * Redirect (read) input from pipe.
		 */
		if (pin != NULL && (f & FPIN) != 0) {
			if (dup2(pin[0], FD0) == -1)
				err(FC_ERR, FMT1S, strerror(errno));
			(void)close(pin[0]);
			(void)close(pin[1]);
		}
		/*
		 * Redirect (write) output to pipe.
		 */
		if (pout != NULL && (f & FPOUT) != 0) {
			if (dup2(pout[1], FD1) == -1)
				err(FC_ERR, FMT1S, strerror(errno));
			(void)close(pout[0]);
			(void)close(pout[1]);
		}
		/*
		 * Redirect (read) input from file.
		 */
		if (t->nfin != NULL && (f & FPIN) == 0) {
			f |= FFIN;
			ascan(t->nfin, &trim);
			if ((i = open(t->nfin, O_RDONLY)) == -1)
				err(FC_ERR, FMT2S, t->nfin, ERR_OPEN);
			if (dup2(i, FD0) == -1)
				err(FC_ERR, FMT1S, strerror(errno));
			(void)close(i);
		}
		/*
		 * Redirect (write) output to file.
		 */
		if (t->nfout != NULL && (f & FPOUT) == 0) {
			if ((f & FCAT) != 0)
				i = O_WRONLY | O_APPEND | O_CREAT;
			else
				i = O_WRONLY | O_TRUNC | O_CREAT;
			ascan(t->nfout, &trim);
			if ((i = open(t->nfout, i, 0666)) == -1)
				err(FC_ERR, FMT2S, t->nfout, ERR_CREATE);
			if (dup2(i, FD1) == -1)
				err(FC_ERR, FMT1S, strerror(errno));
			(void)close(i);
		}
		/*
		 * Set the action for the SIGINT and SIGQUIT signals, and
		 * redirect input for `&' commands from `/dev/null' if needed.
		 */
		if ((f & FINTR) != 0) {
			(void)sasignal(SIGINT, SIG_IGN);
			(void)sasignal(SIGQUIT, SIG_IGN);
			if (t->nfin == NULL && (f & (FFIN|FPIN|FPRS)) == FPRS) {
				(void)close(FD0);
				if (open("/dev/null", O_RDONLY) != FD0)
					err(FC_ERR,FMT2S,"/dev/null",ERR_OPEN);
			}
		} else {
			if ((sig_child & S_SIGINT) != 0)
				(void)sasignal(SIGINT, SIG_DFL);
			if ((sig_child & S_SIGQUIT) != 0)
				(void)sasignal(SIGQUIT, SIG_DFL);
		}
		/* Set the SIGTERM signal to its default action if needed. */
		if ((sig_child & S_SIGTERM) != 0)
			(void)sasignal(SIGTERM, SIG_DFL);
		if (t->ntype == TSUBSHELL) {
			if ((t1 = t->nsub) != NULL)
				t1->nflags |= (f & (FFIN | FPIN | FINTR));
			execute(t1, NULL, NULL);
			_exit(status);
		}
		if (t->nav == NULL || t->nav[0] == NULL)
			err(FC_ERR, FMT1S, "execute: Invalid command");
		cmd = t->nav[0];
		glob_flag = false;
		vscan(t->nav, &tglob);
		if (glob_flag) {
			for (i = 0; t->nav[i] != NULL; i++)
				;	/* nothing */
			tav = xmalloc((i + 2) * sizeof(char *));
			tav[0] = GLOB_PATH;
			cmd = tav[0];
			(void)memcpy(&tav[1], t->nav, (i + 1) * sizeof(char *));
#ifdef	DEBUG
#ifdef	DEBUG_GLOB
			fd_print(FD2, "execute: i == %d;\n", i);
			for (i = 0; tav[i] != NULL; i++)
				fd_print(FD2, "tav[%d]==%p==%p==%s;\n",
				    i, &tav[i], tav[i], tav[i]);
			fd_print(FD2, "tav[%d]==%p==NULL;\n", i, &tav[i]);
#endif
#endif
			(void)err_pexec(cmd, (char *const *)tav);
		} else if (IS_LIBEXEC(EQUAL)) {
			vscan(t->nav, &trim);
			for (i = 0; t->nav[i] != NULL; i++)
				;	/* nothing */
			tav = xmalloc((i + 1) * sizeof(char *));
			if (EQUAL(cmd, "fd2"))
				tav[0] = FD2_PATH;
			else if (EQUAL(cmd, "goto"))
				tav[0] = GOTO_PATH;
			else
				tav[0] = IF_PATH;
			cmd = tav[0];
			(void)memcpy(&tav[1], &t->nav[1], i * sizeof(char *));
#ifdef	DEBUG
#ifdef	DEBUG_LED
			fd_print(FD2, "execute: i == %d;\n", i);
			for (i = 0; tav[i] != NULL; i++)
				fd_print(FD2, "tav[%d]==%p==%p==%s;\n",
				    i, &tav[i], tav[i], tav[i]);
			fd_print(FD2, "tav[%d]==%p==NULL;\n", i, &tav[i]);
#endif
#endif
			(void)err_pexec(cmd, (char *const *)tav);
		} else {
			vscan(t->nav, &trim);
			cmd = t->nav[0];
			(void)err_pexec(cmd, (char *const *)t->nav);
		}
		/*NOTREACHED*/
	}
}

/*
 * If cp > 0, wait for the child process cp to terminate.
 * While doing so, exorcise any zombies for which the shell has not
 * yet waited, and wait for any other child processes which terminate
 * during this time.  Otherwise, if cp < 0, block and wait for all
 * child processes to terminate.
 */
static void
pwait(pid_t cp)
{
	pid_t tp;
	int s;

	if (cp == 0)
		return;
	for (;;) {
		tp = wait(&s);
		if (tp == -1)
			break;
		if (s != 0) {
			if (WIFSIGNALED(s) != 0)
				status = prsig(s, tp, cp);
			else if (WIFEXITED(s) != 0)
				status = WEXITSTATUS(s);
			if (status >= FC_ERR) {
				if (status == FC_ERR)
					status = SH_ERR;
				err(status, NULL);
			}
		} else
			status = SH_TRUE;
		if (tp == cp)
			break;
	}
}

/*
 * Print termination report for process tp according to signal s.
 * Return a value of 128 + signal s (e) for exit status of tp.
 */
static int
prsig(int s, pid_t tp, pid_t cp)
{
	int e, r;
	char buf[SBUFMM(1)];
	const char *c, *m;

	e = WTERMSIG(s);
	if (e >= NSIGMSG || (e >= 0 && *sigmsg[e] != EOS)) {
		if (e < NSIGMSG)
			m = sigmsg[e];
		else {
			r = snprintf(buf, sizeof(buf), "Sig %u", (unsigned)e);
			m = (r < 0 || r >= (int)sizeof(buf))  ?
			    "prsig: snprintf: Internal error" :
			    buf;
		}
		c = "";
#ifdef	WCOREDUMP
		if (WCOREDUMP(s) != 0)
			c = sigmsg[0];
#endif
		if (tp != cp)
			fd_print(FD2, "%u: %s%s\n", (unsigned)tp, m, c);
		else
			fd_print(FD2, "%s%s\n", m, c);
	} else
		fd_print(FD2, FMT1S, "");

	return 128 + e;
}

/*
 * Handle all error exit scenarios for the shell.  This includes
 * setting the exit status to the appropriate value according to
 * es and causing the shell to exit if appropriate.  This function
 * is called by err() and may or may not return.
 */
static void
sh_errexit(int es)
{

#ifdef	DEBUG
	fd_print(FD2,"sh_errexit: getmypid() == %d, es == %d;\n",getmypid(),es);
#endif

	status = es;
	if (prompt == NULL) {
		(void)lseek(FD0, (off_t)0, SEEK_END);
		EXIT(status);
	}
	if (getpid() != getmypid())
		_exit(status);
}

/*
 * Initialize the shell.
 */
static void
sh_init(const char *av0p)
{
	struct stat sb;
	int i;

	setmyerrexit(&sh_errexit);
	setmyname(av0p);
	setmypid(getpid());

	/*
	 * Set-ID execution is not supported.
	 */
	if (geteuid() != getuid() || getegid() != getgid())
		err(SH_ERR, FMT1S, ERR_SETID);

	/*
	 * Save the process ID of the shell as a 5-digit ASCII
	 * string (apid).  Each occurrence of `$$' in a command
	 * line is substituted w/ the value of apid.
	 */
	i = snprintf(apid, sizeof(apid), "%05u", (unsigned)getmypid());
	if (i < 0 || i >= (int)sizeof(apid))
		*apid = EOS;

	/*
	 * Fail if any of the descriptors 0, 1, or 2 is not open.
	 */
	for (i = 0; i < 3; i++)
		if (fstat(i, &sb) == -1)
			err(SH_ERR, "%u: %s\n", (unsigned)i, strerror(errno));

	/*
	 * Set the SIGCHLD signal to its default action.
	 * Correct operation of the shell requires that zombies
	 * be created for its children when they terminate.
	 */
	(void)sasignal(SIGCHLD, SIG_DFL);
}

/*
 * Ignore any `#!shell' sequence as the first line of a regular file.
 * The length of this line is limited to (LINEMAX - 1) characters.
 */
static void
sh_magic(void)
{
	struct stat sb;
	size_t len;

	if (fstat(FD0, &sb) == -1 || !S_ISREG(sb.st_mode))
		return;
	if (lseek(FD0, (off_t)0, SEEK_CUR) == 0) {
		if (readc() == HASH && readc() == BANG) {
			for (len = 2; len < LINEMAX; len++)
				if (readc() == EOL)
					return;
			err(SH_ERR, FMT1S, ERR_TMCHARS);
		}
		(void)lseek(FD0, (off_t)0, SEEK_SET);
	}
}

/*
 * Attempt to free or release all of the file descriptors in the
 * range from (fd_max - 1) through (FD2 + 1); the value of fd_max
 * may fall between FDFREEMIN and FDFREEMAX, inclusive.
 */
static void
fd_free(void)
{
	long fd_max;
	int fd;

	fd_max = sysconf(_SC_OPEN_MAX);
	if (fd_max < FDFREEMIN || fd_max > FDFREEMAX)
		fd_max = FDFREEMIN;
	for (fd = (int)fd_max - 1; fd > FD2; fd--)
		(void)close(fd);
}

/*
 * Return true (1) if the file descriptor fd refers to an open file
 * which is a directory.  Otherwise, return false (0).
 */
static bool
fd_isdir(int fd)
{
	struct stat sb;

	return fstat(fd, &sb) == 0 && S_ISDIR(sb.st_mode);
}

/*
 * Deallocate the memory allocation pointed to by p.
 */
static void
xfree(void *p)
{

	if (p != NULL) {
		free(p);
		p = NULL;
	}
}

/*
 * Allocate memory, and check for error.
 * Return a pointer to the allocated space on success.
 * Do not return on ENOMEM error.
 */
static void *
xmalloc(size_t s)
{
	void *mp;

	if ((mp = malloc(s)) == NULL) {
		err(ESTATUS, FMT1S, ERR_NOMEM);
		/*NOTREACHED*/
	}
	return mp;
}
