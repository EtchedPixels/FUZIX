/*
 *	Make a jv3 file. At the moment we don't support taking a data
 *	file and writing it mashed up to match a skewed disk. So you can
 *	either have a skewed empty disk, or a non skewed data one.
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>


/* JV3 consists of a bunch of byte packed misaligned fields in an array,
   followed by a writeprot byte and then the data, for 2901 sectors. If
   it exceeds 2091 sectors then this repeats.
   
   One of the weirder disk formats emulation folk use */

/* FIXME: need a skew table for PC style media */
static const int skew[2][18] = {
	{
		0x01, 0x08, 0x05, 0x02, 0x09, 0x6, 0x03, 0x00,
		0x07, 0x04
	},
	{
		0x06, 0x0C, 0x01, 0x07, 0x0D, 0x02, 0x08, 0x0E,
		0x03, 0x09, 0x0F, 0x04, 0x0A, 0x10, 0x05, 0x0b,
		0x11, 0x00
	},
};

static int skewed;
static int infd;
static int outfd;
static int ntrack;
static int nside = 2;
static int nsec;
static int ddens;
static int data;

static void jvc_writeheaders(uint8_t * ptr)
{
	int i, j, k;

	/* Write the headers so the data is a straight copy in with dd to an
	   offset */
	for (i = 0; i < ntrack; i++) {
		for (j = 0; j < nside; j++) {
			for (k = 0; k < nsec; k++) {
				*ptr++ = i;
				/* DD media start at sector 1 like sane computers */
				*ptr++ = ddens + (skewed == 0 ? k: skew[ddens][k]);
				*ptr++ = (0x80 * ddens) | (0x10 * j);
			}
		}
	}
	/* Writeable */
	*ptr = 1;
}


static void jvc_pc_writeheaders(uint8_t * ptr)
{
	int i, j, k;

	for (i = 0; i < ntrack; i++) {
		for (j = 0; j < nside; j++) {
			for (k = 1; k <= nsec; k++) {
				*ptr++ = i;
				*ptr++ =
				    skewed == 0 ? k : skew[1][k - 1] + 1;
				/* 512 byte sectors */
				*ptr++ = 0x83 | (0x10 * j);
			}
		}
	}
	/* Writeable */
	*ptr = 1;
}

static char buf[512];

static void jvc_writesectors(void)
{
	int i, j, k;
	for (i = 0; i < ntrack; i++) {
		for (j = 0; j < nside; j++) {
			for (k = 0; k < nsec; k++) {
				/* 0 relative sector */
				int s = skewed == 0 ? k: skew[ddens][k];
				lseek(infd, 256 * (s + nsec * j + (nsec * nside) * i), 0);
				/* 0 itself is fine - we just blank the rest */
				if (data && read(infd, buf, 256) < 0) {
					perror("read sectors");
					exit(1);
				}
				if (write(outfd, buf, 256) != 256) {
					perror("write jvc3");
					exit(1);
				}
			}
		}
	}
}

static void jvc_pc_writesectors(void)
{
	int i, j, k;
	for (i = 0; i < ntrack; i++) {
		for (j = 0; j < nside; j++) {
			for (k = 0; k < nsec; k++) {
				/* 0 relative sector */
				int s = skewed == 0 ? k: skew[ddens][k];
				lseek(infd, 512 * (s + nsec * j + (nsec * nside) * k), 0);
				/* 0 itself is fine - we just blank the rest */
				if (data && read(infd, buf, 512) < 0) {
					perror("read sectors");
					exit(1);
				}
				if (write(outfd, buf, 512) != 512) {
					perror("write jvc3");
					exit(1);
				}
			}
		}
	}
}


static char hdrbuf[8704];

static void usage(void)
{
	fprintf(stderr,
		"makejv3: example types dd40, dd80, dd40s, pcdd40s, dd80s, sd40, sd80, sd40s, sd80s\n");
	exit(1);
}

void main(int argc, char *argv[])
{
	int pc = 0;
	char *p = NULL;
	char *in = NULL;
	int tracks = 0;
	int opt;

	while ((opt = getopt(argc, argv, "t:d:ms")) != -1) {
		switch (opt) {
		case 't':
			p = optarg;
			break;
		case 'd':
			in = optarg;
			break;
		case 'm':
			pc = 1;
			ddens = 1;
			break;
		case 's':
			skewed = 1;
			break;
		default:
			usage();
		}
	}
	if (p == NULL || optind != argc - 1)
		usage();

	if ((*p != 's' && *p != 'd') || p[1] != 'd')
		usage();
	if (*p == 'd')
		ddens = 1;
	p += 2;
	while (isdigit(*p)) {
		ntrack = ntrack * 10 + (*p - '0');
		p++;
	}
	if (*p == 's') {
		nside = 1;
		p++;
	}
	if (*p)
		usage();

	if (in) {
		infd = open(in, O_RDONLY);
		if (infd == -1) {
			perror(in);
			exit(1);
		}
		data = 1;
	}
	outfd = open(argv[optind], O_WRONLY | O_TRUNC | O_CREAT, 0666);
	if (outfd == -1) {
		perror(argv[3]);
		exit(1);
	}
	if (pc) {
		if (!ddens)
			usage();
		nsec = 9;
		jvc_pc_writeheaders(hdrbuf);
	} else {
		nsec = ddens ? 18 : 10;
		jvc_writeheaders(hdrbuf);
	}
	if (write(outfd, hdrbuf, 8704) != 8704) {
		perror(argv[3]);
		exit(1);
	}
	if (pc)
		jvc_pc_writesectors();
	else
		jvc_writesectors();
	close(infd);
	close(outfd);
	exit(0);
}
