/*
 * osh - an enhanced port of the Sixth Edition (V6) UNIX Thompson shell
 */
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
 *	@(#)$Id: 4e61e8697c5eeabedd2727b8ff61ceaa27d3c380 $
 */

#ifndef	PEXEC_H
#define	PEXEC_H

/*
 * NAME
 *	pexec - execute a file or path name
 *
 * SYNOPSIS
 *	#include "pexec.h"
 *
 *	extern char **environ;
 *
 *	int
 *	pexec(const char *file, char *const argv[]);
 *
 * DESCRIPTION
 *	The pexec() function replaces the current process with a new process
 *	by calling execve(2).  The file argument specifies a file or path name
 *	to execute.  The argv argument is a pointer to a NULL-terminated array
 *	of pointers to `\0'-terminated strings, which specifies the argument
 *	list for the new process.
 *
 *	If the name of the specified file contains one or more `/' characters,
 *	it is used as the path name to execute.
 *
 *	Otherwise, a path search is performed.  The environment variable PATH
 *	specifies the search path to be used.  The search builds a sequence of
 *	possible path names for the specified file, attempting to execve(2)
 *	each one until success or failure.  The search continues on any of
 *	the following errors if PATH is not yet exhausted:
 *
 *		EACCES, EISDIR (OpenBSD), ELOOP, ENAMETOOLONG, ENOENT, ENOTDIR
 *
 *	If an EACCES, or EISDIR (OpenBSD), error occurs during the search
 *	and if no other executable file is found, pexec() sets errno to
 *	EACCES upon failure.  The search ceases immediately on any error
 *	not mentioned above (except ENOEXEC), or when PATH is exhausted.
 *
 *	If execve(2) fails and sets errno to ENOEXEC, pexec() attempts to
 *	execute the path name with the shell specified by the environment
 *	variable EXECSHELL.  If this attempt fails, pexec() also fails.
 *
 *	Notice that if PATH is unset or is set to the empty string, the
 *	name of the specified file must contain one or more `/' characters
 *	in order to be executed.  Otherwise, pexec() shall fail.
 *
 *	Notice also that if EXECSHELL is unset or is set to the empty
 *	string, pexec() shall make no attempt to execute the specified
 *	file with any shell.  Instead, pexec() shall fail.
 *
 * RETURN VALUES
 *	On success, pexec() does not return.  Otherwise, it returns
 *	a value of -1 and sets errno according to the error.
 *
 * ERRORS
 *	Possible errno values set by pexec() shall correspond to those
 *	set by execve(2), with the following special exceptions:
 *
 *		EINVAL	The value of file, argv, or argv[0] is NULL.
 *
 *		ENOEXEC	The file argument specifies an executable file
 *			which does not begin with the proper magic number.
 *			At the same time, the value of EXECSHELL is unset,
 *			is set to the empty string, or is set to another
 *			unusable value which causes execve(2) to fail.
 */

/*@maynotreturn@*/
int	pexec(/*@null@*/ const char *, /*@null@*/ char *const *);

/*
 * NAME
 *	err_pexec - execute a file or path name w/ error handling
 *
 * SYNOPSIS
 *	#include "pexec.h"
 *
 *	extern const char *name;
 *	extern bool       no_lnum;
 *
 *	void
 *	err_pexec(const char *file, char *const argv[]);
 */

/*@noreturn@*/
void	err_pexec(/*@null@*/ const char *, /*@null@*/ char *const *);

#endif	/* !PEXEC_H */
