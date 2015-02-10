/*
 * osh - an enhanced port of the Sixth Edition (V6) UNIX Thompson shell
 */
/*-
 * Copyright (c) 2008-2015
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
 *	@(#)$Id: b1509cdba1006155d14553bee5e5b1a3e4246ebe $
 */

#ifndef	STRTOINT_H
#define	STRTOINT_H

/*
 * NAME
 *	strtoint - convert a string value into a long integer value
 *
 * SYNOPSIS
 *	#include "strtoint.h"
 *
 *	bool
 *	strtoint(const char *string, long *integer);
 *
 * DESCRIPTION
 *	The strtoint() function converts the specified string into a
 *	decimal integer value of type long.  It assigns the converted
 *	value to the storage pointed to by the specified long integer.
 *
 * RETURN VALUES
 *	On success, strtoint() returns true with the converted long
 *	integer value assigned to the storage pointed to by the long
 *	integer.  On error, it returns false with 0 assigned to the
 *	storage pointed to by the long integer and sets errno
 *	according to the error.
 *
 * ERRORS
 *	EINVAL	The specified string is NULL, empty,
 *		or cannot be converted by strtol(3).
 *
 *	EINVAL	The specified long integer is NULL.
 *
 *	ERANGE	The specified string is out of range.
 */

bool	strtoint(/*@null@*/ const char *, /*@null@*/ /*@out@*/ long *);

#endif	/* !STRTOINT_H */
