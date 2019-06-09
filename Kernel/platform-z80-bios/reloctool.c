#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/*
 *	Turn a pair of images into a reloctable output
 *	We have some hardcode rules
 *	- F000+ is common
 *	- EFFF or earlier is not
 */

static uint8_t buf[65536];
static uint8_t bufb[65536];

static unsigned int s__INITIALIZER, s__INITIALIZED;
static unsigned int l__INITIALIZER;

static unsigned int s__DATA, s__END;

static unsigned int progload = 0x100;

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

		if (strcmp(p2, "s__DATA") == 0)
			sscanf(p1, "%x", &s__DATA);
		if (strcmp(p2, "s__END") == 0)
			sscanf(p1, "%x", &s__END);
		if (strcmp(p2, "s__INITIALIZED") == 0)
			sscanf(p1, "%x", &s__INITIALIZED);
		if (strcmp(p2, "s__INITIALIZER") == 0)
			sscanf(p1, "%x", &s__INITIALIZER);
		if (strcmp(p2, "l__INITIALIZER") == 0)
			sscanf(p1, "%x", &l__INITIALIZER);
	}
}

static FILE *outf;
static uint8_t lastbyte;
static uint8_t islb;
static uint8_t stuffb;
static uint8_t stuffc;

static void stuffbit(uint8_t n)
{
	if (n > 1) {
		fprintf(stderr, "bad stuffbit.\n");
		exit(1);
	}
	stuffc <<= 1;
	stuffc |= n;
	if (++stuffb == 8) {
		stuffb = 0;
		fputc(stuffc, outf);
	}
}

static void stuffhex(uint8_t c)
{
	int i;
	for (i = 0; i < 4; i++) {
		stuffbit((c & 0x08) ? 1 : 0);
		c <<= 1;
	}
}

static void stuffbyte(uint8_t c)
{
	int i;
	for (i = 0; i < 8; i++) {
		stuffbit((c & 0x80) ? 1 : 0);
		c <<= 1;
	}
}

static void stuffword(uint16_t c)
{
	int i;
	for (i = 0; i < 16; i++) {
		stuffbit((c & 0x8000) ? 1 : 0);
		c <<= 1;
	}
}

static void stuffend(void)
{
	while(stuffb)
		stuffbit(1);
}

static void byte_unreloc(uint16_t addr, uint8_t c)
{
	if (islb) {
		stuffbit(0);
		stuffbyte(lastbyte);
	}
	islb = 1;
	lastbyte = c;	
}

static void relocate_flush(void)
{
	if (islb) {
		stuffbit(0);
		stuffbyte(lastbyte);
	}
	islb = 0;
}

static void relocrecord(uint16_t addr, int type, uint8_t hb)
{
	uint16_t rw;
	if (!islb) {
		fprintf(stderr, "%04X: unable to resolve 16 bit address.\n",
							addr);
		exit(1);
	}
	rw = hb << 8 | lastbyte;
	islb = 0;
	/* 1 introduces a relocation */
	stuffbit(1);
	/* The type 00 - abs 01 program rel 10 data rel 11 common rel */
	/* We use code / data for program / common as CP/M 3 does */
	stuffbit((type >> 1) & 1);
	stuffbit((type & 1));
	stuffword(rw);
}	
	
static void relocate_common(uint16_t addr, uint8_t * p)
{
	relocrecord(addr, 2, *p);
}

static void relocate_code(uint16_t addr, uint8_t * p)
{
	relocrecord(addr, 1, *p);
}

static void relocate_begin(uint16_t addr, int len)
{
	/* Link item */
	stuffbit(1);
	/* Special record */
	stuffbit(0);
	stuffbit(0);
	/* Name */
	stuffhex(2);
	/* 5 byte name */
	stuffbit(1);
	stuffbit(0);
	stuffbit(1);
	stuffbyte('F');
	stuffbyte('U');
	stuffbyte('Z');
	stuffbyte('I');
	stuffbyte('X');
	/* Link item */
	stuffbit(1);
	/* Special record */
	stuffbit(0);
	stuffbit(0);
	/* Program segment size */
	stuffhex(13);
	stuffbit(0);
	stuffbit(0);
	stuffword(s__DATA); 
	/* Link item */
	stuffbit(1);
	/* Special record */
	stuffbit(0);
	stuffbit(0);
	/* Data segment size */
	stuffhex(10);
	stuffbit(1);
	stuffbit(0);
	stuffword(s__END - 0xF000); 
}

static void relocate_end(uint16_t addr)
{
	/* Write out any last data byte */
	relocate_flush();
	/* Link item */
	stuffbit(1);
	/* Special record */
	stuffbit(0);
	stuffbit(0);
	/* End of module */
	stuffhex(14);
	/* No start address */
	stuffbit(0);
	stuffbit(0);
	stuffword(0);
	stuffend();
	/* Link item */
	stuffbit(1);
	/* Special record */
	stuffbit(0);
	stuffbit(0);
	/* End of file */
	stuffhex(15);
	
}

static void end_relocations(void)
{
	stuffend();
}

/* Generate relocation information for a given block */
static void gen_relocations(uint16_t addr, int len)
{
	uint8_t *base = buf + addr;
	uint8_t *base2 = bufb + addr;

	fprintf(stderr, "Gen relocations %04X to %04X\n", addr, addr + len - 1);
	relocate_begin(addr, len);
	while (len > 0) {
		if (*base == *base2) {
			byte_unreloc(addr++, *base++);
			len--;
			base2++;
			continue;
		}
		if (*base2 != *base + 1) {
			fprintf(stderr,
				"Invalid relocation at %04X (%02X v %02X)\n",
				addr + len, *base, *base2);
			exit(1);
		}
		if (*base >= 0xF0)
			relocate_common(addr, base);
		else
			relocate_code(addr, base);
		len--;
		addr++;
		base++;
		base2++;
	}

	relocate_end(addr);
}

void generate_relocatable(FILE *fp)
{
	outf = fp;
	fprintf(stderr, "Processing main block.\n");
	gen_relocations(0, s__DATA);
	fprintf(stderr, "Processing common block.\n");
	gen_relocations(0xF000, s__END - 0xF000);
	end_relocations();
}

int main(int argc, char *argv[])
{
	FILE *map, *bin;
	uint8_t *bp;
	/* Our primary binary is at 0x0100 but we reloc it down without shifting
	   in the buffer */
	static uint16_t progload = 0x0100;

	if (argc != 5) {
		fprintf(stderr, "%s: <binary1> <binary2> <map> <output>\n",
			argv[0]);
		exit(1);
	}

	bin = fopen(argv[1], "r");
	if (bin == NULL) {
		perror(argv[1]);
		exit(1);
	}
	/* Truely aligned */
	if (fread(buf + 0x100, 1, 0xFF00, bin) == 0) {
		fprintf(stderr, "%s: read error on %s\n", argv[0],
			argv[1]);
		exit(1);
	}
	fclose(bin);
	bin = fopen(argv[2], "r");
	if (bin == NULL) {
		perror(argv[2]);
		exit(1);
	}
	/* Offset for link analysis */
	if (fread(bufb + 0x100, 1, 0xFF00, bin) == 0) {
		fprintf(stderr, "%s: read error on %s\n", argv[0],
			argv[2]);
		exit(1);
	}
	fclose(bin);
	map = fopen(argv[3], "r");
	if (map == NULL) {
		perror(argv[3]);
		exit(1);
	}
	ProcessMap(map);
	fclose(map);

	bin = fopen(argv[4], "w");
	if (bin == NULL) {
		perror(argv[4]);
		exit(1);
	}

	if (s__INITIALIZER + l__INITIALIZER > 65535
	    || s__INITIALIZED + l__INITIALIZER > 65535
	    || s__DATA > 65535) {
		fprintf(stderr, "%s: too large.\n", argv[0]);
		exit(1);
	}
	memcpy(buf + s__INITIALIZED, buf + s__INITIALIZER, l__INITIALIZER);
	memcpy(bufb + s__INITIALIZED,
	       bufb + s__INITIALIZER, l__INITIALIZER);

	generate_relocatable(bin);

	fclose(bin);
	exit(0);
}
