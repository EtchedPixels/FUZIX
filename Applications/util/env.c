/*

    Copyright 1999 by Philip Homburg and Kees Bot.
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

      1. Redistributions of source code must retain the above copyright
         notice, this list of conditions and the following disclaimer.
      2. Redistributions in binary form must reproduce the above
         copyright notice, this list of conditions and the following
	 disclaimer in the documentation and/or other materials provided
	 with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS
    OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Kees J. Bot (kjb@cs.vu.nl)
    Philip Homburg (philip@cs.vu.nl)
*/



/*	env 1.0 - Set environment for command		Author: Kees J. Bot
 *								17 Dec 1997
 */
#define nil 0
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	int i;
	int iflag= 0;
	int aflag= 0;
	extern char **environ;

	i= 1;
	while (i < argc && argv[i][0] == '-') {
		char *opt= argv[i++] + 1;

		if (opt[0] == '-' && opt[1] == 0) break;	/* -- */

		if (opt[0] == 0) iflag= 1;			/* - */

		while (*opt != 0) switch (*opt++) {
		case 'i':
			iflag= 1;	/* Clear environment. */
			break;
		case 'a':		/* Specify arg 0 separately. */
			aflag= 1;
			break;
		default:
			fprintf(stderr,
		"Usage: env [-ia] [name=value] ... [utility [argument ...]]\n");
			exit(1);
		}
	}

	/* Clear the environment if -i. */
	if (iflag) *environ= nil;

	/* Set the new environment strings. */
	while (i < argc && strchr(argv[i], '=') != nil) {
		if (putenv(argv[i]) != 0) {
			fprintf(stderr, "env: Setting '%s' failed: %s\n",
				argv[i], strerror(errno));
			exit(1);
		}
		i++;
	}

	if (i >= argc) {
		/* No utility given; print environment. */
		char **ep;

		for (ep= environ; *ep != nil; ep++) {
			if (puts(*ep) == EOF) {
				fprintf(stderr, "env: %s\n", strerror(errno));
				exit(1);
			}
		}
		return 0;
	} else {
		char *util, **args;
		int err;

		util= argv[i];
		args= argv + i;
		if (aflag) args++;
		execvp(util, args);
		err= errno;
		fprintf(stderr, "env: Can't execute %s: %s\n",
			util, strerror(err));
		return err == ENOENT ? 127 : 126;
	}
}
