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

/* We should do skewing, but that makes writing the image fun, so for now
   don't bother */

#if 0
static int skew[2][18] = {
	{
	 0x06, 0x0C, 0x01, 0x07, 0x0D, 0x02, 0x08, 0x0E,
	 0x03, 0x09, 0x0F, 0x04, 0x0A, 0x10, 0x05, 0x0b,
	 0x11, 0x00},
	{
	 0x01, 0x05, 0x09, 0x02, 0x06, 0x0A, 0x03, 0x00, 0x07, 0x04}
};
#endif

static void jvc_writeheaders(int dd, int ntrack, int nside, int nsec,
			     uint8_t * ptr)
{
	int i, j, k;

	/* Write the headers so the data is a straight copy in with dd to an
	   offset */
	for (i = 0; i < ntrack; i++) {
		for (j = 0; j < nside; j++) {
			for (k = 0; k < nsec; k++) {
				*ptr++ = i;
				*ptr++ = k;	/*skew[dd][k] + 1; */
				*ptr++ = (0x80 * dd) | (0x10 * j);
			}
		}
	}
	/* Writeable */
	*ptr = 1;
}


static void jvcwritesectors(int infd, int outfd, int ntrack, int nside,
			    int nsec)
{
	int i, j, k;
	static char buf[256];
	for (i = 0; i < ntrack; i++) {
		for (j = 0; j < nside; j++) {
			for (k = 1; k <= nsec; k++) {
				/* 0 itself is fine - we just blank the rest */
				if (read(infd, buf, 256) < 0) {
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

static char hdrbuf[8704];

static void usage(void)
{
	fprintf(stderr,
		"makejv3: example types dd40, dd80, dd40s, dd80s, sd40, sd80, sd40s, sd80s\n");
	exit(1);
}

void main(int argc, char *argv[])
{
	int infd, outfd;
	int nside = 2, dd = 1;
	char *p = argv[1];
	int tracks = 0;

	if (argc != 4) {
		fprintf(stderr, "%s [type] [rawfs] [jv3file]\n", argv[0]);
		exit(1);
	}

	if ((*p != 's' && *p != 'd') || p[1] != 'd')
		usage();
	if (*p == 's')
		dd = 0;
	p += 2;
	while (isdigit(*p)) {
		tracks = tracks * 10 + (*p - '0');
		p++;
	}
	if (*p == 's') {
		nside = 1;
		p++;
	}
	if (*p)
		usage();

	infd = open(argv[2], O_RDONLY);
	if (infd == -1) {
		perror(argv[2]);
		exit(1);
	}
	outfd = open(argv[3], O_WRONLY | O_TRUNC | O_CREAT, 0666);
	if (outfd == -1) {
		perror(argv[3]);
		exit(1);
	}
	jvc_writeheaders(dd, tracks, nside, dd ? 18 : 10, hdrbuf);
	if (write(outfd, hdrbuf, 8704) != 8704) {
		perror(argv[3]);
		exit(1);
	}
	jvcwritesectors(infd, outfd, tracks, nside, dd ? 18 : 10);
	close(infd);
	close(outfd);
	exit(0);
}
