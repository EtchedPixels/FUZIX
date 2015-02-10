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
 *	@(#)$Id: 6c1e7b6b334c2d8fddbc8ce8bfa4fb837262b2bf $
 */

#ifndef	ERR_H
#define	ERR_H

/*
 * required header files
 */
#include <stdarg.h>

/*
 * diagnostics
 */
#define	ERR_ALIASLOOP	"Alias loop error"
#define	ERR_GARGCOUNT	"Arg count"
#define	ERR_E2BIG	"Arg list too long"
#define	ERR_FORK	"Cannot fork - try again"
#define	ERR_PIPE	"Cannot pipe - try again"
#define	ERR_TRIM	"Cannot trim"
#define	ERR_WRITE	"Cannot write"
#define	ERR_CLOVERFLOW	"Command line overflow"
#define	ERR_GNOTFOUND	"Command not found."
#define	ERR_ALINVAL	"Invalid argument list"
#define	ERR_AVIINVAL	"Invalid argv index"
#define	ERR_NODIR	"No directory"
#define	ERR_NOHOMEDIR	"No home directory"
#define	ERR_NOMATCH	"No match"
#define	ERR_NOPWD	"No previous directory"
#define	ERR_NOSHELL	"No shell!"
#define	ERR_NOTTY	"No terminal!"
#define	ERR_NOMEM	"Out of memory"
#define	ERR_PATTOOLONG	"Pattern too long"
#define	ERR_SETID	"Set-ID execution denied"
#define	ERR_TMARGS	"Too many args"
#define	ERR_TMCHARS	"Too many characters"
#define	ERR_ARGCOUNT	"arg count"
#define	ERR_BADDIGIT	"bad digit"
#define	ERR_BADDIR	"bad directory"
#define	ERR_BADINTEGER	"bad integer"
#define	ERR_BADMASK	"bad mask"
#define	ERR_BADNAME	"bad name"
#define	ERR_BADSIGNAL	"bad signal"
#define	ERR_CREATE	"cannot create"
#define	ERR_EXEC	"cannot execute"
#define	ERR_OPEN	"cannot open"
#define	ERR_SEEK	"cannot seek"
#define	ERR_GENERIC	"error"
#define	ERR_LABNOTFOUND	"label not found"
#define	ERR_LABTOOLONG	"label too long"
#define	ERR_NOARGS	"no args"
#define	ERR_NOTFOUND	"not found"
#define	ERR_RANGE	"out of range"
#define	ERR_SYNTAX	"syntax error"
#define	ERR_PAREN	") expected"
#define	ERR_ARGUMENT	"argument expected"
#define	ERR_COMMAND	"command expected"
#define	ERR_DIGIT	"digit expected"
#define	ERR_EXPR	"expression expected"
#define	ERR_INTEGER	"integer expected"
#define	ERR_OPERATOR	"operator expected"
#define	ERR_BRACE	"} expected"
#define	ERR_OPUNKNOWN	"unknown operator"
#define	FD2_USAGE	"usage: %s [-e] [-f file] [--] command [arg ...]\n"

#define	FMT1S		"%s\n"
#define	FMT2LS		"%s: %ld: %s\n"
#define	FMT2S		"%s: %s\n"
#define	FMT3LFS		"%s: %ld: %s: %s\n"
#define	FMT3LS		"%s: %s: %ld: %s\n"
#define	FMT3S		"%s: %s: %s\n"
#define	FMT4LFS		"%s: %ld: %s: %s: %s\n"
#define	FMT4LS		"%s: %s: %ld: %s: %s\n"
#define	FMT4S		"%s: %s: %s: %s\n"
#define	FMT5LS		"%s: %s: %ld: %s: %s: %s\n"
#define	FMT5S		"%s: %s: %s: %s: %s\n"
#define	FMTAT1LS	"%s: %s: %ld: %s %s\n"
#define	FMTAT1S		"%s: %s: %s %s\n"
#define	FMTATLS		"%s: %ld: %s %s\n"
#define	FMTATS		"%s: %s %s\n"

/*
 * exit status values
 */
#define	FC_ERR		124	/* fatal child error (changed in pwait()) */
#define	SH_ERR		2	/* shell-detected error (default value)   */
#define	SH_FALSE	1
#define	SH_TRUE		0

#define	ESTATUS		((getpid() == getmypid()) ? SH_ERR : FC_ERR)
#define	EXIT(s)		((getpid() == getmypid()) ? exit((s)) : _exit((s)))

/*
 * #undef DEBUG* for production build
 */
#undef	DEBUG
#undef	DEBUG_ALIAS
#undef	DEBUG_GLOB
#undef	DEBUG_LED
#undef	DEBUG_PROC

/*@maynotreturn@*/
void		err(int, /*@null@*/ const char *, /*@printflike@*/ ...);
void		fd_print(int, /*@null@*/ const char *, /*@printflike@*/ ...);
long		get_lnum(void);
/*@observer@*/
const char	*getmyname(void);
pid_t		getmypid(void);
void		setmyerrexit(void (*)(int));
void		setmyname(/*@null@*/ /*@observer@*/ const char *);
void		setmypid(const pid_t);
/*@noreturn@*/
void		ut_errexit(int);

#endif	/* !ERR_H */
