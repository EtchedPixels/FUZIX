/*
 * Copyright (c) 1993 by David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Most simple built-in commands are here.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define putstr(x)	write(1, x, strlen(x))
#define eputstr(x)	write(2, x, strlen(x))

static char buf1[512];
static char buf2[512];

int main(int argc, char *argv[])
{
	int		fd1;
	int		fd2;
	int		cc1;
	int		cc2;
	long		pos;
	char		*bp1;
	char		*bp2;
	struct	stat	statbuf1;
	struct	stat	statbuf2;

	if (argc < 3) {
		eputstr(argv[0]);
		eputstr(": file1 file2.\n");
		exit(2);
	}
	if (stat(argv[1], &statbuf1) < 0) {
		perror(argv[1]);
		exit(2);
	}

	if (stat(argv[2], &statbuf2) < 0) {
		perror(argv[2]);
		exit(2);
	}

	if ((statbuf1.st_dev == statbuf2.st_dev) &&
		(statbuf1.st_ino == statbuf2.st_ino))
	{
		putstr("Files are links to each other\n");
		exit(0);
	}

	if (statbuf1.st_size != statbuf2.st_size) {
		putstr("Files are different sizes\n");
		exit(1);
	}

	fd1 = open(argv[1], 0);
	if (fd1 < 0) {
		perror(argv[1]);
		exit(2);
	}

	fd2 = open(argv[2], 0);
	if (fd2 < 0) {
		perror(argv[2]);
		close(fd1);
		exit(2);
	}

	pos = 0;
	while (1) {
		cc1 = read(fd1, buf1, sizeof(buf1));
		if (cc1 < 0) {
			perror(argv[1]);
			exit(2);
		}

		cc2 = read(fd2, buf2, sizeof(buf2));
		if (cc2 < 0) {
			perror(argv[2]);
			goto differ;
		}

		if ((cc1 == 0) && (cc2 == 0)) {
			putstr("Files are identical\n");
			goto same;
		}

		if (cc1 < cc2) {
			putstr("First file is shorter than second\n");
			goto differ;
		}

		if (cc1 > cc2) {
			putstr("Second file is shorter than first\n");
			goto differ;
		}

		if (memcmp(buf1, buf2, cc1) == 0) {
			pos += cc1;
			continue;
		}

		bp1 = buf1;
		bp2 = buf2;
		while (*bp1++ == *bp2++)
			pos++;

		putstr("Files differ at byte position ");
		bp1 = (char *)_ultoa(pos);
		putstr(bp1);
		putstr("\n");
		goto differ;
	}
same:
	exit(0);
differ:
	exit(1);
}
