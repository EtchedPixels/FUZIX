/*
 * Trivial implementation of uname command
 *  -n option is not supported, no nodename from struct utsname
 *
 * Copyright (C) 2016 David Flamand, All rights reserved.
 *
 * This file is part of FUZIX Operating System.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <sys/utsname.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int fd;
char *argv0, aflag, mflag, rflag, sflag, vflag, next;

void error(void)
{
	perror(argv0);
	exit(1);
}

void putstr(char *str)
{
	ssize_t ret, len;
	len = strlen(str);
	while (len > 0) {
		ret = write(fd, str, len);
		if (ret == -1)
			error();
		str += ret;
		len -= ret;
	}
}

void usage(void)
{
	fd = STDERR_FILENO;
	putstr("usage: ");
	putstr(argv0);
	putstr(" [-amrsv]\n");
	exit(1);
}

void print(char *s)
{
	if (next)
		putstr(" ");
	putstr(s);
	next = 1;
}

int main(int argc, char *argv[])
{
	struct utsname un;
	char *p;
	int i;

	fd = STDOUT_FILENO;
	argv0 = "";
	if (argc) {
		argv0 = argv[0];
		for (i = 1; i < argc; ++i) {
			p = argv[i];
			if (p[0] == '-') {
				for (++p; *p; ++p) {
					switch (*p) {
					case 'a': aflag = 1; break;
					case 'm': mflag = 1; break;
					case 'r': rflag = 1; break;
					case 's': sflag = 1; break;
					case 'v': vflag = 1; break;
					default:
						usage();
					}
				}
			} else
				usage();
		}
	}

	if (uname(&un) != 0)
		error();

	if (!mflag && !rflag && !vflag)
		sflag = 1;

	if (aflag)
		mflag = rflag = sflag = vflag = 1;

	if (sflag)
		print(un.sysname);

	if (rflag)
		print(un.release);

	if (vflag)
		print(un.version);

	if (mflag)
		print(un.machine);

	next = 0;
	print("\n");

	return 0;
}
