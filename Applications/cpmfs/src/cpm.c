/*	cpm.c	1.15	83/06/20	*/
/*
 * This original copyright has been obsoleted, and is mentioned only
 * for completeness here. - jw
 */
/*
	Copyright (c) 1982, 1983 by
	Helge Skrivervik, UCB.
	All rights reserved.
	Permission granted for use by UNIX* licencees.
	UNIX is a trademark of Bell Labs.

*/
/* the actual copyright notice: */
/*
 * Copyright (c) 1982, 1983, 1994 Helge Skrivervik
 *
 * All rights reserved.
 *
 * This program is free software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Joerg Wunsch
 * 4. The name of the developer may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE DEVELOPERS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE DEVELOPERS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include "cpm.h"

#define BIG	0xFFFFUL

C_FILE c_iob[C_NFILE];
int fid;


int dflag, cflag, iflag, tflag;
int blksiz = 1024;		/* default block size */
int tracks = 77;		/* default number of tracks */
int maxdir = 64;		/* default number of directory slots per disk */
int seclth = 128;		/* default sector length [bytes] */
int sectrk = 26;		/* default number of sectors per track */
int skew = 6;			/* default sector skew factor */
int restrk = 2;			/* reserved tracks (for system) */

int *bitmap = 0;
struct directory *dirbuf;
int use16bitptrs;		/* use 16-bit pointers in directory */

char *string;

int main(int argc, char *argv[])
{

	char *cpmname = NULL, *unixname = NULL ;
	int xflag = 0, stat = 0, Cflag = 0, Iflag = 0, Bflag = 0;

	if (argc == 1)
		usage();
	for (; argc > 1 && argv[1][0] == '-'; argc--, argv++) {
		switch (argv[1][1]) {

		case 0:
			break;

		case 'B':
			Bflag++;
			continue;

		case 'd':
			dflag++;
			continue;

		case 'c':
			if (Cflag)
				stat++;
			cpmname = argv[2];
			unixname = argv[3];
			argc -= 2;
			argv += 2;
			cflag++;
			continue;

		case 'i':
			if (isatty(0) && isatty(1))
				iflag++;
			continue;

		case 'p':
			cpmname = argv[2];
			++argv;
			--argc;
			tflag++;
			continue;

		case 's':
			string = argv[2];
			++argv;
			skew = number(BIG);
			argc--;
			if ((skew < 1) || (skew > 10)) {
				printf("skew factor out of range\n");
				exit(1);
			}
			continue;

		case 'b':
			string = argv[2];
			++argv;
			blksiz = number(BIG);
			argc--;
			if (blksiz & 0xc3) {
				printf("illegal block size: %d\n", blksiz);
				exit(1);
			}
			continue;

		case 'm':
			string = argv[2];
			++argv;
			maxdir = number(BIG);
			argc--;
			if ((maxdir < 64) || (tracks > 1024)) {
				printf("illegal value of m-flag: %d\n", maxdir);
				exit(1);
			}
			continue;

		case 'l':
			string = argv[2];
			++argv;
			seclth = number(BIG);
			argc--;
			if ((seclth < 128) || (tracks > 1024)) {
				printf("illegal sector length: %d\n", seclth);
				exit(1);
			}
			continue;

		case 'R':
			string = argv[2];
			++argv;
			restrk = number(BIG);
			argc--;
			if (restrk < 0) {
				printf("illegal # res'd tracks: %d\n", restrk);
				exit(1);
			}
			continue;

		case 't':
			string = argv[2];
			++argv;
			tracks = number(BIG);
			argc--;
			if (tracks < 0 || tracks > 1024) {
				printf("illegal # of tracks: %d\n", tracks);
				exit(1);
			}
			continue;

		case 'r':
			string = argv[2];
			++argv;
			sectrk = number(BIG);
			argc--;
			if (sectrk > 48) {
				printf("illegal r-flag: %d\n", sectrk);
				exit(1);
			}
			continue;

		case 'C':
			if (cflag)
				stat++;
			cpmname = argv[3];
			unixname = argv[2];
			argc -= 2;
			argv += 2;
			Cflag++;
			continue;

		case 'I':
			Iflag++;
			continue;

#ifdef DEBUG
		case 'x':
			xflag++;
			continue;
#endif

		default:
			printf("Illegal flag: -%c\n", argv[1][1]);
			stat++;
		}
		break;
	}
	if (stat > 0) {
	}
	/* 16-bit pointers are used if the number of blocks exceeds 255 */
	use16bitptrs = ((tracks - restrk) * sectrk * seclth) / blksiz > 255;

	if (argc <= 1 && iflag) {
		printf("cpm: can't copy from stdin in interactive mode\n");
		exit(1);
	} else {
		if (argc <= 1)
			fid = fileno(stdin);
		else {
			int ic, mode = O_RDWR;
			argv++;
		      again:
			if ((fid = open(*argv, mode)) == -1) {
				if (errno == ENOENT) {
					/*
					 * The specified input file does not exist,
					 * does the user want to initialize a new
					 * file?
					 */

					printf("Input file \"%s\" does not exist.\n", *argv);
					printf("Initialize new floppy file? (y/n)?");
					if ((ic = getchar()) != 'y' && ic != 'Y')
						exit(1);
					fid = initcpm(*argv);
					ic = getchar();	/* get <eol> */
				} else {
					if (mode == O_RDWR) {
						fprintf(stderr, "Cannot open input file \"%s\" read/write, trying read-only\n", *argv);
						mode = O_RDONLY;
						goto again;
					}
					perror("Cannot open input file");
					exit(1);
				}
			} else {
				if (Iflag) {
					printf("Initialize floppy file? (y/n)?");
					if ((ic = getchar()) != 'y' && ic != 'Y')
						exit(1);
					fid = initcpm(*argv);
					ic = getchar();	/* get <eol> */
				}
			}
		}
	}
	/* allocate memory for the directory buffer */
	if ((dirbuf = (struct directory *) malloc(maxdir * 32)) == NULL) {
		printf("can't allocate memory for directory\n");
		exit(1);
	}
	gen_sktab();
	getdir();
	build_bmap();
#ifdef DEBUG
	if (xflag > 0) {
		int i;
		char ch;

		dbmap("current bitmap:\n");
		for (i = (int) dirbuf; i < (int) dirbuf + maxdir * 32; i++) {
			ch = *(char *) i;
			putchar((int) ch);
		}
	}
#endif

	/* ignore broken pipes (could happen if someone aborts a PAGER) */
	signal(SIGPIPE, SIG_IGN);

	if (iflag > 0) {
		interact();
		exit(0);
	}
	if (dflag > 0)
		dispdir();
	if (cflag > 0) {
		if (cpmname == NULL || unixname == NULL)
			usage();
		copy(cpmname, unixname, Bflag);
		exit(0);
	}
	if (Cflag > 0) {
		if (cpmname == NULL || unixname == NULL)
			usage();
		pipc(unixname, cpmname, Bflag);
		exit(0);
	}
	if (tflag > 0) {
		if (cpmname == NULL)
			usage();
		copy(cpmname, NULL, 0);
		exit(0);
	}
}


/* Look at needed types carefully - uint16_t or uint32_t ?? */
int number(int big)
{
	register char *cs;
	long n;

	cs = string;
	n = 0;
	while (*cs >= '0' && *cs <= '9')
		n = n * 10 + *cs++ - '0';
	for (;;)
		switch (*cs++) {

		case 'k':
			n *= 1024;
			continue;

		case 'w':
			n *= 2;
			continue;

		case 'b':
			n *= 512;
			continue;

		case '\0':
			if (n >= big || n < 0) {
				fprintf(stderr, "number: argument %ld out of range\n", n);
				exit(1);
			}
			return n;
		}
}

void usage(void)
{
	printf("Usage: cpm [-i][-d][-p name][-c|C name1 name2] file-name\n");
	printf("Disk geometry options:\n");
	printf("       -s skew         [6]\n");
	printf("       -b block size   [1k]\n");
	printf("       -m max dir ents [64]\n");
	printf("       -l sec size     [128]\n");
	printf("       -r secs per trk [26]\n");
	printf("       -t tracks       [77]\n");
	printf("       -R resvd trks   [2]\n");
	exit(1);
}
