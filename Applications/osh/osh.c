/*
 * osh.c - an enhanced port of the Sixth Edition (V6) UNIX Thompson shell
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
 *	@(#)$Id: 6e2bdd5f895ca9e608f47f343453e6f368dfbda2 $
 */
/*
 *	Derived from: osh-20061230
 *			osh.c	(1.1 (jneitzel) 2006/12/26)
 *			sh6.c	(1.3 (jneitzel) 2006/09/15)
 *			glob6.c	(1.3 (jneitzel) 2006/09/23)
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
#include "sasignal.h"
#include "sh.h"
#include "strtoint.h"

#ifdef	__GNUC__
#define	IS_UNUSED	__attribute__((__unused__))
#else
#define	IS_UNUSED	/* nothing */
#endif

/*
 * The following file descriptors are reserved for special use by osh.
 */
#define	DUPFD0		10	/* used for input redirection w/ `<-' */
#define	HWFD		11	/* used for history write fildes      */
#define	PWD		12	/* used in do_chdir()                 */
#define	SAVFD0		13	/* used in do_source()                */

/*
 * These are the rc (init and logout) files used by osh.
 * The `FILE_DOT_*' files are in user's HOME directory.
 */
#define	DO_SYSTEM_LOGIN		1
#define	PATH_SYSTEM_LOGIN	SYSCONFDIR/**/"/osh.login"
#define	DO_SYSTEM_OSHRC		2
#define	PATH_SYSTEM_OSHRC	SYSCONFDIR/**/"/osh.oshrc"
#define	DO_DOT_LOGIN		3
#define	FILE_DOT_LOGIN		".osh.login"
#define	DO_DOT_OSHRC		4
#define	FILE_DOT_OSHRC		".oshrc"
#define	DO_INIT_DONE		5
#define	DO_SYSTEM_LOGOUT	1
#define	PATH_SYSTEM_LOGOUT	SYSCONFDIR/**/"/osh.logout"
#define	DO_DOT_LOGOUT		2
#define	FILE_DOT_LOGOUT		".osh.logout"
#define	DO_LOGOUT_DONE		3

/*
 * This is the history file (in user's HOME directory) used by osh.
 */
#define	FILE_DOT_HISTORY	".osh.history"

/*
 * These are the symbolic names for the types checked by fd_type().
 */
#define	FD_TMASK	S_IFMT	/* file descriptor (FD) type mask             */
#define	FD_ISOPEN	01	/* Does FD refer to an open file?             */
#define	FD_ISDIROK	02	/* Does FD refer to an existent directory?    */
#define	FD_ISREGOK	04	/* Does FD refer to an existent regular file? */
#define	FD_ISDIR	S_IFDIR	/* Does FD refer to a  directory?             */
#define	FD_ISREG	S_IFREG	/* Does FD refer to a  regular file?          */

/*
 * (NSIG - 1) is the maximum signal number value accepted by `trap'.
 */
#ifndef	NSIG
#define	NSIG		32
#endif

#define	HALT		true
#define	PROMPT		((shtype & ST_MASK) == ST_INTERACTIVE)
#define	SHTYPE(f)	((shtype & (f)) != 0)
#define	DO_TRIM(k)	((k) != SBI_CD && (k) != SBI_CHDIR)

/*
 * shell type flags
 */
enum stflags {
	ST_ONELINE     = 001,
	ST_COMMANDFILE = 002,
	ST_INTERACTIVE = 004,
	ST_RCFILE      = 010,
	ST_SOURCE      = 020,
	ST_MASK        = 037
};

/*
 * **** Global Variables ****
 */
const char	*name;	/* $0 - shell command name */
bool		no_lnum;/* no line number flag     */
uid_t		sheuid;	/* effective shell user ID */

/*
 * shell sbi command structure array
 */
static	const struct sbicmd {
	const char *sbi_command;
	const enum sbikey sbi_key;
} sbi[] = {
	{ ".",		SBI_DOT      },
	{ ":",		SBI_NULL     },
	{ "alias",	SBI_ALIAS    },
	{ "cd",		SBI_CD       },
	{ "chdir",	SBI_CHDIR    },
	{ "echo",	SBI_ECHO     },
	{ "exec",	SBI_EXEC     },
	{ "exit",	SBI_EXIT     },
	{ "fd2",	SBI_FD2      },
	{ "goto",	SBI_GOTO     },
	{ "if",		SBI_IF       },
	{ "help",	SBI_HELP     },
	{ "login",	SBI_LOGIN    },
	{ "newgrp",	SBI_NEWGRP   },
	{ "set",	SBI_SET      },
	{ "setenv",	SBI_SETENV   },
	{ "shift",	SBI_SHIFT    },
	{ "source",	SBI_SOURCE   },
	{ "trap",	SBI_TRAP     },
	{ "umask",	SBI_UMASK    },
	{ "unalias",	SBI_UNALIAS  },
	{ "unset",	SBI_UNSET    },
	{ "unsetenv",	SBI_UNSETENV },
	{ "version",	SBI_VERSION  },
	{ "wait",	SBI_WAIT     }
};
#define	NSBICMD		((int)(sizeof(sbi) / sizeof(sbi[0])))

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
	"Broken pipe"
};
#define	NSIGMSG		((int)(sizeof(sigmsg) / sizeof(sigmsg[0])))

/*@null@*/
static	const char	*argv2p;	/* string for `-c' option           */
static	int		dolac;		/* $* dollar-argument count         */
static	char		dolbuf[DOLMAX];	/* dollar buffer for $$, $#, $?, $v */
static	int		dolc;		/* $N dollar-argument count         */
/*@null@*/ /*@only@*/
static	const char	*dolp;		/* $ dollar-value pointer           */
static	char	*const	*dolv;		/* $N dollar-argument value array   */
static	int		dupfd0;		/* duplicate of the standard input  */
/*@null@*/ /*@observer@*/
static	const char	*error_message;	/* error msg for read/parse errors  */
static	bool		error_source;	/* error flag for `source' command  */
static	int		hwfd = -1;	/* history write file descriptor    */
static	bool		is_first;	/* first line flag                  */
static	bool		is_login;	/* login shell flag                 */
static	bool		is_noexec;	/* not executable file flag         */
static	bool		is_verbose;	/* verbose flag for `-v' option     */
static	char		line[LINEMAX];	/* command-line buffer              */
static	char		aline[LINEMAX];	/* alias-line buffer                */
static	char		*linep;		/* [a]line pointer                  */
static	char		*elinep;	/* end of [a]line pointer           */
static	volatile sig_atomic_t
			logout_now;	/* SIGHUP caught flag (1 if caught) */
static	int		nul_count;	/* `\0'-character count (per line)  */
static	int		peekc;		/* just-read, pushed-back character */
/*@null@*/ /*@only@*/
static	char		*rcfilename;	/* rc file name for diagnostics     */
static	enum stflags	shtype;		/* shell type (determines behavior) */
static	enum sigflags	sig_child;	/* SIG(INT|QUIT|TERM) child flags   */
static	enum sigflags	sig_state;	/* SIG(INT|QUIT|TERM) state flags   */
static	int		status;		/* shell exit status                */
static	int		tree_count;	/* talloc() call count (per line)   */
static	char		*word[WORDMAX];	/* arg/word pointer array           */
static	char		*aword[WORDMAX];/* alias arg/word pointer array     */
/*@null@*/
static	char		**wordp;	/* [a]word pointer                  */
static	char		**ewordp;	/* end of [a]word pointer           */
/*@null@*/ /*@only@*/
static	struct anode	*anp;		/* shell alias node pointer         */
/*@null@*/
static	const char	*asp;		/* shell alias string pointer       */
/*@only@*/
static	struct tnode	*tnp;		/* shell command tree node pointer  */
/*@null@*/ /*@only@*/
static	struct vnode	*vnp;		/* shell variable node pointer      */
/*@null@*/ /*@only@*/
static	char		*tty;		/* $t - terminal name               */
/*@null@*/ /*@only@*/
static	char		*user;		/* $u - effective user name         */

/*
 * **** Function Prototypes ****
 */
static	void		cmd_loop(bool);
static	void		cmd_verbose(void);
static	void		error(int, /*@null@*/ const char *);
static	void		error1(int, /*@null@*/ const char *, /*@null@*/ const char *);
static	void		error2(int, /*@null@*/ const char *, /*@null@*/ const char *, /*@null@*/ const char *);
static	int		rpx_line(void);
/*@null@*/
static	const char	**rp_alias(const char *);
static	int		get_word(void);
static	int		xgetc(bool);
static	int		readc(void);
/*@null@*/ /*@only@*/
static	const char	*get_dolp(int);
static	void		varalloc(int, /*@null@*/ const char *);
/*@maynotreturn@*/
static	struct vnode	*varalloc1(int, const char *);
static	bool		varfree(int);
/*@null@*/
static	const char	*varget(int);
static	void		aalloc(/*@null@*/ const char *, /*@null@*/ const char *);
/*@maynotreturn@*/
static	struct anode	*aalloc1(const char *, const char *);
static	bool		afree(/*@null@*/ const char *);
/*@null@*/
static	const char	*asget(/*@null@*/ const char *);
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
static	bool		any(int, const char *);
static	bool		vtglob(char **);
static	void		vtrim(char **);
static	int		vacount(const char **);
static	void		execute(/*@null@*/ struct tnode *,
				/*@null@*/ int *, /*@null@*/ int *);
static	void		execute1(struct tnode *);
static	void		execute2(struct tnode *,
				 /*@null@*/ int *, /*@null@*/ int *);
static	void		do_chdir(char **);
static	void		do_trap(char **, enum tnflags);
static	void		set_ss_flags(long, action_type);
static	void		do_source(char **);
static	int		source_open(const char *, const char *);
static	void		pwait(pid_t);
static	int		prsig(int, pid_t, pid_t);
/*@maynotreturn@*/
static	void		sh_errexit(int);
static	void		sh_init(/*@dependent@*/ /*@null@*/ /*@temp@*/
				const char *);
static	void		sh_magic(void);
static	bool		sh_on_tty(void);
static	void		sighup(/*@unused@*/ int IS_UNUSED);
static	void		rc_init(int *);
static	void		rc_logout(int *);
/*@maynotreturn@*/
static	bool		rc_open(/*@null@*/ const char *);
static	char		*pn_build(/*@out@*/ /*@returned@*/ char *,
				  const char *, size_t);
static	void		hist_write(bool);
static	void		hist_open(void);
static	void		fd_free(void);
static	bool		fd_type(int, mode_t);
/*@maynotreturn@*/ /*@null@*/
static	char		*atrim(/*@returned@*/ UChar *);
/*@maynotreturn@*/ /*@null@*/
static	char		*gtrim(/*@returned@*/ UChar *);
/*@null@*/
static	char		*gchar(/*@returned@*/ const char *);
static	void		vfree(/*@null@*/ char **);
static	void		xfree(/*@null@*/ /*@only@*/ void *);
/*@maynotreturn@*/ /*@out@*/
static	void		*xmalloc(size_t);
/*@maynotreturn@*/
static	void		*xrealloc(/*@only@*/ void *, size_t);
/*@maynotreturn@*/
static	char		*xstrdup(const char *);
/*@maynotreturn@*/ /*@null@*/ /*@only@*/
static	const char	**glob(enum sbikey, /*@only@*/ char **);

/*
 * NAME
 *	osh - old shell (command interpreter)
 *
 * SYNOPSIS
 *	osh [-v] [- | -c [string] | -i | -l | -t | file [arg1 ...]]
 *
 * DESCRIPTION
 *	See the osh(1) manual page for full details.
 */
int
main(int argc, char **argv)
{
	char *av0p, *snamep;
	int rcflag = 0;
	bool dosigs = false;

	sh_init(argv[0]);
	if (argv[0] == NULL || *argv[0] == EOS)
		err(SH_ERR, FMT2S, getmyname(), ERR_ALINVAL);
	if (fd_type(FD0, FD_ISDIR))
		goto done;

	if (argc > 1 && *argv[1] == HYPHEN && argv[1][1] == 'v') {
		is_verbose = true;
		av0p = argv[0], argv = &argv[1], argv[0] = av0p;
		argc--;
	}
	if (argc > 1) {
		name = argv[1];
		dolv = &argv[1];
		dolc = argc - 1;
		if (*argv[1] == HYPHEN) {
			dosigs = true;
			if (argv[1][1] == 'c' && argc > 2) {
				shtype  = ST_ONELINE;
				no_lnum = true;
				dolv   += 1;
				dolc   -= 1;
				argv2p  = argv[2];
			} else if (argv[1][1] == 'i') {
				rcflag = DO_SYSTEM_OSHRC;
				shtype = ST_INTERACTIVE;
				if (!sh_on_tty())
					err(SH_ERR, FMT3S,
					    getmyname(), argv[1], ERR_NOTTY);
			} else if (argv[1][1] == 'l') {
				is_login = true;
				rcflag   = DO_SYSTEM_LOGIN;
				shtype   = ST_INTERACTIVE;
				if (!sh_on_tty())
					err(SH_ERR, FMT3S,
					    getmyname(), argv[1], ERR_NOTTY);
			} else if (argv[1][1] == 't') {
				shtype  = ST_ONELINE;
				no_lnum = true;
			}
		} else {
			shtype = ST_COMMANDFILE;
			(void)close(FD0);
			if (open(argv[1], O_RDONLY) != FD0)
				err(SH_ERR,FMT3S,getmyname(),argv[1],ERR_OPEN);
			if (fd_type(FD0, FD_ISDIR))
				goto done;
		}
		fd_free();
	} else {
		dosigs = true;
		fd_free();
		if (sh_on_tty())
			shtype = ST_INTERACTIVE;
	}
	if (dosigs) {
		if (sasignal(SIGINT, SIG_IGN) == SIG_DFL)
			sig_child |= S_SIGINT;
		if (sasignal(SIGQUIT, SIG_IGN) == SIG_DFL)
			sig_child |= S_SIGQUIT;
		if (PROMPT) {
			if (sasignal(SIGTERM, SIG_IGN) == SIG_DFL)
				sig_child |= S_SIGTERM;
			if (rcflag == 0) {
				if (*argv[0] == HYPHEN) {
					is_login = true;
					rcflag   = DO_SYSTEM_LOGIN;
				} else
					rcflag   = DO_SYSTEM_OSHRC;
			}
			if (is_login)
				if (sasignal(SIGHUP, sighup) == SIG_IGN)
					(void)sasignal(SIGHUP, SIG_IGN);
			rc_init(&rcflag);
			hist_open();
		}
	}

	if (SHTYPE(ST_ONELINE))
		(void)rpx_line();
	else {
		/* Read and execute any rc init files if needed. */
		while (SHTYPE(ST_RCFILE)) {
			snamep = (char *)name;
			name   = rcfilename;
			cmd_loop(!HALT);
			xfree(rcfilename);
			rcfilename = NULL;
			name = NULL;
			name = snamep;
			if (logout_now != 0) {
				logout_now = 0;
				if (is_login)
					goto logout;
				goto done;
			}
			rc_init(&rcflag);
		}

		/* Read and execute the shell's input. */
		cmd_loop(!HALT);
		if (logout_now != 0)
			logout_now = 0;

logout:
		/* Read and execute any rc logout files if needed. */
		rcflag = DO_SYSTEM_LOGOUT;
		rc_logout(&rcflag);
		while (SHTYPE(ST_RCFILE)) {
			snamep = (char *)name;
			name   = rcfilename;
			cmd_loop(!HALT);
			xfree(rcfilename);
			rcfilename = NULL;
			name = NULL;
			name = snamep;
			if (logout_now != 0)
				logout_now = 0;
			rc_logout(&rcflag);
		}
	}

done:
	xfree(tty);
	tty = NULL;
	xfree(user);
	user = NULL;
	(void)varfree(0);
	(void)afree(NULL);
	return status;
}

/*
 * Determine whether or not the string pointed to by cmd
 * is a special built-in command.  Return the key value.
 */
enum sbikey
cmd_lookup(const char *cmd)
{
	const struct sbicmd *lp, *mp, *rp;
	int d;

	for (lp = sbi, rp = &sbi[NSBICMD]; lp < rp; /* nothing */) {
		mp = lp + (rp - lp) / 2;
		if ((d = strcmp(cmd, mp->sbi_command)) == 0)
			return mp->sbi_key;
		if (d > 0)
			lp = mp + 1;
		else
			rp = mp;
	}
	return SBI_UNKNOWN;
}

/*
 * Read and execute command lines until EOF, or until one of
 * error_source or logout_now != 0 is true according to halt
 * or is_login.
 */
static void
cmd_loop(bool halt)
{
	bool gz;

	sh_magic();

	for (error_source = gz = false; ; ) {
		if (PROMPT)
			fd_print(FD2, "%s", (sheuid != 0) ? "% " : "# ");
		if (rpx_line() == EOF) {
			if (!gz)
				status = SH_TRUE;
			break;
		}
		if (halt && error_source)
			break;
		if (is_login && logout_now != 0)
			break;
		gz = true;
	}
}

/*
 * If is_verbose is true, print each argument/word
 * in the word pointer array to the standard error.
 * Otherwise, do nothing.
 */
static void
cmd_verbose(void)
{
	char **vp;

	if (!is_verbose)
		return;
	for (vp = word; **vp != EOL; vp++)
		fd_print(FD2, "%s%s", *vp, (**(vp + 1) != EOL) ? " " : "");
	fd_print(FD2, FMT1S, "");
}

/*
 * Print diagnostic w/ $0, line number, and so forth if possible.
 */
static void
error(int s, const char *m)
{
	long ln;

	if (m == NULL) {
		err(ESTATUS, FMT3S, getmyname(), "error", strerror(EINVAL));
		/*NOTREACHED*/
	}

	ln = (is_noexec || no_lnum) ? -1 : get_lnum();
	if (name != NULL) {
		if (ln != -1)
			err(s, FMT3LS, getmyname(), name, ln, m);
		else
			err(s, FMT3S, getmyname(), name, m);
	} else {
		if (ln != -1)
			err(s, FMT2LS, getmyname(), ln, m);
		else
			err(s, FMT2S, getmyname(), m);
	}
}

/*
 * Print diagnostic w/ $0, line number, and so forth if possible.
 */
static void
error1(int s, const char *f, const char *m)
{
	long ln;

	if (f == NULL || m == NULL) {
		err(ESTATUS, FMT3S, getmyname(), "error1", strerror(EINVAL));
		/*NOTREACHED*/
	}

	ln = (is_noexec || no_lnum) ? -1 : get_lnum();
	if (name != NULL) {
		if (ln != -1)
			err(s, FMT4LS, getmyname(), name, ln, f, m);
		else
			err(s, FMT4S, getmyname(), name, f, m);
	} else {
		if (ln != -1)
			err(s, FMT3LFS, getmyname(), ln, f, m);
		else
			err(s, FMT3S, getmyname(), f, m);
	}
}

/*
 * Print diagnostic w/ $0, line number, and so forth if possible.
 */
static void
error2(int s, const char *c, const char *a, const char *m)
{
	long ln;

	if (c == NULL || a == NULL || m == NULL) {
		err(ESTATUS, FMT3S, getmyname(), "error2", strerror(EINVAL));
		/*NOTREACHED*/
	}

	ln = (is_noexec || no_lnum) ? -1 : get_lnum();
	if (name != NULL) {
		if (ln != -1)
			err(s, FMT5LS, getmyname(), name, ln, c, a, m);
		else
			err(s, FMT5S, getmyname(), name, c, a, m);
	} else {
		if (ln != -1)
			err(s, FMT4LFS, getmyname(), ln, c, a, m);
		else
			err(s, FMT4S, getmyname(), c, a, m);
	}
}

/*
 * Read, parse, and execute a command line.
 */
static int
rpx_line(void)
{
	struct tnode *t;
	sigset_t nmask, omask;
	char *wp;

	linep  = line;
	elinep = &line[LINEMAX - 5];
	wordp  = word;
	ewordp = &word[WORDMAX - 5];
	error_message = NULL;
	nul_count = 0;
	tree_count = 0;
	do {
		wp = linep;
		if (get_word() == EOF)
			return EOF;
	} while (*wp != EOL);
	*wordp = NULL;

	cmd_verbose();
	hist_write(PROMPT && wordp - word > 1);

	if (error_message != NULL) {
		error(-1, error_message);
		return 1;
	}

	if (wordp - word > 1) {
		(void)sigfillset(&nmask);
		(void)sigprocmask(SIG_SETMASK, &nmask, &omask);
		t = tnp;
		tnp = NULL;
		tnp = syntax(word, wordp);
		(void)sigprocmask(SIG_SETMASK, &omask, NULL);
		if (error_message != NULL)
			error(-1, error_message);
		else
			execute(tnp, NULL, NULL);
		tfree(tnp);
		tnp = NULL;
		tnp = t;
	}
	return 1;
}

/*
 * Read and parse the alias string specified by string.
 * Return a pointer to aword on success.
 * Return a pointer to NULL  on error.
 */
static const char **
rp_alias(const char *string)
{
	struct tnode *t;
	sigset_t nmask, omask;
	char *wp;

	asp = string;
	linep  = aline;
	elinep = &aline[LINEMAX - 5];
	wordp  = aword;
	ewordp = &aword[WORDMAX - 5];
	error_message = NULL;
	nul_count = 0;
	do {
		wp = linep;
		if (get_word() == EOF) {
			error_message = ERR_SYNTAX;/* same as: ([...] \) */
			asp = NULL;
			return NULL;
		}
	} while (*wp != EOL);
	*wordp = NULL;

	if (error_message != NULL) {
		asp = NULL;
		return NULL;
	}

	if (wordp - aword > 1) {
		if (any(**aword, ";&")) {
			error_message = ERR_SYNTAX;/* same as: (;) or (&) */
			asp = NULL;
			return NULL;
		}
		(void)sigfillset(&nmask);
		(void)sigprocmask(SIG_SETMASK, &nmask, &omask);
		t = NULL;
		t = syntax(aword, wordp);
		(void)sigprocmask(SIG_SETMASK, &omask, NULL);
		if (error_message != NULL) {
			tfree(t);
			t = NULL;
			asp = NULL;
			return NULL;
		}
		tfree(t);
		t = NULL;
		asp = NULL;
		return (const char **)aword;
	}
	error_message = ERR_SYNTAX;/* same as: () */
	asp = NULL;
	return NULL;
}

/*
 * Copy a word from the standard input into the [a]line buffer,
 * and point to it w/ *wordp.  Each copied word is represented
 * in [a]line as an individual `\0'-terminated string.
 */
static int
get_word(void)
{
	int c, c1;
	bool ds;

	*wordp++ = linep;

	for (;;) {
		switch (c = xgetc(DOLSUB)) {
		case EOF:
			return EOF;

		case SPACE:
		case TAB:
			continue;

		case DQUOT:/* "..." == multi-char literal + $ substitution */
		case SQUOT:/* '...' == multi-char literal                  */
			c1 = c;
			ds = (c1 == DQUOT) ? DOLSUB : !DOLSUB;
			*linep++ = c;
			while ((c = xgetc(ds)) != c1) {
				if (c == EOF)
					return EOF;
				if (c == EOL) {
					if (error_message == NULL)
						error_message = ERR_SYNTAX;
					peekc = c;
					*linep++ = EOS;
					return 1;
				}
				if (c == BQUOT) {
					if ((c = xgetc(!DOLSUB)) == EOF)
						return EOF;
					if (c == EOL)
						c = SPACE;/* continue line */
					else {
						peekc = c;
						c = BQUOT;
					}
				}
				*linep++ = c;
			}
			*linep++ = c;
			break;

		case BQUOT:/* \. == one-char literal */
			if ((c = xgetc(!DOLSUB)) == EOF)
				return EOF;
			if (c == EOL)
				continue;
			*linep++ = BQUOT;
			*linep++ = c;
			break;

		case LPARENTHESIS: case RPARENTHESIS:
		case SEMICOLON:    case AMPERSAND:
		case VERTICALBAR:  case CARET:
		case LESSTHAN:     case GREATERTHAN:
		case EOL:
			*linep++ = c;
			*linep++ = EOS;
			return 1;

		default:
			peekc = c;
		}

		for (;;) {
			if ((c = xgetc(DOLSUB)) == EOF)
				return EOF;
			if (c == BQUOT) {
				if ((c = xgetc(!DOLSUB)) == EOF)
					return EOF;
				if (c == EOL)
					c = SPACE;/* continue line */
				else {
					*linep++ = BQUOT;
					*linep++ = c;
					continue;
				}
			}
			if (any(c, WORDPACK)) {
				peekc = c;
				if (any(c, QUOTPACK))
					break;
				*linep++ = EOS;
				return 1;
			}
			*linep++ = c;
		}
	}
	/*NOTREACHED*/
}

/*
 * If dolsub is true, get either the next literal character from the
 * standard input or substitute the current $ dollar w/ the next
 * character of its value, which is pointed to by dolp.  Otherwise,
 * get only the next literal character from the standard input.
 */
static int
xgetc(bool dolsub)
{
	int c;

	if (peekc != EOS) {
		c = peekc;
		peekc = EOS;
		return c;
	}

	if (wordp >= ewordp) {
#ifdef	DEBUG
		fd_print(FD2, "xgetc: wordp: %p, ewordp: %p;\n", wordp, ewordp);
#endif
		wordp -= 10;
		while ((c = xgetc(!DOLSUB)) != EOF && c != EOL)
			;	/* nothing */
		wordp += 10;
		error_message = ERR_TMARGS;
		goto geterr;
	}
	if (linep >= elinep) {
#ifdef	DEBUG
		fd_print(FD2, "xgetc: linep: %p, elinep: %p;\n", linep, elinep);
#endif
		linep -= 10;
		while ((c = xgetc(!DOLSUB)) != EOF && c != EOL)
			;	/* nothing */
		linep += 10;
		error_message = ERR_TMCHARS;
		goto geterr;
	}

getd:
	if (dolp != NULL) {
		c = *dolp++;
		if (c != EOS)
			return c;
		if (dolac > 0 && ++dolac < dolc) {
			dolp = dolv[dolac];
			return SPACE;
		}
		dolac = 0;
		dolp  = NULL;
	}
	c = readc();
	if (c == DOLLAR && dolsub) {
		c = readc();
		if (c == ASTERISK) {
			if (dolc > 1) {
				dolac = 1;
				dolp  = dolv[dolac];
			}
			goto getd;
		}
		if ((dolp = get_dolp(c)) != NULL)
			goto getd;
	}
	/* Handle EOS/NUL characters as needed. */
	if (c == EOS) do {
		if (!PROMPT) {
			if (is_first)
				is_noexec = true;
			if (is_first || ++nul_count >= LINEMAX) {
				error_message = ERR_EXEC;
				goto geterr;
			}
		}
		c = readc();
	} while (c == EOS);
	if (c == EOL)
		is_first = false;
	return c;

geterr:
	if (c == EOL)
		is_first = false;
	return EOL;
}

/*
 * Read and return a character from the string pointed to by asp
 * or argv2p or from the standard input.  When reading from asp
 * or argv2p, return the character, `\n', or EOF.  When reading
 * from the standard input, return the character or EOF.
 */
static int
readc(void)
{
	UChar c;

	if (asp != NULL) {
		if (asp == (char *)-1)
			return EOF;
		if ((c = UCHAR(*asp++)) == EOS) {
			asp = (char *)-1;
			c = UCHAR(EOL);
		}
		return c;
	}
	if (argv2p != NULL) {
		if (argv2p == (char *)-1)
			return EOF;
		if ((c = UCHAR(*argv2p++)) == EOS) {
			argv2p = (char *)-1;
			c = UCHAR(EOL);
		}
		return c;
	}
	if (read(FD0, &c, (size_t)1) != 1)
		return EOF;

	return c;
}

/*
 * Return a pointer to the appropriate $ dollar value on success.
 * Return a NULL pointer on error.
 */
static const char *
get_dolp(int c)
{
	int n, r;
	const char *v;

	*dolbuf = EOS;
	switch (c) {
	case HASH:
		n = (dolc > 1) ? dolc - 1 : 0;
		r = snprintf(dolbuf, sizeof(dolbuf), "%u", (unsigned)n);
		v = (r < 0 || r >= (int)sizeof(dolbuf)) ? NULL : dolbuf;
		break;
	case DOLLAR:
		r = snprintf(dolbuf,sizeof(dolbuf),"%05u",(unsigned)getmypid());
		v = (r < 0 || r >= (int)sizeof(dolbuf)) ? NULL : dolbuf;
		break;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		n = c - '0';
		if (IS_DIGIT(n, c) && n < dolc)
			v = (n > 0) ? dolv[n] : name;
		else
			v = dolbuf;
		break;
	case QUESTION:
		r = snprintf(dolbuf, sizeof(dolbuf), "%u", (unsigned)status);
		v = (r < 0 || r >= (int)sizeof(dolbuf)) ? NULL : dolbuf;
		break;
	case 'd':
		if ((v = getenv("OSHDIR")) == NULL)
			v = dolbuf;
		break;
	case 'e':
		if ((v = getenv("EXECSHELL")) == NULL)
			v = dolbuf;
		break;
	case 'h':
		if ((v = getenv("HOME")) == NULL)
			v = dolbuf;
		break;
	case 'm':
		if ((v = getenv("MANPATH")) == NULL)
			v = dolbuf;
		break;
	case 'p':
		if ((v = getenv("PATH")) == NULL)
			v = dolbuf;
		break;
	case 't':
		v = tty;
		break;
	case 'u':
		v = user;
		break;
	case 'v':
		r = snprintf(dolbuf, sizeof(dolbuf), "%s", OSH_VERSION);
		v = (r < 0 || r >= (int)sizeof(dolbuf)) ? NULL : dolbuf;
		break;
	default:
		v = varget(c);
	}
	return v;
}

/*
 * Allocate memory for new variable node (specified by name and value)
 * if needed, and link it to variable node list.  Insert new variable,
 * or replace old variable value w/ new variable value if needed.
 */
static void
varalloc(int name, const char *value)
{
	struct vnode *n, *p, *v;
	int d;

	if (name == 0 || value == NULL)
		return;

	v = vnp;
	if (v == NULL) { /* First */
		vnp = varalloc1(name, value);
		return;
	}

	/* Do ascending ASCII sort. */
	p = v;
	while (v != NULL) {
		if ((d = (name - v->name)) == 0) {
			if (!EQUAL(value, v->value)) {
				/* Replace old value w/ new value. */
				xfree(v->value);
				v->value = xstrdup(value);
			}
			return;
		}
		if (d < 0)
			break;
		p = v;
		v = v->next;
	}

	/* Do ascending ASCII insert. */
	if (v == NULL) { /* Last */
		p->next = varalloc1(name, value);
		return;
	}
	if (v == vnp) { /* New Head */
		n = varalloc1(name, value);
		n->next = vnp;
		vnp = n;
		return;
	}
	/* Insert new variable between p and v. */
	n = varalloc1(name, value);
	n->next = p->next;
	p->next = n;
}

/*
 * Allocate and initialize memory for new variable node
 * specified by name and value.  Return pointer to new
 * node on success.  Do not return on ENOMEM error.
 */
static struct vnode *
varalloc1(int name, const char *value)
{
	struct vnode *v;

	v = xmalloc(sizeof(struct vnode));
	v->next  = NULL;
	v->name  = name;/* one ASCII char - enforced by IS_VARNAME() macro */
	v->value = xstrdup(value);
	return v;
}

/*
 * If name is specified (is not 0), free its variable.
 * If name is not specified (is 0), free all variables.
 * Return true (1) or false (0) as needed.
 */
static bool
varfree(int name)
{
	struct vnode *p, *v;
	bool r;

	if (name != 0) {
		v = vnp;
		p = v;
		r = false;
		while (v != NULL) {
			if (name == v->name) {
				if (v == vnp)
					vnp = v->next;
				else
					p->next = v->next;
				v->name = 0;
				xfree(v->value);
				xfree(v);
				r = true;
				break;
			}
			p = v;
			v = v->next;
		}
	} else {
		r = (vnp != NULL) ? true : false;
		v = vnp;
		while (v != NULL) {
			p = v;
			v->name = 0;
			xfree(v->value);
			v = v->next;
			xfree(p);
		}
		vnp = NULL;
	}
	return r;
}

/*
 * Get the variable value specified by name.
 * Return a pointer to the variable value on success.
 * Return a pointer to NULL on no-such-name error.
 */
static const char *
varget(int name)
{
	struct vnode *v;
	const char *vv;

	if (name == 0)
		return NULL;

	vv = NULL;
	v  = vnp;
	while (v != NULL) {
		if (name == v->name) {
			vv = v->value;
			break;
		}
		v = v->next;
	}
	return vv;
}

/*
 * Allocate memory for new alias node (specified by name and string)
 * if needed, and link it to alias node list.  Insert new alias, or
 * replace old alias string w/ new alias string if needed.
 */
static void
aalloc(const char *name, const char *string)
{
	struct anode *a, *n, *p;
	int d;

	if (name == NULL || *name == EOS || string == NULL || *string == EOS)
		return;

	a = anp;
	if (a == NULL) { /* First */
		anp = aalloc1(name, string);
		return;
	}

	/* Do ascending ASCII sort. */
	p = a;
	while (a != NULL) {
		if ((d = strcmp(name, a->name)) == 0) {
			if (!EQUAL(string, a->string)) {
				/* Replace old string w/ new string. */
				xfree(a->string);
				a->string = xstrdup(string);
			}
			return;
		}
		if (d < 0)
			break;
		p = a;
		a = a->next;
	}

	/* Do ascending ASCII insert. */
	if (a == NULL) { /* Last */
		p->next = aalloc1(name, string);
		return;
	}
	if (a == anp) { /* New Head */
		n = aalloc1(name, string);
		n->next = anp;
		anp = n;
		return;
	}
	/* Insert new alias between p and a. */
	n = aalloc1(name, string);
	n->next = p->next;
	p->next = n;
}

/*
 * Allocate and initialize memory for new alias node
 * specified by name and string.  Return pointer to new
 * node on success.  Do not return on ENOMEM error.
 */
static struct anode *
aalloc1(const char *name, const char *string)
{
	struct anode *a;

	a = xmalloc(sizeof(struct anode));
	a->next   = NULL;
	a->name   = xstrdup(name);
	a->string = xstrdup(string);
	return a;
}

/*
 * If name is specified (is not NULL), free its alias.
 * If name is not specified (is NULL), free all aliases.
 * Return true (1) or false (0) as needed.
 */
static bool
afree(const char *name)
{
	struct anode *a, *p;
	bool r;

	if (name != NULL) {
		a = anp;
		p = a;
		r = false;
		while (a != NULL) {
			if (EQUAL(name, a->name)) {
				if (a == anp)
					anp = a->next;
				else
					p->next = a->next;
				xfree(a->name);
				xfree(a->string);
				xfree(a);
				r = true;
				break;
			}
			p = a;
			a = a->next;
		}
	} else {
		r = (anp != NULL) ? true : false;
		a = anp;
		while (a != NULL) {
			p = a;
			xfree(a->name);
			xfree(a->string);
			a = a->next;
			xfree(p);
		}
		anp = NULL;
	}
	return r;
}

/*
 * Get the alias string specified by name.
 * Return a pointer to the alias string on success.
 * Return a pointer to NULL on no-such-name error.
 */
static const char *
asget(const char *name)
{
	struct anode *a;
	const char *as;

	if (name == NULL)
		return NULL;

	as = NULL;
	a  = anp;
	while (a != NULL) {
		if (EQUAL(name, a->name)) {
			as = a->string;
			break;
		}
		a = a->next;
	}
	return as;
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
	t->nkey   = 0;
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
		xfree(t->nfin);
		xfree(t->nfout);
		vfree(t->nav);
		break;
	case TSUBSHELL:
		xfree(t->nfin);
		xfree(t->nfout);
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
	static int alcnt;
	int ac, c, n, subcnt;
	char **p, **pv, **lp, **rp;
	char **tav, **tavp;
	const char **av;
	char *fin, *fout;
	const char *as;

	pv    = xmalloc(((p2 - p1) + 1) * sizeof(char *));
	pv[0] = NULL;
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
					fin = xstrdup(*p);
				} else {
					if (fout != NULL)
						goto syn3err;
					fout = xstrdup(*p);
				}
			}
			continue;

		default:
			if (subcnt == 0)
				pv[n++] = xstrdup(*p);
		}

	if (lp == NULL) {
		if (n == 0)
			goto syn3err;
		if ((as = asget(pv[0])) != NULL && asp == NULL) {
			/*
			 * Substitute alias string pointed to by as for
			 * alias name pointed to by pv[0].  Read, parse,
			 * allocate, and copy it into tav.  Execute as
			 * TSUBSHELL w/o ( ) .
			 */
			if (error_message != NULL)
				goto syn3err;
#ifdef	DEBUG
#ifdef	DEBUG_ALIAS
			fd_print(FD2, "syn3: alcnt == %d;\n", alcnt);
#endif
#endif
			if (alcnt > 2) {
				error_message = ERR_ALIASLOOP;
				goto syn3err;
			}
			if ((av = rp_alias(as)) == NULL)
				goto syn3err;
			ac   = vacount(av);
			tav  = xmalloc((ac + n) * sizeof(char *));
			tavp = tav;
#ifdef	DEBUG
#ifdef	DEBUG_ALIAS
			fd_print(FD2, "    : (%d + %d) == %d;\n",ac,n,(ac+n));
			fd_print(FD2, "    :  av : %p;\n", av);
			fd_print(FD2, "    : tav : %p;\n", tav);
#endif
#endif
			for (ac = 0; *av[ac] != EOL; ac++)
				*tavp++ = xstrdup(av[ac]);
			for (ac = 1; ac < n; ac++)
				*tavp++ = xstrdup(pv[ac]);
			*tavp++ = xstrdup("\n");
			*tavp   = NULL;
#ifdef	DEBUG
#ifdef	DEBUG_ALIAS
			for (tavp = tav; *tavp != NULL; tavp++)
				fd_print(FD2, "    : tavp: %p, %p, %s;\n",
				    tavp, *tavp, (**tavp==EOL) ? "\\n" : *tavp);
			fd_print(FD2, "    : tavp: %p, NULL;\n", tavp);
			fd_print(FD2,"    : (tavp - tav) == %d;\n",(tavp-tav));
#endif
#endif
			alcnt++;
			if ((t = talloc()) == NULL)
				goto syn3err;
			t->ntype = TSUBSHELL;
			t->nsub  = syn1(tav, tavp);
			vfree(tav);
			alcnt--;
		} else {
			/*
			 * Execute as TCOMMAND.
			 */
			if ((t = talloc()) == NULL)
				goto syn3err;
			t->ntype = TCOMMAND;
			t->nav   = xmalloc((n + 1) * sizeof(char *));
			for (ac = 0; ac < n; ac++)
				t->nav[ac] = xstrdup(pv[ac]);
			t->nav[ac] = NULL;
			t->nkey    = cmd_lookup(t->nav[0]);
		}
	} else {
		/*
		 * Execute as TSUBSHELL w/ ( ) .
		 */
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
	pv[n] = NULL;
	vfree(pv);
	return t;

syn3err:
	xfree(fin);
	xfree(fout);
	pv[n] = NULL;
	vfree(pv);
	if (error_message == NULL)
		error_message = ERR_SYNTAX;
	return NULL;
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
 * Return true (1) if any argument in the argument vector pointed to by vp
 * contains any unquoted glob characters.  Otherwise, return false (0).
 */
static bool
vtglob(char **vp)
{
	char **p;

	for (p = vp; *p != NULL; p++)
		if (gchar(*p) != NULL)
			return true;
	return false;
}

/*
 * Remove any unquoted quote characters from each argument
 * in the argument vector pointed to by vp.
 */
static void
vtrim(char **vp)
{
	char **p;

	for (p = vp; *p != NULL; p++)
		(void)atrim(UCPTR(*p));
}

/*
 * Return number of arguments in argument vector pointed to by vp.
 */
static int
vacount(const char **vp)
{
	const char **p;

	for (p = vp; *p != NULL; p++)
		;	/* nothing */

	return (int)(p - vp);
}

/*
 * Try to execute the shell command tree pointed to by t.
 */
static void
execute(struct tnode *t, int *pin, int *pout)
{
	struct tnode *t1;
	enum tnflags f;
	pid_t cpid;
	int pfd[2];

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
			error(-1, ERR_PIPE);
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
			error(-1, "execute: Invalid command");
			return;
		}
		if (vtglob(t->nav)) {
			if ((t->nav = (char **)glob(t->nkey, t->nav)) == NULL)
				return;
		} else if (DO_TRIM(t->nkey))
			vtrim(t->nav);
		switch (t->nkey) {
		case SBI_ECHO:
			break;
		case SBI_EXEC:
			/*
			 * Replace the current shell w/ an instance of
			 * the specified external command.
			 *
			 * usage: exec command [arg ...]
			 */
			if (t->nav[1] == NULL) {
				error1(-1, t->nav[0], ERR_ARGCOUNT);
				return;
			}
			if ((t->nkey = cmd_lookup(t->nav[1])) != SBI_UNKNOWN) {
				error2(-1, t->nav[0], t->nav[1], ERR_EXEC);
				return;
			}
			t->nav++;
			t->nflags |= FNOFORK;
			(void)sasignal(SIGCHLD, SIG_IGN);
			break;
		case SBI_FD2: case SBI_GOTO: case SBI_IF: case SBI_UNKNOWN:
			break;
		default:
			execute1(t);
			return;
		}
		/*FALLTHROUGH*/

	case TSUBSHELL:
		f = t->nflags;
		if ((cpid = ((f & FNOFORK) != 0) ? 0 : fork()) == -1) {
			error(-1, ERR_FORK);
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
			if ((f & FPOUT) == 0) {
#ifdef	DEBUG
#ifdef	DEBUG_PROC
				fd_print(FD2, "execute: pwait(%d);\n", cpid);
#endif
#endif
				pwait(cpid);
			}
			return;
		}
		/**** Child! ****/
		execute2(t, pin, pout);
		/*NOTREACHED*/
	}
}

/*
 * Try to execute the special built-in command which is specified by the
 * t->nkey and t->nav fields in the shell command tree pointed to by t.
 */
static void
execute1(struct tnode *t)
{
	struct anode *a;
	struct vnode *v;
	mode_t m;
	const char *emsg, *p;
	bool aok;

	if (t->nav == NULL || t->nav[0] == NULL) {
		/* should never (but can) be true */
		error(-1, "execute1: Invalid command");
		return;
	}
	switch (t->nkey) {
	case SBI_NULL:
		/*
		 * Do nothing and set the exit status to zero.
		 */
		status = SH_TRUE;
		return;

	case SBI_ALIAS:
		/*
		 * If both name and string are specified, set alias name
		 * to alias string.  If no string is specified, print the
		 * current value of alias name's string.  If no arguments
		 * are specified, print each alias name and string.
		 *
		 * NOTE: Each alias string value is printed as (value)
		 *	 to indicate that the shell executes each alias
		 *	 in a subshell.
		 *
		 * usage: alias [name [string]]
		 */
		if (t->nav[1] != NULL) {
			if (t->nav[2] != NULL && t->nav[3] != NULL) {
				emsg = ERR_ARGCOUNT;
				break;
			}
			aok = true;
			for (p = t->nav[1]; *p != EOS; p++)
				if (any(*p, BANCHARS)) {
					aok = false;
					break;
				}
			if (!aok || *t->nav[1] == EOS ||
			    cmd_lookup(t->nav[1]) != SBI_UNKNOWN) {
				error2(-1, t->nav[0], t->nav[1], ERR_BADNAME);
				return;
			}
			if (t->nav[2] != NULL) {
				if (rp_alias(t->nav[2]) == NULL) {
					emsg = error_message;
					break;
				}
				aalloc(t->nav[1], t->nav[2]);
				status = SH_TRUE;
			} else {
				if ((p = asget(t->nav[1])) != NULL)
					fd_print(FD1, "(%s)\n", p);
				status = (p != NULL) ? SH_TRUE : SH_FALSE;
			}
		} else {
			a = anp;
			while (a != NULL) {
				fd_print(FD1, "%s\t(%s)\n", a->name, a->string);
				a = a->next;
			}
			status = (anp != NULL) ? SH_TRUE : SH_FALSE;
		}
		return;

	case SBI_CD:
	case SBI_CHDIR:
		/*
		 * Change the shell's current working directory.
		 */
		do_chdir(t->nav);
		return;

	case SBI_EXIT:
		/*
		 * If the shell is invoked w/ the `-c' or `-t' option, or is
		 * executing an rc file, exit the shell outright if it is not
		 * sourcing another file in the current context.  Otherwise,
		 * cause the shell to cease execution of a file of commands
		 * by seeking to the end of the file (and explicitly exiting
		 * the shell only if the file is not being sourced).
		 */
		if (!PROMPT) {
			if (SHTYPE(ST_ONELINE|ST_RCFILE) && !SHTYPE(ST_SOURCE))
				EXIT(status);
			(void)lseek(FD0, (off_t)0, SEEK_END);
			if (!SHTYPE(ST_SOURCE))
				EXIT(status);
		}
		return;
	case SBI_HELP:
		fd_print(FD1,"        .      goto      source                    ");
		fd_print(FD1,"        :        if        trap                    ");
		fd_print(FD1,"    alias      help       umask                    ");
		fd_print(FD1,"       cd     login     unalias                    ");
		fd_print(FD1,"    chdir    newgrp       unset                    ");
		fd_print(FD1,"     echo       set    unsetenv                    ");
		fd_print(FD1,"     exec     unset     version                    ");
		fd_print(FD1,"     exit    setenv        wait                    ");
		fd_print(FD1,"      fd2     shift                                ");
		break;
	case SBI_LOGIN:
	case SBI_NEWGRP:
		/*
		 * Replace the current interactive shell w/ an
		 * instance of login(1) or newgrp(1).
		 */
		if (PROMPT) {
			p = (t->nkey == SBI_LOGIN) ? PATH_LOGIN : PATH_NEWGRP;
			(void)sasignal(SIGINT, SIG_DFL);
			(void)sasignal(SIGQUIT, SIG_DFL);
			(void)pexec(p, (char *const *)t->nav);
			(void)sasignal(SIGINT, SIG_IGN);
			(void)sasignal(SIGQUIT, SIG_IGN);
		}
		emsg = ERR_EXEC;
		break;

	case SBI_SET:
		/*
		 * If both name and value are specified, set variable
		 * name to variable value.  If no value is specified,
		 * print the current value of variable name.  If no
		 * arguments are specified, print the name and value
		 * of each variable.
		 *
		 * usage: set [name [value]]
		 */
		if (t->nav[1] != NULL) {
			if (t->nav[2] != NULL && t->nav[3] != NULL) {
				emsg = ERR_ARGCOUNT;
				break;
			}
			if (!IS_VARNAME(t->nav[1])) {
				error2(-1, t->nav[0], t->nav[1], ERR_BADNAME);
				return;
			}
			if (t->nav[2] != NULL) {
				varalloc(*t->nav[1], t->nav[2]);
				status = SH_TRUE;
			} else {
				if ((p = varget(*t->nav[1])) != NULL)
					fd_print(FD1, "%s\n", p);
				status = (p != NULL) ? SH_TRUE : SH_FALSE;
			}
		} else {
			v = vnp;
			while (v != NULL) {
				fd_print(FD1, "%c\t%s\n", v->name, v->value);
				v = v->next;
			}
			status = (vnp != NULL) ? SH_TRUE : SH_FALSE;
		}
		return;

	case SBI_SETENV:
		/*
		 * Set the specified environment variable name.
		 *
		 * usage: setenv name value
		 */
		if (t->nav[1]!=NULL && t->nav[2]!=NULL && t->nav[3]==NULL) {

			for (p = t->nav[1]; *p != '=' && *p != EOS; p++)
				;	/* nothing */
			if (*t->nav[1] == EOS || *p == '=') {
				error2(-1, t->nav[0], t->nav[1], ERR_BADNAME);
				return;
			}
			if (setenv(t->nav[1], t->nav[2], 1) == -1)
				error(ESTATUS, ERR_NOMEM);

			status = SH_TRUE;
			return;

		}
		emsg = ERR_ARGCOUNT;
		break;

	case SBI_SHIFT:
		/*
		 * Shift all positional-parameter values to the left by 1.
		 * The value of $0 does not shift.
		 */
		if (dolc > 1) {
			dolv = &dolv[1];
			dolc--;
			status = SH_TRUE;
			return;
		}
		emsg = ERR_NOARGS;
		break;

	case SBI_DOT:
	case SBI_SOURCE:
		/*
		 * Read and execute commands from file and return.
		 *
		 * usage: source file [arg1 ...]
		 */
		if (t->nav[1] != NULL) {
			do_source(t->nav);
			return;
		}
		emsg = ERR_ARGCOUNT;
		break;

	case SBI_TRAP:
		/*
		 * Ignore (or unignore) the specified signals, or
		 * print a list of those signals which are ignored
		 * because of a previous invocation of `trap' in
		 * the current shell.
		 *
		 * usage: trap ['' | - signal_number ...]
		 */
		do_trap(t->nav, t->nflags);
		return;

	case SBI_UMASK:
		/*
		 * Set the file creation mask to the specified
		 * octal value, or print its current value.
		 *
		 * usage: umask [mask]
		 */
		if (t->nav[1] != NULL && t->nav[2] != NULL) {
			emsg = ERR_ARGCOUNT;
			break;
		}
		if (t->nav[1] == NULL) {
			(void)umask(m = (mode_t)umask(0));
			fd_print(FD1, "%04o\n", (unsigned)m);
		} else {
			m = 0;
			for (p = t->nav[1]; *p >= '0' && *p <= '7'; p++)
				m = m * 8 + (*p - '0');
			if (*t->nav[1] == EOS || *p != EOS || m > 0777) {
				error2(-1, t->nav[0], t->nav[1], ERR_BADMASK);
				return;
			}
			(void)umask(m);
		}
		status = SH_TRUE;
		return;

	case SBI_UNALIAS:
		/*
		 * Unset the specified alias name.
		 *
		 * usage: unalias name
		 */
		if (t->nav[1] != NULL && t->nav[2] == NULL) {
			aok = true;
			for (p = t->nav[1]; *p != EOS; p++)
				if (any(*p, BANCHARS)) {
					aok = false;
					break;
				}
			if (!aok || *t->nav[1] == EOS ||
			    cmd_lookup(t->nav[1]) != SBI_UNKNOWN) {
				error2(-1, t->nav[0], t->nav[1], ERR_BADNAME);
				return;
			}
			status = (afree(t->nav[1])) ? SH_TRUE : SH_FALSE;
			return;
		}
		emsg = ERR_ARGCOUNT;
		break;

	case SBI_UNSET:
		/*
		 * Unset the specified variable name.
		 *
		 * usage: unset name
		 */
		if (t->nav[1] != NULL && t->nav[2] == NULL) {
			if (!IS_VARNAME(t->nav[1])) {
				error2(-1, t->nav[0], t->nav[1], ERR_BADNAME);
				return;
			}
			status = (varfree(*t->nav[1])) ? SH_TRUE : SH_FALSE;
			return;
		}
		emsg = ERR_ARGCOUNT;
		break;

	case SBI_UNSETENV:
		/*
		 * Unset the specified environment variable name.
		 *
		 * usage: unsetenv name
		 */
		if (t->nav[1] != NULL && t->nav[2] == NULL) {

			for (p = t->nav[1]; *p != '=' && *p != EOS; p++)
				;	/* nothing */
			if (*t->nav[1] == EOS || *p == '=') {
				error2(-1, t->nav[0], t->nav[1], ERR_BADNAME);
				return;
			}
			unsetenv(t->nav[1]);

			status = SH_TRUE;
			return;

		}
		emsg = ERR_ARGCOUNT;
		break;

	case SBI_VERSION:
		/*
		 * Print version and build system of current shell.
		 * This information comes from $v (OSH_VERSION) and
		 * from build-time output of `uname -srm' (see uname(1)
		 * and OSH_UNAME_SRM in "config.h").
		 */
		fd_print(FD1, "%s (%s)\n", OSH_VERSION, OSH_UNAME_SRM);
		status = SH_TRUE;
		return;

	case SBI_WAIT:
		/*
		 * Wait for all asynchronous processes to terminate,
		 * reporting on abnormal terminations.
		 */
		pwait(-1);
		return;

	default:
		emsg = ERR_EXEC;
	}
	error1(-1, t->nav[0], emsg);
}

/*
 * Try to execute the child process which is specified by combination
 * of the t->nsub, t->nflags, t->nav, t->nfin, and t->nfout fields in
 * the shell command tree pointed to by t.
 */
static void
execute2(struct tnode *t, int *pin, int *pout)
{
	struct tnode *t1;
	enum tnflags f;
	int i;

	f = t->nflags;

	/*
	 * Redirect (read) input from pipe.
	 */
	if (pin != NULL && (f & FPIN) != 0) {
		if (dup2(pin[0], FD0) == -1)
			error(FC_ERR, strerror(errno));
		(void)close(pin[0]);
		(void)close(pin[1]);
	}
	/*
	 * Redirect (write) output to pipe.
	 */
	if (pout != NULL && (f & FPOUT) != 0) {
		if (dup2(pout[1], FD1) == -1)
			error(FC_ERR, strerror(errno));
		(void)close(pout[0]);
		(void)close(pout[1]);
	}
	/*
	 * Redirect (read) input from file.
	 */
	if (t->nfin != NULL && (f & FPIN) == 0) {
		f |= FFIN;
		i  = 0;
		if (*t->nfin == HYPHEN && *(t->nfin + 1) == EOS)
			i = 1;
		if (i != 0)
			i = dup(dupfd0);
		else
			i = open(atrim(UCPTR(t->nfin)), O_RDONLY);
		if (i == -1)
			error1(FC_ERR, t->nfin, ERR_OPEN);
		if (dup2(i, FD0) == -1)
			error(FC_ERR, strerror(errno));
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
		if ((i = open(atrim(UCPTR(t->nfout)), i, 0666)) == -1)
			error1(FC_ERR, t->nfout, ERR_CREATE);
		if (dup2(i, FD1) == -1)
			error(FC_ERR, strerror(errno));
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
				error1(FC_ERR, "/dev/null", ERR_OPEN);
		}
	} else {
		if ((sig_state&S_SIGINT) == 0 && (sig_child&S_SIGINT) != 0)
			(void)sasignal(SIGINT, SIG_DFL);
		if ((sig_state&S_SIGQUIT) == 0 && (sig_child&S_SIGQUIT) != 0)
			(void)sasignal(SIGQUIT, SIG_DFL);
	}
	/* Set the SIGTERM signal to its default action if needed. */
	if ((sig_state&S_SIGTERM) == 0 && (sig_child&S_SIGTERM) != 0)
		(void)sasignal(SIGTERM, SIG_DFL);
	if (t->ntype == TSUBSHELL) {
		if ((t1 = t->nsub) != NULL)
			t1->nflags |= (f & (FFIN | FPIN | FINTR));
		execute(t1, NULL, NULL);
		_exit(status);
	}
	if (t->nav == NULL || t->nav[0] == NULL) {
		/* should never (but can) be true */
		error(FC_ERR, "execute2: Invalid command");
		/*NOTREACHED*/
	}
	if (t->nkey == SBI_UNKNOWN)
		(void)err_pexec(t->nav[0], (char *const *)t->nav);
	else
		_exit(uexec(t->nkey, vacount((const char **)t->nav), t->nav));
	/*NOTREACHED*/
}

/*
 * Change the shell's current working directory.
 *
 *	`chdir'		Changes to the user's home directory.
 *	`chdir -'	Changes to the previous working directory.
 *	`chdir dir'	Changes to the directory specified by `dir'.
 *
 * NOTE: The user must have both read and search permission on
 *	 a directory in order for `chdir -' to be effective.
 */
static void
do_chdir(char **av)
{
	const char *emsg, *home;
	int cwd;
	static int pwd = -1;

	emsg = ERR_BADDIR;

	cwd = open(".", O_RDONLY | O_NONBLOCK);

	if (av[1] == NULL) {
		home = getenv("HOME");
		if (home == NULL || *home == EOS) {
			emsg = ERR_NOHOMEDIR;
			goto chdirerr;
		}
		if (chdir(home) == -1)
			goto chdirerr;
	} else if (EQUAL(av[1], "-")) {
		if (pwd == -1) {
			emsg = ERR_NOPWD;
			goto chdirerr;
		}
		if (!fd_type(pwd, FD_ISDIROK) || fchdir(pwd) == -1) {
			if (close(pwd) != -1)
				pwd = -1;
			goto chdirerr;
		}
	} else if (chdir(atrim(UCPTR(av[1]))) == -1)
		goto chdirerr;

	if (cwd != -1) {
		if (cwd != PWD && (pwd = dup2(cwd, PWD)) == PWD)
			(void)fcntl(pwd, F_SETFD, FD_CLOEXEC);
		(void)close(cwd);
	} else if (close(pwd) != -1)
		pwd = -1;
	status = SH_TRUE;
	return;

chdirerr:
	if (cwd != -1)
		(void)close(cwd);
	error1(-1, av[0], emsg);
}

/*
 * Ignore (or unignore) the specified signals, or print a list of
 * those signals which are ignored because of a previous invocation
 * of `trap' in the current shell.
 */
static void
do_trap(char **av, enum tnflags f)
{
	struct sigaction act, oact;
	sigset_t new_mask, old_mask;
	long signo;
	int i, sigerr;
	static bool ignlst[NSIG], gotlst;

	/* Temporarily block all signals in this function. */
	(void)sigfillset(&new_mask);
	(void)sigprocmask(SIG_SETMASK, &new_mask, &old_mask);

	if (!gotlst) {
		/* Initialize the list of already ignored signals. */
		for (i = 1; i < NSIG; i++) {
			ignlst[i - 1] = false;
			if (sigaction(i, NULL, &oact) < 0)
				continue;
			if (oact.sa_handler == SIG_IGN)
				ignlst[i - 1] = true;
		}
		gotlst = true;
	}

	sigerr = 0;
	if (av[1] != NULL) {
		if (av[2] == NULL) {
			sigerr = 1;
			goto sigdone;
		}

		(void)memset(&act, 0, sizeof(act));
		(void)sigemptyset(&act.sa_mask);
		if (EQUAL(av[1], ""))
			act.sa_handler = SIG_IGN;
		else if (EQUAL(av[1], "-"))
			act.sa_handler = SIG_DFL;
		else {
			sigerr = 1;
			goto sigdone;
		}
		act.sa_flags = SA_RESTART;

		for (i = 2; av[i] != NULL; i++) {
			if (!strtoint(av[i], &signo) ||
			     signo <= 0 || signo >= NSIG) {
				sigerr = i;
				goto sigdone;
			}

			/* Do nothing if no signal changes are needed. */
			if (sigaction((int)signo, NULL, &oact) < 0 ||
			    (act.sa_handler  == SIG_DFL &&
			     oact.sa_handler == SIG_DFL))
				continue;

			/* Set up already ignored signal if needed. */
			if (ignlst[signo - 1]) {
				set_ss_flags(signo, act.sa_handler);
				if (act.sa_handler == SIG_DFL)
					continue;
			}

			/* Reinstall SIGHUP signal handler if needed. */
			if (is_login&&signo==SIGHUP&&act.sa_handler==SIG_DFL) {
				if (oact.sa_handler == sighup)
					continue;
				if (sasignal((int)signo, sighup) == SIG_ERR) {
					sigerr = (int)signo;
					goto sigdone;
				}
				continue;
			}

			/*
			 * Trying to ignore SIGKILL, SIGSTOP, and/or SIGCHLD
			 * has no effect.
			 */
			if (signo == SIGKILL ||
			    signo == SIGSTOP || signo == SIGCHLD ||
			    sigaction((int)signo, &act, NULL) < 0)
				continue;

			set_ss_flags(signo, act.sa_handler);
		}
	} else {
		/* Print signals currently ignored because of `trap'. */
		for (i = 1; i < NSIG; i++) {
			if (sigaction(i, NULL, &oact) < 0 ||
			    oact.sa_handler != SIG_IGN)
				continue;
			if (!ignlst[i - 1] || ((f & FINTR) == 0 &&
			    ((i == SIGINT  && (sig_state & S_SIGINT)  != 0 &&
			     (sig_child & S_SIGINT)  != 0)   ||
			     (i == SIGQUIT && (sig_state & S_SIGQUIT) != 0 &&
			     (sig_child & S_SIGQUIT) != 0))) ||
			     (i == SIGTERM && (sig_state & S_SIGTERM) != 0 &&
			     (sig_child & S_SIGTERM) != 0))
				fd_print(FD1,"%s '' %2u\n", av[0], (unsigned)i);
		}
	}

sigdone:
	(void)sigprocmask(SIG_SETMASK, &old_mask, NULL);
	if (sigerr == 0)
		status = SH_TRUE;
	else if (sigerr == 1)
		error1(-1, av[0], ERR_GENERIC);
	else
		error2(-1, av[0], av[sigerr], ERR_BADSIGNAL);
}

/*
 * Set global sig_state flags according to signal sig and action act.
 */
static void
set_ss_flags(long sig, action_type act)
{

	if (act == SIG_IGN) {
		if (sig == SIGINT)
			sig_state |= S_SIGINT;
		else if (sig == SIGQUIT)
			sig_state |= S_SIGQUIT;
		else if (sig == SIGTERM)
			sig_state |= S_SIGTERM;
	} else if (act == SIG_DFL) {
		if (sig == SIGINT)
			sig_state &= ~S_SIGINT;
		else if (sig == SIGQUIT)
			sig_state &= ~S_SIGQUIT;
		else if (sig == SIGTERM)
			sig_state &= ~S_SIGTERM;
	}
}

/*
 * Read and execute commands from the file av[1] and return.
 * Calls to this function may be nested either to the point imposed by
 * SOURCEMAX (64) or to that of any limits in the shell's environment,
 * such as running out of file descriptors, hitting a limit on the
 * size of the stack, etc.
 */
static void
do_source(char **av)
{
	static const char *sargv2p;
	const char *sname;
	char *const *sdolv;
	int nfd, sfd, sdolc;
	static int cnt;

	if ((nfd = source_open(av[0], av[1])) == -1) {
		error2(-1, av[0], av[1], (errno != ENOENT && errno != ENOTDIR) ?
		       ERR_OPEN : ERR_NOTFOUND);
		return;
	}
	if (nfd >= SAVFD0 || !fd_type(nfd, FD_ISREG)) {
		(void)close(nfd);
		error2(-1, av[0], av[1], ERR_EXEC);
		return;
	}
	sfd = dup2(FD0, SAVFD0 + cnt);
	if (cnt > SOURCEMAX || sfd == -1 || dup2(nfd, FD0) == -1 ||
	    fcntl(FD0, F_SETFL, O_RDONLY & ~O_NONBLOCK) == -1) {
		(void)close(sfd);
		(void)close(nfd);
		error2(-1, av[0], av[1], strerror(EMFILE));
		return;
	}
	(void)fcntl(sfd, F_SETFD, FD_CLOEXEC);
	(void)close(nfd);

	if (!SHTYPE(ST_SOURCE)) {
		shtype |= ST_SOURCE;
		sargv2p = argv2p;
		argv2p  = NULL;
	}

	/* Save and initialize any positional parameters. */
	sname = name, sdolv = dolv, sdolc = dolc;
	name  = av[1], dolv = &av[1];
	for (dolc = 0; dolv[dolc] != NULL; dolc++)
		;	/* nothing */

	cnt++;
	cmd_loop(HALT);
	cnt--;

	/* Restore any saved positional parameters. */
	name = sname, dolv = sdolv, dolc = sdolc;

	if (error_source || (is_login && logout_now != 0)) {
		/*
		 * Terminate any and all source commands (nested or not).
		 * Restore original standard input before returning for
		 * the final time, and call err() if needed.
		 */
		if (cnt == 0) {
			/* Restore original standard input or die trying. */
			if (dup2(SAVFD0, FD0) == -1)
				error2(ESTATUS, av[0], av[1], strerror(errno));
			(void)close(SAVFD0);
			shtype &= ~ST_SOURCE;
			argv2p  = sargv2p;
			sargv2p = NULL;
			if (!SHTYPE(ST_RCFILE))
				err(0, NULL);
			return;
		}
		(void)close(sfd);
		return;
	}

	/* Restore previous standard input or die trying. */
	if (dup2(sfd, FD0) == -1) {
		(void)close(sfd);
		error2(ESTATUS, av[0], av[1], strerror(errno));
		return;
	}
	(void)close(sfd);

	if (cnt == 0) {
		shtype &= ~ST_SOURCE;
		argv2p  = sargv2p;
		sargv2p = NULL;
	}
}

/*
 * Open the specified file (or path name) for the specified `.' or
 * `source' special built-in command.  Return the file descriptor
 * fd >= 0 for the open file on success.  Return fd == -1 on error.
 *
 *	Derived from:
 *		- pexec() in osh-20120102/pexec.c
 */
static int
source_open(const char *command, const char *file)
{
	size_t dlen, flen;
	int fd;
	const char *d, *f, *pnp, *upp;
	char pnbuf[PATHMAX];

	/* Fail if the value of file is invalid. */
	errno = 0;
	if (*file == EOS)
		goto open_fail;
	flen = strlen(file);

	/*
	 * If the name of the specified file contains one or more
	 * `/' characters, it is used as the path name to source.
	 */
	for (f = file; *f != EOS; f++)
		if (*f == SLASH) {
			pnp = file;
			upp = "";
			goto source_pathname;
		}
	*pnbuf = EOS;
	pnp = pnbuf;

	/* Get the user's PATH or fail. */
	upp = getenv("PATH");
	if (upp == NULL || *upp == EOS)
		goto open_fail;

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
		 * path name for file.  Then, attempt to open(2) it.
		 */
		if (dlen + flen + 1 >= sizeof(pnbuf)) {
			struct iovec msg[6];
			msg[0].iov_base = (char *)getmyname();
			msg[0].iov_len  = strlen(getmyname());
			msg[1].iov_base = ": ";
			msg[1].iov_len  = (size_t)2;
			msg[2].iov_base = (char *)command;
			msg[2].iov_len  = strlen(command);
			msg[3].iov_base = ": ";
			msg[3].iov_len  = (size_t)2;
			msg[4].iov_base = (char *)d;
			msg[4].iov_len  = dlen;
			msg[5].iov_base = ": path too long\n";
			msg[5].iov_len  = (size_t)16;
			(void)writev(FD2, msg, 6);
			errno = ENAMETOOLONG;
			continue;
		}
		(void)memcpy(pnbuf, d, dlen);
		pnbuf[dlen] = SLASH;
		(void)memcpy(pnbuf + dlen + 1, file, flen);
		pnbuf[dlen + flen + 1] = EOS;

source_pathname:
		if ((fd = open(pnp, O_RDONLY | O_NONBLOCK)) != -1)
			return fd;
		switch (errno) {
		case EACCES: case EISDIR:
		case ELOOP:  case ENAMETOOLONG:
		case ENOENT: case ENOTDIR:
			break;
		default:
			goto open_fail;
		}
	} while (*upp++ == COLON);	/* Otherwise, *upp was NUL. */

open_fail:
	if (errno == 0)
		errno = ENOENT;
#ifdef	DEBUG
	fd_print(FD2, "source_open: strerror(errno) == %s;\n", strerror(errno));
#endif
	return -1;
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
				err(0, NULL);
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
#ifdef	DEBUG
#ifdef	DEBUG_PROC
		fd_print(FD2, "prsig: tp == %d, cp == %d;\n", tp, cp);
#endif
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

	switch (es) {
	case -2:
		/* For compatibility, glob() errors are not fatal. */
		status = SH_ERR;
		return;
	case -1:
		status = SH_ERR;
		/*FALLTHROUGH*/
	case 0:
		if (SHTYPE(ST_SOURCE)) {
			(void)lseek(FD0, (off_t)0, SEEK_END);
			error_source = true;
			return;
		}
		if (SHTYPE(ST_RCFILE) && (status == 130 || status == 131)) {
			(void)lseek(FD0, (off_t)0, SEEK_END);
			return;
		}
		if (!SHTYPE(ST_INTERACTIVE)) {
			if (!SHTYPE(ST_ONELINE))
				(void)lseek(FD0, (off_t)0, SEEK_END);
			break;
		}
		return;
	default:
		status = es;
	}
	EXIT(status);
}

/*
 * Initialize the shell.
 */
static void
sh_init(const char *av0p)
{
	struct passwd *pwu;
	int fd;
	const char *p;

	setmyerrexit(&sh_errexit);
	setmyname(av0p);
	setmypid(getpid());
	sheuid = geteuid();

	/*
	 * Set-ID execution is not supported.
	 */
	if (sheuid != getuid() || getegid() != getgid())
		err(SH_ERR, FMT2S, getmyname(), ERR_SETID);

	/*
	 * Fail if any of the descriptors 0, 1, or 2 is not open,
	 * or if dupfd0 cannot be set up properly.
	 */
	for (fd = 0; fd < 3; fd++)
		if (!fd_type(fd, FD_ISOPEN))
			err(SH_ERR, "%s: %u: %s\n",
			    getmyname(), (unsigned)fd, strerror(errno));
	if ((dupfd0 = dup2(FD0, DUPFD0)) == -1 ||
	    fcntl(dupfd0, F_SETFD, FD_CLOEXEC) == -1)
		err(SH_ERR, "%s: %u: %s\n",
		    getmyname(), DUPFD0, strerror(errno));

	/* Try to get the terminal name for $t. */
	p   = ttyname(dupfd0);
	tty = xstrdup((p != NULL) ? p : "");

	/* Try to get the effective user name for $u. */
	pwu  = getpwuid(sheuid);
	user = xstrdup((pwu != NULL) ? pwu->pw_name : "");

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
	size_t len;
	int c;

	is_first  = true;
	is_noexec = false;
	if (fd_type(FD0, FD_ISREG) && lseek(FD0, (off_t)0, SEEK_CUR) == 0) {
		if (readc() == HASH && readc() == BANG) {
			for (len = 2; len < LINEMAX; len++) {
				if ((c = readc()) == EOS) {
					is_noexec = true;
					break;
				}
				if (c == EOL)
					is_first = false;
				if (c == EOL || c == EOF)
					return;
			}
			error(-1, ERR_EXEC);
		} else
			(void)lseek(FD0, (off_t)0, SEEK_SET);
	}
}

/*
 * Return true (1) if the shell is connected to a terminal.
 * Otherwise, return false (0).
 */
static bool
sh_on_tty(void)
{

	return isatty(FD0) != 0 && isatty(FD2) != 0;
}

/*
 * Handle the SIGHUP signal by setting the volatile logout_now flag.
 */
static void
sighup(/*@unused@*/ int signo IS_UNUSED)
{

	logout_now = 1;
}

/*
 * Process the sequence of rc init files used by the shell.
 * For each call to rc_init(), temporarily assign the shell's
 * standard input to come from a given file in the sequence if
 * possible and return.  When DO_INIT_DONE, restore the shell's
 * original standard input (or die trying), unset the ST_RCFILE
 * flag, and return.
 */
static void
rc_init(int *rcflag)
{
	char path[PATHMAX];
	const char *file;

	while (*rcflag <= DO_INIT_DONE) {
		file = NULL;
		switch (*rcflag) {
		case DO_SYSTEM_LOGIN:
			file = PATH_SYSTEM_LOGIN;
			break;
		case DO_SYSTEM_OSHRC:
			if (!is_login)
				(*rcflag)++;
			file = PATH_SYSTEM_OSHRC;
			break;
		case DO_DOT_LOGIN:
			file = pn_build(path, FILE_DOT_LOGIN, sizeof(path));
			break;
		case DO_DOT_OSHRC:
			file = pn_build(path, FILE_DOT_OSHRC, sizeof(path));
			break;
		case DO_INIT_DONE:
			if (dup2(dupfd0, FD0) == -1)
				err(SH_ERR,FMT2S,getmyname(),strerror(errno));
			shtype &= ~ST_RCFILE;
			(*rcflag)++;
			return;
		default:
			shtype &= ~ST_RCFILE;
			return;
		}
		if (file != NULL) {
			rcfilename = xstrdup(file);
			(*rcflag)++;
			if (rc_open(file))
				break;
		} else
			/* should never (but can) be true */
			err(SH_ERR,FMT2S,getmyname(),"rc_init: Invalid file");
	}
}

/*
 * If is_login is false, unset the ST_RCFILE flag and return.
 * Otherwise, process the sequence of rc logout files used by
 * the shell.  For each call to rc_logout(), temporarily assign
 * the shell's standard input to come from a given file in the
 * sequence if possible and return.  When DO_LOGOUT_DONE, unset
 * the ST_RCFILE flag and return.
 */
static void
rc_logout(int *rcflag)
{
	char path[PATHMAX];
	const char *file;

	if (!is_login) {
		shtype &= ~ST_RCFILE;
		return;
	}

	while (*rcflag <= DO_LOGOUT_DONE) {
		file = NULL;
		switch (*rcflag) {
		case DO_SYSTEM_LOGOUT:
			file = PATH_SYSTEM_LOGOUT;
			break;
		case DO_DOT_LOGOUT:
			file = pn_build(path, FILE_DOT_LOGOUT, sizeof(path));
			break;
		case DO_LOGOUT_DONE:
			shtype &= ~ST_RCFILE;
			(*rcflag)++;
			return;
		default:
			shtype &= ~ST_RCFILE;
			return;
		}
		if (file != NULL) {
			rcfilename = xstrdup(file);
			(*rcflag)++;
			if (rc_open(file))
				break;
		} else
			/* should never (but can) be true */
			err(SH_ERR,FMT2S,getmyname(),"rc_logout: Invalid file");
	}
}

/*
 * Open the rc file specified by file, and prepare it for execution.
 * The specified file must be readable and regular (or a link to a
 * regular file) for the shell to use it.  Return true (1) if file
 * is ready for execution, or return false (0) if file cannot be
 * opened or executed.  Do not return on dup2(2) or fcntl(2) error.
 */
static bool
rc_open(const char *file)
{
	int fd;

	if (file == NULL || *file == EOS)
		return false;

	if ((fd = open(file, O_RDONLY | O_NONBLOCK)) == -1)
		return false;

	if (!fd_type(fd, FD_ISREG)) {
		err(-1, FMT3S, getmyname(), file, ERR_EXEC);
		(void)close(fd);
		return false;
	}

	/* NOTE: A dup2(2) or fcntl(2) error here is fatal. */
	if (dup2(fd, FD0) == -1 ||
	    fcntl(FD0, F_SETFL, O_RDONLY & ~O_NONBLOCK) == -1)
		err(SH_ERR, FMT3S, getmyname(), file, strerror(errno));

	(void)close(fd);
	shtype |= ST_RCFILE;
	return true;
}

/*
 * Build a path name for the file name pointed to by file.
 * Write the resulting path name to the buffer pointed to by path.
 * The size of the buffer pointed to by path is specified by size.
 * Return path, which will be the empty string if the build fails.
 */
static char *
pn_build(char *path, const char *file, size_t size)
{
	int len;
	const char *home;

	*path = EOS;
	home  = getenv("HOME");

	if (home != NULL && *home != EOS) {
		len = snprintf(path, size, "%s/%s", home, file);
		if (len < 0 || len >= (int)size) {
			err(-1, "%s: %s/%s: %s\n",
			    getmyname(), home, file, strerror(ENAMETOOLONG));
			*path = EOS;
		}
	}

	return path;
}

/*
 * If hwfd is a valid open file descriptor and hwflag is true,
 * write each argument/word in the word pointer array to hwfd.
 * Otherwise, do nothing.
 */
static void
hist_write(bool hwflag)
{
	char **vp;

	if (hwfd == -1 || !hwflag)
		return;
	if (!fd_type(hwfd, FD_ISREGOK)) {
		if (close(hwfd) != -1)
			hwfd = -1;
		return;
	}
	for (vp = word; **vp != EOL; vp++)
		fd_print(hwfd, "%s%s", *vp, (**(vp + 1) != EOL) ? " " : "");
	fd_print(hwfd, FMT1S, "");
}

/*
 * Open the user's FILE_DOT_HISTORY file for writing if possible.
 * Set the global hwfd to HWFD on success.
 * Set the global hwfd to -1   on error.
 */
static void
hist_open(void)
{
	char path[PATHMAX];
	const char *file;
	int fd, fdw;

	hwfd = -1;
	file = pn_build(path, FILE_DOT_HISTORY, sizeof(path));
	if ((fd = open(file, O_WRONLY | O_APPEND | O_NONBLOCK)) == -1)
		return;
	if (!fd_type(fd, FD_ISREG)) {
		(void)close(fd);
		return;
	}
	if ((fdw = dup2(fd, HWFD)) == -1 ||
	     fcntl(fdw, F_SETFL, (O_WRONLY | O_APPEND) & ~O_NONBLOCK) == -1 ||
	     fcntl(fdw, F_SETFD, FD_CLOEXEC) == -1) {
		(void)close(fd);
		(void)close(fdw);
		return;
	}
	(void)close(fd);
	hwfd = fdw;
}

/*
 * Attempt to free or release all of the file descriptors in the range
 * from (fd_max - 1) through (FD2 + 1), skipping DUPFD0; the value of
 * fd_max may fall between FDFREEMIN and FDFREEMAX, inclusive.
 */
static void
fd_free(void)
{
	long fd_max;
	int fd;

	fd_max = sysconf(_SC_OPEN_MAX);
	if (fd_max < FDFREEMIN || fd_max > FDFREEMAX)
		fd_max = FDFREEMIN;
	for (fd = (int)fd_max - 1; fd > DUPFD0; fd--)
		(void)close(fd);
	for (fd--; fd > FD2; fd--)
		(void)close(fd);
}

/*
 * Check if the file descriptor fd refers to an open file
 * of the specified type; return true (1) or false (0).
 */
static bool
fd_type(int fd, mode_t type)
{
	struct stat sb;
	bool rv = false;

	if (fstat(fd, &sb) < 0)
		return rv;

	switch (type) {
	case FD_ISOPEN:
		rv = true;
		break;
	case FD_ISDIROK:
		if (S_ISDIR(sb.st_mode))
			rv = sb.st_nlink > 0;
		break;
	case FD_ISREGOK:
		if (S_ISREG(sb.st_mode))
			rv = sb.st_nlink > 0;
		break;
	case FD_ISDIR:
	case FD_ISREG:
		rv = (sb.st_mode & FD_TMASK) == type;
		break;
	}
	return rv;
}

/*
 * Remove (trim) any unquoted quote characters from argument
 * pointed to by ap, make copy of, and return pointer to it.
 * This function never returns on error.
 */
static char *
atrim(UChar *ap)
{
	size_t siz;
	long l;
	const char *m;
	UChar *a, *b;
	UChar buf[LINEMAX], c;
	bool d;

	*buf = UCHAR(EOS);
	for (a = ap, b = buf; b < &buf[LINEMAX]; a++, b++) {
		switch (*a) {
		case EOS:
			*b = UCHAR(EOS);
			siz = (b - buf) + 1;
			(void)memcpy(ap, buf, siz);
			return (char *)ap;
		case DQUOT:
		case SQUOT:
			c = *a++;
			d = (c == DQUOT) ? true : false;
			while (*a != c && b < &buf[LINEMAX]) {
				if (*a == EOS)
					goto aterr;
				if (d && *a == BQUOT && *(a + 1) == DOLLAR)
					a++;
				*b++ = *a++;
			}
			b--;
			continue;
		case BQUOT:
			if (*++a == EOS) {
				a--, b--;
				continue;
			}
			break;
		}
		*b = *a;
	}

aterr:
	l = (is_noexec || no_lnum) ? -1 : get_lnum();
	m = (const char *)ap;
	if (name != NULL) {
		if (l != -1)
			err(ESTATUS,FMTAT1LS,getmyname(), name, l, ERR_TRIM, m);
		else
			err(ESTATUS, FMTAT1S, getmyname(), name, ERR_TRIM, m);
	} else {
		if (l != -1)
			err(ESTATUS, FMTATLS, getmyname(), l, ERR_TRIM, m);
		else
			err(ESTATUS, FMTATS, getmyname(), ERR_TRIM, m);
	}
	/*NOTREACHED*/
	return NULL;
}

/*
 * Prepare possible glob() pattern pointed to by ap.
 *
 *	1) Remove (trim) any unquoted quote characters;
 *	2) Escape (w/ backslash `\') any previously quoted
 *	   glob or quote characters as needed;
 *	3) Reallocate memory for (if needed), make copy of,
 *	   and return pointer to new glob() pattern, nap.
 *
 * This function returns NULL on error.
 */
static char *
gtrim(UChar *ap)
{
	size_t siz;
	UChar *a, *b, *nap;
	UChar buf[PATHMAX], c;
	bool d;

	*buf = UCHAR(EOS);
	for (a = ap, b = buf; b < &buf[PATHMAX]; a++, b++) {
		switch (*a) {
		case EOS:
			*b = UCHAR(EOS);
			siz = (b - buf) + 1;
			nap = ap;
			if (siz > strlen((const char *)ap) + 1) {
				xfree(nap);
				nap = xmalloc(siz);
			}
			(void)memcpy(nap, buf, siz);
			return (char *)nap;
		case DQUOT:
		case SQUOT:
			c = *a++;
			d = (c == DQUOT) ? true : false;
			while (*a != c && b < &buf[PATHMAX]) {
				switch (*a) {
				case EOS:
					goto gterr;
				case BQUOT:
					if (d && *(a + 1) == DOLLAR)
						a++;
					/*FALLTHROUGH*/
				case ASTERISK: case QUESTION:
				case LBRACKET: case RBRACKET: case HYPHEN:
				case DQUOT:    case SQUOT:
					*b = UCHAR(BQUOT);
					if (++b >= &buf[PATHMAX])
						goto gterr;
					break;
				}
				*b++ = *a++;
			}
			b--;
			continue;
		case BQUOT:
			switch (*++a) {
			case EOS:
				a--, b--;
				continue;
			case ASTERISK: case QUESTION:
			case LBRACKET: case RBRACKET: case HYPHEN:
			case DQUOT:    case SQUOT:    case BQUOT:
				*b = UCHAR(BQUOT);
				if (++b >= &buf[PATHMAX])
					goto gterr;
				break;
			}
			break;
		}
		*b = *a;
	}

gterr:
	error(-2, (gchar((const char *)ap) != NULL) ?
	      ERR_PATTOOLONG : strerror(ENAMETOOLONG));
	return NULL;
}

/*
 * Return pointer to first unquoted glob character (`*', `?', `[')
 * in argument pointed to by ap.  Otherwise, return NULL pointer
 * on error or if argument contains no glob characters.
 */
static char *
gchar(const char *ap)
{
	char c;
	const char *a;

	for (a = ap; *a != EOS; a++)
		switch (*a) {
		case DQUOT:
		case SQUOT:
			for (c = *a++; *a != c; a++)
				if (*a == EOS)
					return NULL;
			continue;
		case BQUOT:
			if (*++a == EOS)
				return NULL;
			continue;
		case ASTERISK:
		case QUESTION:
		case LBRACKET:
			return (char *)a;
		}
	return NULL;
}

/*
 * Deallocate the argument vector pointed to by vp.
 */
static void
vfree(char **vp)
{
	char **p;

	if (vp != NULL) {
		for (p = vp; *p != NULL; p++) {
			free(*p);
			*p = NULL;
		}
		free(vp);
		vp = NULL;
	}
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
		error(ESTATUS, ERR_NOMEM);
		/*NOTREACHED*/
	}
	return mp;
}

/*
 * Reallocate memory, and check for error.
 * Return a pointer to the reallocated space on success.
 * Do not return on ENOMEM error.
 */
static void *
xrealloc(void *p, size_t s)
{
	void *rp;

	if ((rp = realloc(p, s)) == NULL) {
		error(ESTATUS, ERR_NOMEM);
		/*NOTREACHED*/
	}
	return rp;
}

/*
 * Allocate memory for a copy of the string src, and copy it to dst.
 * Return a pointer to dst on success.
 * Do not return on ENOMEM error.
 */
static char *
xstrdup(const char *src)
{
	size_t siz;
	char *dst;

	siz = strlen(src) + 1;
	dst = xmalloc(siz);
	(void)memcpy(dst, src, siz);
	return dst;
}

static	const char	**gavp;	/* points to current gav position     */
static	const char	**gave;	/* points to current gav end          */
static	unsigned	gavmult;/* GAVNEW reallocation multiplier     */
static	size_t		gavtot;	/* total bytes used for all arguments */

static	const char	**gnew(/*@only@*/ const char **);
/*@null@*/
static	char		*gcat(/*@null@*/ const char *,
			      /*@null@*/ const char *, bool);
static	const char	**glob1(enum sbikey, /*@only@*/ const char **,
				char *, int *, bool *);
static	bool		glob2(const UChar *, const UChar *);
static	void		gsort(const char **);
/*@null@*/
static	DIR		*gopendir(/*@out@*/ char *, const char *);

/*
 * Attempt to generate file-name arguments which match the given
 * pattern arguments in av.  Return pointer to newly allocated
 * argument vector, gav, on success.  Return NULL on error.
 */
static const char **
glob(enum sbikey key, char **av)
{
	char *tpap;		/* temporary pattern argument pointer  */
	char **oav;		/* points to original argument vector  */
	const char **gav;	/* points to generated argument vector */
	int pmc = 0;		/* pattern match count                 */
	bool gerr = false;	/* glob error flag                     */

	gavmult = 1;
	gavtot  = 0;

	oav  = av;
	gav  = xmalloc(GAVNEW * sizeof(char *));
	*gav = NULL;
	gavp = gav;
	gave = &gav[GAVNEW - 1];
	while (*av != NULL) {
		if ((tpap = gtrim(UCPTR(*av))) == NULL) {
			*gavp = NULL;
			gerr  = true;
			break;
		}
		*av = tpap;/* for successful vfree(oav); */
		gav = glob1(key, gav, tpap, &pmc, &gerr);
		if (gerr)
			break;
		av++;
	}
	gavp = NULL;

	if (pmc == 0 && !gerr) {
		error(-2, ERR_NOMATCH);
		gerr = true;
	}
	if (gerr) {
		vfree((char **)gav);
		gav = NULL;
	}
	vfree(oav);
	oav = NULL;
	return gav;
}

static const char **
gnew(const char **gav)
{
	size_t siz;
	ptrdiff_t gidx;

	if (gavp == gave) {
		gavmult *= GAVMULT;
#ifdef	DEBUG
#ifdef	DEBUG_GLOB
		fd_print(FD2, "gnew: gavmult == %u;\n", gavmult);
#endif
#endif
		gidx = (ptrdiff_t)(gavp - gav);
		siz  = (size_t)((gidx + (GAVNEW * gavmult)) * sizeof(char *));
#ifdef	DEBUG
#ifdef	DEBUG_GLOB
		fd_print(FD2, "    : (GAVNEW * gavmult) == %u, siz == %zu;\n",
		    (GAVNEW * gavmult), siz);
#endif
#endif
		gav  = xrealloc(gav, siz);
		gavp = gav + gidx;
		gave = &gav[gidx + (GAVNEW * gavmult) - 1];
	}
	return gav;
}

static char *
gcat(const char *src1, const char *src2, bool slash)
{
	size_t siz;
	char *b, buf[PATHMAX], c, *dst;
	const char *s;

	if (src1 == NULL || src2 == NULL) {
		/* never true, but appease splint(1) */
		error(-2, "gcat: Invalid argument");
		return NULL;
	}

	*buf = EOS;
	b = buf;
	s = src1;
	while ((c = *s++) != EOS) {
		if (b >= &buf[PATHMAX - 1]) {
			error(-2, strerror(ENAMETOOLONG));
			return NULL;
		}
		*b++ = c;
	}
	if (slash)
		*b++ = SLASH;
	s = src2;
	do {
		if (b >= &buf[PATHMAX]) {
			error(-2, strerror(ENAMETOOLONG));
			return NULL;
		}
		*b++ = c = *s++;
	} while (c != EOS);
	b--;

	siz = (b - buf) + 1;
	gavtot += siz;
	if (gavtot > GAVMAX) {
		error(-2, ERR_E2BIG);
		return NULL;
	}
#ifdef	DEBUG
#ifdef	DEBUG_GLOB
	fd_print(FD2, "gcat: siz == %zu, (%p < %p) == %s;\n",
	    siz, b, &buf[PATHMAX], (b < &buf[PATHMAX]) ? "true" : "false");
	fd_print(FD2, "    : strlen(buf) == %zu;\n", strlen(buf));
#endif
#endif
	dst = xmalloc(siz);

	(void)memcpy(dst, buf, siz);
	return dst;
}

static const char **
glob1(enum sbikey key, const char **gav, char *as, int *pmc, bool *gerr)
{
	DIR *dirp;
	struct dirent *entry;
	ptrdiff_t gidx;
	bool slash;
	char dirbuf[PATHMAX], *p, *ps;
	const char *ds;

	ds = as;
	slash = false;
	if ((ps = gchar(as)) == NULL) {
		gav = gnew(gav);
		if (DO_TRIM(key))
			(void)atrim(UCPTR(as));
		if ((p = gcat(as, "", slash)) == NULL) {
			*gavp = NULL;
			*gerr = true;
			return gav;
		}
		*gavp++ = p;
		*gavp = NULL;
		return gav;
	}
	for (;;) {
		if (ps == ds) {
			ds = "";
			break;
		}
		if (*--ps == SLASH) {
			*ps = EOS;
			if (ds == ps)
				ds = "/";
			else
				slash = true;
			ps++;
			break;
		}
	}
	if ((dirp = gopendir(dirbuf, *ds != EOS ? ds : ".")) == NULL) {
		error(-2, ERR_NODIR);
		*gavp = NULL;
		*gerr = true;
		return gav;
	}
	if (*ds != EOS)
		ds = dirbuf;
	gidx = (ptrdiff_t)(gavp - gav);
	while ((entry = readdir(dirp)) != NULL) {
		if (entry->d_name[0] == DOT && *ps != DOT)
			continue;
		if (glob2(UCPTR(entry->d_name), UCPTR(ps))) {
			gav = gnew(gav);
			if ((p = gcat(ds, entry->d_name, slash)) == NULL) {
				(void)closedir(dirp);
				*gavp = NULL;
				*gerr = true;
				return gav;
			}
			*gavp++ = p;
			(*pmc)++;
		}
	}
	(void)closedir(dirp);
	gsort(gav + gidx);
	*gavp = NULL;
	*gerr = false;
	return gav;
}

static bool
glob2(const UChar *ename, const UChar *pattern)
{
	int cok, rok;		/* `[...]' - cok (class), rok (range) */
	UChar c, ec, pc;
	const UChar *e, *n, *p;

	e = ename;
	p = pattern;

	ec = *e++;
	switch (pc = *p++) {
	case EOS:
		return ec == EOS;

	case ASTERISK:
		/*
		 * Ignore all but the last `*' in a group of consecutive
		 * `*' characters to avoid unnecessary glob2() recursion.
		 */
		while (*p++ == ASTERISK)
			;	/* nothing */
		if (*--p == EOS)
			return true;
		e--;
		while (*e != EOS)
			if (glob2(e++, p))
				return true;
		break;

	case QUESTION:
		if (ec != EOS)
			return glob2(e, p);
		break;

	case LBRACKET:
		if (*p == EOS)
			break;
		for (c = UCHAR(EOS), cok = rok = 0, n = p + 1; ; ) {
			pc = *p++;
			if (pc == RBRACKET && p > n) {
				if (cok > 0 || rok > 0)
					return glob2(e, p);
				break;
			}
			if (*p == EOS)
				break;
			if (pc == HYPHEN && c != EOS && *p != RBRACKET) {
				if ((pc = *p++) == BQUOT)
					pc = *p++;
				if (*p == EOS)
					break;
				if (c <= ec && ec <= pc)
					rok++;
				else if (c == ec)
					cok--;
				c = UCHAR(EOS);
			} else {
				if (pc == BQUOT) {
					pc = *p++;
					if (*p == EOS)
						break;
				}
				c = pc;
				if (ec == c)
					cok++;
			}
		}
		break;

	case BQUOT:
		if (*p != EOS)
			pc = *p++;
		/*FALLTHROUGH*/

	default:
		if (pc == ec)
			return glob2(e, p);
	}
	return false;
}

static void
gsort(const char **ogavp)
{
	const char **p1, **p2, *sap;

	p1 = ogavp;
	while (gavp - p1 > 1) {
		p2 = p1;
		while (++p2 < gavp)
			if (strcmp(*p1, *p2) > 0) {
				sap = *p1;
				*p1 = *p2;
				*p2 = sap;
			}
		p1++;
	}
}

static DIR *
gopendir(char *buf, const char *dname)
{
	char *b;
	const char *d;

	for (*buf = EOS, b = buf, d = dname; b < &buf[PATHMAX]; b++, d++)
		if ((*b = *d) == EOS) {
			(void)atrim(UCPTR(buf));
			break;
		}
	return (b >= &buf[PATHMAX]) ? NULL : opendir(buf);
}
