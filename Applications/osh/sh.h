/*
 * osh - an enhanced port of the Sixth Edition (V6) UNIX Thompson shell
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
 *	@(#)$Id: f37776fa012580259b55e26e0186ec32e6ad4ace $
 */

#ifndef	SH_H
#define	SH_H

/*
 * signal flags
 */
enum sigflags {
	S_SIGINT  = 01,
	S_SIGQUIT = 02,
	S_SIGTERM = 04
};

#ifdef	OSH_SHELL
/*
 * shell special built-in (sbi) command keys
 */
enum sbikey {
	SBI_NULL,  SBI_ALIAS,    SBI_CD,      SBI_CHDIR,  SBI_ECHO,
	SBI_EXEC,  SBI_EXIT,     SBI_FD2,     SBI_GOTO,   SBI_IF,
	SBI_LOGIN, SBI_NEWGRP,   SBI_SET,     SBI_SETENV, SBI_SHIFT,
	SBI_DOT,   SBI_SOURCE,   SBI_TRAP,    SBI_UMASK,  SBI_UNALIAS,
	SBI_UNSET, SBI_UNSETENV, SBI_VERSION, SBI_WAIT,   SBI_UNKNOWN
};

#define	IS_VARNAME(v)	(((*(v) >= 'A' && *(v) <= 'Z') || (*(v) >= 'a' && *(v) <= 'z')) && !any(*(v), "#$*?0123456789dehmptuv") && *((v) + 1) == EOS)

/*
 * shell variable node structure
 */
struct vnode {
	/*@null@*/ /*@only@*/
	struct vnode	*next;	/* Pointer to next variable node.       */
	int		 name;	/* This variable name (one ASCII char). */
	/*@only@*/
	char		*value;	/* Pointer to this variable value.      */
};

/*
 * shell alias node structure
 */
struct anode {
	/*@null@*/ /*@only@*/
	struct anode	*next;	/* Pointer to next alias node.   */
	/*@only@*/
	char		*name;	/* Pointer to this alias name.   */
	/*@only@*/
	char		*string;/* Pointer to this alias string. */
};
#endif

/*
 * shell command tree node flags
 */
enum tnflags {
	FAND    = 0001,		/* A `&'  designates asynchronous execution.  */
	FCAT    = 0002,		/* A `>>' appends output to file.             */
	FFIN    = 0004,		/* A `<'  redirects input from file.          */
	FPIN    = 0010,		/* A `|' or `^' redirects input from pipe.    */
	FPOUT   = 0020,		/* A `|' or `^' redirects output to pipe.     */
	FNOFORK = 0040,		/* No fork(2) for last command in `( list )'. */
	FINTR   = 0100,		/* Child process ignores SIGINT and SIGQUIT.  */
	FPRS    = 0200		/* Print process ID of child as a string.     */
};

/*
 * shell command tree node structure
 */
struct tnode {
	/*@null@*/ /*@only@*/
	struct tnode	 *nleft;	/* Pointer to left node.            */
	/*@null@*/ /*@only@*/
	struct tnode	 *nright;	/* Pointer to right node.           */
	/*@null@*/ /*@only@*/
	struct tnode	 *nsub;		/* Pointer to TSUBSHELL node.       */
	/*@null@*/ /*@only@*/
	char		 *nfin;		/* Pointer to input file (<).       */
	/*@null@*/ /*@only@*/
	char		 *nfout;	/* Pointer to output file (>, >>).  */
	/*@null@*/ /*@only@*/
	char		**nav;		/* Argument vector for TCOMMAND.    */
#ifdef	OSH_SHELL
	enum sbikey	  nkey;		/* Shell sbi command key.           */
#endif
	enum tnflags	  nflags;	/* Shell command tree node flags.   */
	enum {				/* Shell command tree node type.    */
		TLIST     = 1,	/* pipelines separated by `;', `&', or `\n' */
		TPIPE     = 2,	/* commands separated by `|' or `^'         */
		TCOMMAND  = 3,	/* command  [arg ...]  [< in]  [> [>] out]  */
		TSUBSHELL = 4	/* ( list )            [< in]  [> [>] out]  */
	} ntype;
};

#ifdef	OSH_SHELL
/* osh.c */
extern	uid_t	sheuid;	/* effective shell user ID */

enum sbikey	cmd_lookup(const char *);

/* util.c */
/*@maynotreturn@*/
int		uexec(enum sbikey, int, char **);
#endif

#endif	/* !SH_H */
