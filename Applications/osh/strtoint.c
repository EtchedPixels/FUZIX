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
 *	@(#)$Id: 3a25f8695e5619c78b02d47eff50ddd75c41098b $
 */

#include "defs.h"
#include "err.h"
#include "strtoint.h"

bool
strtoint(const char *s, long *l)
{
	long ll;
	char *lbad;
	const char *n;

	n = getmyname();

	errno = 0;
	if (s == NULL || l == NULL) {
		errno = EINVAL;
		fd_print(FD2, FMT3S, n, "strtoint", strerror(errno));
		if (l != NULL)
			*l = 0;
		return false;
	}

	ll = strtol(s, &lbad, 10);
	if (*s == EOS || (*s == '0' && *(s + 1) >= '0' && *(s + 1) <= '9') ||
	    *lbad != EOS) {
		errno = EINVAL;
		if (EQUAL(n, "if"))
			fd_print(FD2, FMT3S, n, s, ERR_BADINTEGER);
		*l = 0;
		return false;
	}
	if (errno == ERANGE) {
		errno = ERANGE;
		if (EQUAL(n, "if"))
			fd_print(FD2, FMT3S, n, s, ERR_RANGE);
		*l = 0;
		return false;
	}

	*l = ll;
	return true;
}
