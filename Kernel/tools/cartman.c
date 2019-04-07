#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/* This code has probably killed Kenny.... */

static unsigned char buf[65536];
static unsigned char out[65536];

static unsigned int s__CODE, s__DISCARD, s__INITIALIZER, s__DATA,
    s__INITIALIZED, s__INITIALIZER, s__COMMONMEM, l__INITIALIZED,
    l__COMMONMEM, l__CODE, l__DATA, l__DISCARD;


static void ProcessMap(FILE * fp)
{
	char buf[512];

	while (fgets(buf, 511, fp)) {
		char *p1 = strtok(buf, " \t\n");
		char *p2 = NULL;

		if (p1)
			p2 = strtok(NULL, " \t\n");

		if (p1 == NULL || p2 == NULL)
			continue;

		if (strcmp(p2, "s__CODE") == 0)
			sscanf(p1, "%x", &s__CODE);
		if (strcmp(p2, "l__CODE") == 0)
			sscanf(p1, "%x", &l__CODE);
		if (strcmp(p2, "s__DATA") == 0)
			sscanf(p1, "%x", &s__DATA);
		if (strcmp(p2, "l__DATA") == 0)
			sscanf(p1, "%x", &l__DATA);
		if (strcmp(p2, "s__INITIALIZED") == 0)
			sscanf(p1, "%x", &s__INITIALIZED);
		if (strcmp(p2, "s__INITIALIZER") == 0)
			sscanf(p1, "%x", &s__INITIALIZER);
		if (strcmp(p2, "l__INITIALIZED") == 0)
			sscanf(p1, "%x", &l__INITIALIZED);
		if (strcmp(p2, "s__COMMONMEM") == 0)
			sscanf(p1, "%x", &s__COMMONMEM);
		if (strcmp(p2, "l__COMMONMEM") == 0)
			sscanf(p1, "%x", &l__COMMONMEM);
		if (strcmp(p2, "s__DISCARD") == 0)
			sscanf(p1, "%x", &s__DISCARD);
		if (strcmp(p2, "l__DISCARD") == 0)
			sscanf(p1, "%x", &l__DISCARD);
	}
}


int main(int argc, char *argv[])
{
	FILE *map, *bin;
	int tail = 0;
	int start;
	unsigned int end = 0;
	int reloc = 0;
	int pack_discard = 0;
	int no_pack = 0;
	int base;

	if (argc != 4) {
		fprintf(stderr, "%s: [binary] [map] [output]\n", argv[0]);
		exit(1);
	}
	bin = fopen(argv[1], "r");
	if (bin == NULL) {
		perror(argv[1]);
		exit(1);
	}
	if (fread(buf, 1, 65536, bin) == 0) {
		fprintf(stderr, "%s: read error on %s\n", argv[0],
			argv[1]);
		exit(1);
	}
	fclose(bin);

        /* Start with the output matching the input */
	memcpy(out, buf, 65536);

	map = fopen(argv[2], "r");
	if (map == NULL) {
		perror(argv[2]);
		exit(1);
	}
	ProcessMap(map);
	fclose(map);

	/* The packing for an MSX cartridge is not dissimilar to the usual
	   but has different constaints and because it's ROM we actually
	   have to pack difering bits */

        printf("Scanning data from 0x%x to 0x%x\n",
                s__DATA, s__DATA + l__DATA);
	base = s__DATA;
	while (base < s__DATA + l__DATA) {
	        if (buf[base] && buf[base] != 0xFF) {
	                fprintf(stderr, "0x%04x:0x%02x\n",
	                        base, (unsigned char)buf[base]);
                }
                base++;
        }
	memset(out + s__DATA, 0, l__DATA);

	/* Our standard layout begins with the code */
	start = s__CODE;

	if (s__CODE != 0 || l__CODE > 0x4000) {
		fprintf(stderr, "Segment CODE must start at 0 and be under 16K long (%04X %04X).\n",
			s__CODE, l__CODE);
		exit(1);
	}
	/* Our prepared image ends with initialized ready to copy from ROM so
	   put the right bits in place */
	memcpy(out + s__INITIALIZED, buf + s__INITIALIZER, l__INITIALIZED);

	end = s__DISCARD + l__DISCARD;
	printf("Image ends at %04X\n", end);

	if (end >= 0xFC80) {
		fprintf(stderr, "Image does not fit with bootstrap.\n");
		exit(1);
	}

	bin = fopen(argv[3], "w");
	if (bin == NULL) {
		perror(argv[3]);
		exit(1);
	}
	if (fwrite(out, 65536, 1, bin) != 1) {
		perror(argv[3]);
		exit(1);
	}
	fclose(bin);
	exit(0);
}

