#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static uint8_t use[10][256];

static int banked = 0;
static int maxbank = 0;

struct section {
	struct section *next;
	char name[64];
	char code;
	unsigned int start;
	unsigned int len;
	int flags;
#define KNOW_START	1
#define KNOW_SIZE	2
};

static struct section *head;

static struct section *find_create(const char *name)
{
	struct section *s = head;
	while (s) {
		if (strcmp(s->name, name) == 0)
			return s;
		s = s->next;
	}
	if (strcmp(name, "STUBS") == 0)
		banked = 1;
	s = malloc(sizeof(struct section));
	if (s == NULL) {
		fputs("Out of memory.\n", stderr);
		exit(1);
	}
	s->flags = 0;
	strncpy(s->name, name, 64);
	s->name[63] = 0;
	s->next = head;
	head = s;
	return s;
}

static char code_for(const char *name)
{
	if (strcmp(name, "BOOT") == 0)
		return '!';
	if (strcmp(name, "BOOT0") == 0)
		return '!';
	if (strcmp(name, "HEADER") == 0)
		return '!';
	if (strcmp(name, "CODE") == 0)
		return '0';
	if (strcmp(name, "CODE1") == 0)
		return '1';
	if (strcmp(name, "CODE2") == 0)
		return '2';
	if (strcmp(name, "CODE3") == 0)
		return '3';
	if (strcmp(name, "CODE4") == 0)
		return '4';
	if (strcmp(name, "CODE5") == 0)
		return '5';
	if (strcmp(name, "CODE6") == 0)
		return '6';
	if (strcmp(name, "VECTORS") == 0)
		return 'v';
	if (strcmp(name, "VIDEO") == 0)
		return 'V';
	if (strcmp(name, "FONT") == 0)
		return 'F';
	if (strcmp(name, "FONTCOMMON") == 0)
		return 'F';
	if (strcmp(name, "INITIALIZED") == 0)
		return 'I';
	if (strcmp(name, "HOME") == 0)
		return 'H';
	if (strcmp(name, "DISCARD") == 0)
		return 'X';
	if (strcmp(name, "DISCARD1") == 0)
		return 'X';
	if (strcmp(name, "DISCARD2") == 0)
		return 'X';
	if (strcmp(name, "DATA") == 0)
		return 'D';
	if (strcmp(name, "BUFFERS") == 0)
		return 'B';
	if (strcmp(name, "BUFFERS1") == 0)
		return 'B';
	if (strcmp(name, "BUFFERS2") == 0)
		return 'B';
	if (strcmp(name, "COMMONMEM") == 0)
		return 'S';
	if (strcmp(name, "COMMONDATA") == 0)
		return 's';
	if (strcmp(name, "CONST") == 0)
		return 'C';
	if (strcmp(name, "INITIALIZER") == 0)
		return 'i';
	if (strcmp(name, "STUBS") == 0)
		return '@';
	if (strcmp(name, "UDATA") == 0)
		return 'U';
	return '?';
}

static char bank_for(const char *name)
{
	if (!banked)
		return 0;
	/* Really this is very system dependant */
	if (strcmp(name, "BOOT") == 0)
		return 0;
	if (strcmp(name, "BOOT0") == 0)
		return 0;
	if (strcmp(name, "HEADER") == 0)
		return 0;
	if (strcmp(name, "CODE") == 0)
		return 0;
	if (strcmp(name, "DATA") == 0)
		return 0;
	if (strncmp(name, "CODE", 4) == 0 || strncmp(name, "DATA", 4) == 0)
		return name[4] - '0';
	if (strcmp(name, "VECTORS") == 0)
		return 0;
	if (strcmp(name, "VIDEO") == 0)
		return 3;
	if (strcmp(name, "FONT") == 0)
		return 3;
	if (strcmp(name, "FONTCOMMON") == 0)
		return 0;
	if (strcmp(name, "INITIALIZED") == 0)
		return 0;
	if (strcmp(name, "HOME") == 0)
		return 1;
	if (strcmp(name, "DISCARD") == 0)
		return 0;
	if (strcmp(name, "DISCARD1") == 0)
		return 1;
	if (strcmp(name, "DISCARD2") == 0)
		return 2;
	if (strcmp(name, "BUFFERS") == 0)
		return 0;
	if (strcmp(name, "BUFFERS1") == 0)
		return 1;
	if (strcmp(name, "BUFFERS2") == 0)
		return 2;
	if (strcmp(name, "COMMONMEM") == 0)
		return 0;
	if (strcmp(name, "COMMONDATA") == 0)
		return 0;
	if (strcmp(name, "CONST") == 0)
		return 0;
	if (strcmp(name, "INITIALIZER") == 0)
		return 0;
	if (strcmp(name, "STUBS") == 0)
		return 0;
	if (strcmp(name, "UDATA") == 0)
		return 0;
	fprintf(stderr, "Unknown bank '%s'.\n", name);
	exit(1);
}

static void learn_size(const char *size, const char *name)
{
	struct section *s = find_create(name);
	if (sscanf(size, "%8X", &s->len) != 1) {
		fprintf(stderr, "Invalid size '%s'.\n", size);
		exit(1);
	}
	s->flags |= KNOW_SIZE;
}

static void learn_start(const char *addr, const char *name)
{
	struct section *s = find_create(name);
	if (sscanf(addr, "%8X", &s->start) != 1) {
		fprintf(stderr, "Invalid start '%s'.\n", addr);
		exit(1);
	}
	s->flags |= KNOW_START;
}

static void mark_map(void)
{
	unsigned int base, end, bank;
	struct section *s = head;
	while (s) {
		if (s->flags != (KNOW_SIZE | KNOW_START)) {
			fprintf(stderr, "Incomplete section '%s'.\n",
				s->name);
			exit(1);
		}
		if (s->start + s->len > 0x10000) {
			fprintf(stderr, "Section '%s' overruns memory.\n",
				s->name);
			exit(1);
		}
		if (s->len) {
			s->code = code_for(s->name);
			base = (s->start + 127) >> 8;
			end = (s->start + s->len + 127) >> 8;
			bank = bank_for(s->name);
			if (bank > 9) {
				fprintf(stderr, "Invalid bank %d for '%s'.\n", bank, s->name);
				exit(1);
			}
			if (bank > maxbank)
				maxbank = bank;
			while (base <= end) {
				use[bank][base++] = s->code;
			}
		}
		s = s->next;
	}
}

int main(int argc, char *argv[])
{
	char buf[512];
	int i;
	int r, b;

	memset(use, '#', sizeof(use));

	while (fgets(buf, 511, stdin)) {
		char *p1 = strtok(buf, " \t\n");
		char *p2 = NULL;

		if (p1)
			p2 = strtok(NULL, " \t\n");

		if (p1 == NULL || p2 == NULL)
			continue;

		if (strncmp(p2, "l__", 3) == 0)
			learn_size(p1, p2 + 3);
		if (strncmp(p2, "s__", 3) == 0)
			learn_start(p1, p2 + 3);
	}
	mark_map();
	for (b = 0; b <= maxbank; b++) {
		for (r = 0; r < 4; r++) {
			for (i = 0; i < 256; i += 4) {
				putchar(use[b][i + r]);
				if ((i & 0x3C) == 0x3C)
					putchar(' ');
			}
			putchar('\n');
		}
		putchar('\n');
		putchar('\n');
	}
	exit(0);
}
