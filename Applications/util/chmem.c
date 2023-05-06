#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#if defined(__m68k__)

static void chmem_flat(FILE * fp, unsigned char *buf, int argc, char *argv[])
{
	uint32_t *hdr = (uint32_t *) buf;
	uint32_t top;
	unsigned int v;

	if (ntohl(hdr[1]) != 4) {
		fprintf(stderr, "%s: unsupported revision %d.\n", ntohl(hdr[1]));
		exit(1);
	}
	if (argc == 2) {
		top = ntohl(hdr[6]);
		if (top)
			printf("Flat binary set at %d bytes.\n", top);
		else
			printf("Flat binary, set to allocate all available.\n");
		return;
	}

	if (sscanf(argv[2], "%u", &v) != 1) {
		fprintf(stderr, "%s: invalid chmem value '%s'.\n", argv[0], argv[2]);
		exit(1);
	}
	hdr[6] = htonl(v);
	rewind(fp);
	if (fwrite(buf, 32, 1, fp) != 1) {
		fprintf(stderr, "%s: write error.\n", argv[0]);
		exit(1);
	}
}
#endif

static void chmem_fzx2(FILE * fp, unsigned char *buf, int argc, char *argv[])
{
	unsigned long int v;
	unsigned short top;

	if (argc == 2) {
		top = buf[13] << 8;
		if (top)
			printf("Fuzix binary set at %u bytes.\n", top);
		else
			printf("Fuzix binary, set to allocate all available.\n");
		return;
	}

	if (sscanf(argv[2], "%lu", &v) != 1 || v > 65536) {
		fprintf(stderr, "%s: invalid chmem value '%s'.\n", argv[0], argv[2]);
		exit(1);
	}
	v += 255;
	buf[13] = v >> 8;
	rewind(fp);
	if (fwrite(buf, 16, 1, fp) != 1) {
		fprintf(stderr, "%s: write error.\n", argv[0]);
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	FILE *fp;
	unsigned char buf[32];

	if (argc != 2 && argc != 3) {
		fprintf(stderr, "%s [executable] {size}\n", argv[0]);
		exit(1);
	}
	if (argc == 2)
		fp = fopen(argv[1], "r");
	else
		fp = fopen(argv[1], "r+");
	if (fp == NULL) {
		perror(argv[1]);
		exit(1);
	}
	if (fread(buf, 32, 1, fp) != 1) {
		fprintf(stderr, "%s: too short ?\n", argv[0]);
		exit(1);
	}
	/* Big endian */
	if (*buf == 0x80 && buf[1] == 0xA8)
		chmem_fzx2(fp, buf, argc, argv);
	/* Little endian */
	else if (*buf == 0xA8 && buf[1] == 0x80)
		chmem_fzx2(fp, buf, argc, argv);
#if defined(__m68k__)
	else if (memcmp(buf, "bFLT", 4) == 0)
		chmem_flat(fp, buf, argc, argv);
#endif
	else {
		fprintf(stderr, "%s: not a Fuzix binary format.\n", argv[1]);
		exit(1);
	}
	fclose(fp);
	exit(0);
}
