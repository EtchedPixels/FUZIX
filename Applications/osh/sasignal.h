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
 *	@(#)$Id: e5035837d8b9ca4722ddc145921ef1c6f193b346 $
 */

#ifndef	SASIGNAL_H
#define	SASIGNAL_H

/*
 * required header files
 */
#include <signal.h>

/*
 * The sasignal() function is a signal() with BSD-like semantics.
 * On success, sasignal() returns the previous action.  Otherwise,
 * it returns SIG_ERR and sets errno according to the error.
 */

typedef	void	(*action_type)(int);		/* signal action type */

action_type	sasignal(int, action_type);

#endif	/* !SASIGNAL_H */
