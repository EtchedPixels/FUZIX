#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static uint8_t use[256];

struct section {
	struct section *next;
	uint8_t name;
	uint16_t base;
	uint16_t size;
};

struct section *sections;

/* The sections from the map we pre allocate */
static struct section sect[10];

static struct section *new_section(const char c, uint16_t base, uint16_t size)
{
	struct section *n = malloc(sizeof(struct section));
	if (n == NULL) {
		fprintf(stderr, "Out of memory.\n");
		exit(1);
	}
	n->name = c;
	n->base = base;
	n->size = size;
	n->next = NULL;
	return n;
}

static void insert_section(struct section *s)
{
	s->next = sections;
	sections = s;
}

static void check_overlap(struct section *sa, struct section *sb)
{
	/* Don't check versus self */
	if (sa == sb)
		return;
	/* A finishes below B start -> OK */
	if (sa->base + sa->size <= sb->base)
		return;
	/* B finishes before A start -> OK */
	if (sb->base + sb->size <= sa->base)
		return;
	fprintf(stderr, "Section %c (%04X-%04X) overlaps section %c (%04X-%04X).\n",
		sa->name,
		sa->base, sa->base + sa->size - 1,
		sb->name,
		sb->base, sb->base + sb->size - 1);
}

static void add_section(struct section *s)
{
	memset(use + (s->base >> 8),
		s->name, (s->size + 255) >> 8);
}

static void mark_map(void)
{
	struct section *s = sections;
	struct section *a;

	while (s) {
		if ((unsigned)s->base + s->size > 0xFFFF) {
			fprintf(stderr, "Section %c runs over the top of memory.\n",
				s->name, s->base + s->size - 1);
		}
		a = sections;
		while(a) {
			check_overlap(s, a);
			a = a->next;
		}
		add_section(s);
		s = s->next;
	}
}


static char linebuf[128];

int get_line(FILE *fp)
{
    char *p;
    if (fgets(linebuf, sizeof(linebuf) - 1, fp) == NULL)
        return 0;
    p = linebuf + strlen(linebuf) - 1;
    if (*p == '\n')
        *p = 0;
    return 1;
}
void load_info(FILE *fp)
{
    unsigned int st, en;
    char name;
    struct map *m;

    while(get_line(fp)) {
        if (*linebuf == '#')
            continue;
        if (sscanf(linebuf, "%c %x-%x", &name, &st, &en) != 3) {
                fprintf(stderr, "Unknown info line '%s'.\n", linebuf);
                exit(1);
        }
        insert_section(new_section(name, st, en - st + 1));
    }
}

int main(int argc, char *argv[])
{
	char buf[512];
	int i;
	int r, b;
	FILE *fp;

	fp = fopen("map.info", "r");
	if (fp) {
		load_info(fp);
		fclose(fp);
	}

	for (i = 0; i < 10; i++)
		sect[i].name = "ACDBXZSLsb??????"[i];
		
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

		if (strcmp(name, "__commondata_siz") == 0)
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
	/* Add any present sections to the map */
	for (i = 0; i < 10; i++)
		if (sect[i].size)
			insert_section(sect + i);
			
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
