#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>
#include <arpa/inet.h>

static void set_hint_flat(FILE * fp, unsigned char *buf, int argc, char *argv[])
{
	fprintf(stderr, "%s: set hint not supported on flat binaries.\n", argv[0]);
	exit(1);
}

static void set_hint_fzx2(FILE * fp, unsigned char *buf, int argc, char *argv[])
{
	uint32_t v;
	unsigned short top;

	if (argc == 2) {
		printf("Fuzix binary hints %02X bytes.\n", buf[5]);
		return;
	}

	if (sscanf(argv[2], "%u", &v) != 1 || v > 255) {
		fprintf(stderr, "%s: invalid set_hint value '%s'.\n", argv[0], argv[2]);
		exit(1);
	}
	buf[5] = v;
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
		fprintf(stderr, "%s [executable] {hintcode}\n", argv[0]);
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
		set_hint_fzx2(fp, buf, argc, argv);
	/* Little endian */
	else if (*buf == 0xA8 && buf[1] == 0x80)
		set_hint_fzx2(fp, buf, argc, argv);
	/* 32bit flat */
	else if (memcmp(buf, "bFLT", 4) == 0)
		set_hint_flat(fp, buf, argc, argv);
	else {
		fprintf(stderr, "%s: not a Fuzix binary format.\n", argv[1]);
		exit(1);
	}
	fclose(fp);
	exit(0);
}
