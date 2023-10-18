#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static uint8_t use[256];

struct section {
	uint16_t base;
	uint16_t size;
};

struct section sect[14];

static char sectname[14] = "ACDBXZSLsb";

static void check_overlap(int a, int b)
{
	/* A finishes below B start -> OK */
	if (sect[a].base + sect[a].size <= sect[b].base)
		return;
	/* B finishes before A start -> OK */
	if (sect[b].base + sect[b].size <= sect[a].base)
		return;
	fprintf(stderr, "Section %c (%04X-%04X) overlaps section %c (%04X-%04X).\n",
		sectname[a],
		sect[a].base, sect[a].base + sect[a].size - 1,
		sectname[b],
		sect[b].base, sect[b].base + sect[b].size - 1);
}

static void add_segment(int i)
{
	memset(use + (sect[i].base >> 8),
		sectname[i], (sect[i].size + 255) >> 8);
}

static void mark_map(void)
{
	int i, j;

	for (i = 1; i < 10; i++) {
		if ((unsigned)sect[i].base + sect[i].size > 0xFFFF) {
			fprintf(stderr, "Section %c runs over the top of memory.\n",
				sectname[i], sect[i].base + sect[i].size - 1);
		}
		for (j = 1; j < i; j++)
			check_overlap(i, j);
		add_segment(i);
	}
}

int main(int argc, char *argv[])
{
	char buf[512];
	int i;
	int r, b;

	memset(use, '#', sizeof(use));

	while (fgets(buf, 511, stdin)) {
		char *val = strtok(buf, " ");
		char *type = strtok(NULL, " ");
		char *name = strtok(NULL, " \n");

		unsigned int addr;

		sscanf(val, "%x", &addr);

		if (strcmp(name, "__code_size") == 0)
			sect[1].size = addr;

		if (strcmp(name, "__data_size") == 0)
			sect[2].size = addr;

		if (strcmp(name, "__bss_size") == 0)
			sect[3].size = addr;

		if (strcmp(name, "__discard_size") == 0)
			sect[4].size = addr;

		if (strcmp(name, "__zp_size") == 0)
			sect[5].size = addr;

		if (strcmp(name, "__common_size") == 0)
			sect[6].size = addr;

		if (strcmp(name, "__literal_size") == 0)
			sect[7].size = addr;

		if (strcmp(name, "__commondata_size") == 0)
			sect[8].size = addr;

		if (strcmp(name, "__buffers_size") == 0)
			sect[9].size = addr;

		if (strcmp(name, "__code") == 0)
			sect[1].base = addr;

		if (strcmp(name, "__data") == 0)
			sect[2].base = addr;

		if (strcmp(name, "__bss") == 0)
			sect[3].base = addr;

		if (strcmp(name, "__discard") == 0)
			sect[4].base = addr;

		if (strcmp(name, "__zp") == 0)
			sect[5].base = addr;

		if (strcmp(name, "__common") == 0)
			sect[6].base = addr;

                if (strcmp(name, "__literal") == 0)
			sect[7].base = addr;

                if (strcmp(name, "__commondata") == 0)
			sect[8].base = addr;

                if (strcmp(name, "__buffers") == 0)
			sect[9].base = addr;
	}
	mark_map();
	for (r = 0; r < 4; r++) {
		for (i = 0; i < 256; i += 4) {
			putchar(use[i + r]);
			if ((i & 0x3C) == 0x3C)
				putchar(' ');
		}
		putchar('\n');
	}
	putchar('\n');
	exit(0);
}
