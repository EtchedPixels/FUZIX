#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>
#include <arpa/inet.h>

static int err = 0;
static int head = 0;

static unsigned int bufpair(int bigend, unsigned char *p, int n)
{
	if (bigend) {
		/* mc6809, big endian */
		return (p[n] << 8) | p[n + 1];
	}
	return p[n] | (p[n + 1] << 8);
}

#if defined(__m68k__)
static void size_binflat(char *name, uint8_t * buf)
{
	uint32_t *hdr = (uint32_t *) buf;
	uint32_t txtsz, datsz, bsssz, stksz;

	if (ntohl(hdr[1]) != 4) {
		fprintf(stderr, "%s: unknown binflat revision %d.\n", name, hdr[1]);
		err = 1;
		return;
	}
	txtsz = ntohl(hdr[3]);	/* Start of data, end of text */
	datsz = ntohl(hdr[4]);	/* End of data, start of bss */
	bsssz = ntohl(hdr[5]);	/* End of bss */
	bsssz -= datsz;		/* Get the BSS size */
	datsz -= txtsz;		/* And data size */

	stksz = ntohl(hdr[6]);

	if (head != 2) {
		if (head == 1)
			printf("\n");
		printf("   text    data     bss    stack   size     hex filename\n");
		head = 2;
	}

	printf("%7d %7d %7d %7d %7d %7x %s\n",
		txtsz, datsz, bsssz, stksz,
		txtsz + datsz + bsssz + stksz,
		txtsz + datsz + bsssz + stksz,
		name);
}
#endif

static void size_fzx1(char *name, uint8_t * buf)
{
	unsigned int basepage = 0;
	int endian;
	unsigned int txtsz, datsz, bsssz;

	if (buf[0] == 0xC3 || buf[0] == 0x18 || buf[0] == 0x4C || buf[0] == 0x38) {
		/* Z-80 or 6502 */
		endian = 0;
		basepage = buf[7] << 8;
	} else if (buf[0] == 0x7E || buf[0] == 0x20) {
		/* 6809 */
		endian = 1;
	} else {
		endian = -1;
	}
	if (endian == -1) {
		fprintf(stderr, "%s: not a known FZX1 platform.\n", name);
		err = 1;
		return;
	}
	/* Text, data, BSS */
	txtsz = bufpair(endian, buf, 10);
	datsz = bufpair(endian, buf, 12);
	bsssz = bufpair(endian, buf, 14);

	if (head != 1) {
		if (head == 2)
			printf("\n");
		printf(" base text data  bss   size  hex filename\n");
		head = 1;
	}
	printf("%5x%5x%5x%5x%7d%5x %s\n", basepage, txtsz, datsz, bsssz, txtsz + datsz + bsssz, txtsz + datsz + bsssz, name);
}

int main(int argc, char *argv[])
{
	FILE *fp;
	unsigned char buf[32];
	int n;

	if (argc < 2) {
		fprintf(stderr, "%s [executable...]\n", argv[0]);
		exit(1);
	}


	for (n = 1; n < argc; n++) {
		fp = fopen(argv[n], "r");
		if (fp == NULL) {
			perror(argv[n]);
			exit(1);
		}
		if (fread(buf, 32, 1, fp) != 1) {
			fprintf(stderr, "%s: too short ?\n", argv[0]);
			exit(1);
		}
		fclose(fp);

		if (memcmp(buf + 3, "FZX1", 4) == 0 ||
				memcmp(buf + 3, "FZL1", 4) == 0)
			size_fzx1(argv[n], buf);
#if defined(__m68k__)
		else if (memcmp(buf, "bFLT", 4) == 0)
			size_binflat(argv[n], buf);
#endif
		else
			fprintf(stderr, "%s: not a Fuzix binary format.\n", argv[n]);
	}
	exit(err);
}
