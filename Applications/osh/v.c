/*
 * v.c - osh package version and copyright notices
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
 *	@(#)$Id: e7ab59be0e1b80fd62b6325bc76a41882cb68447 $
 */

#include "config.h"

#ifndef	lint
#ifndef	OSH_ATTR
# if __GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ >= 4
#  define	OSH_ATTR	__attribute__((__used__))
# elif defined(__GNUC__)
#  define	OSH_ATTR	__attribute__((__unused__))
# else
#  define	OSH_ATTR	/* nothing */
# endif
#endif	/* !OSH_ATTR */

/*@unused@*/
static const char *const vcid[] OSH_ATTR = {
	"\100(#)\044Id: " OSH_VERSION " (" OSH_UNAME_SRM ") \044",
	"\100(#)\044Id: Copyright (c) 2003-2015 Jeffrey Allen Neitzel. \044",
	"\100(#)\044Id: Copyright (c) 2001-2002 Caldera International Inc. \044",
	"\100(#)\044Id: Copyright (c) 1985, 1989, 1991, 1993 The Regents of the University of California. \044"
};
#endif	/* !lint */
