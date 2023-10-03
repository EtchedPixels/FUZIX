#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

static char linebuf[128];

static char map[257];	/* Allow an extra for rounding up */

struct map {
    struct map *next;
    uint16_t start;
    uint16_t end;
    char name[16];
};

struct map *maps;

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

void map_add(struct map *m)
{
    struct map *mp = maps;
    for (mp = maps; mp != NULL; mp = mp->next) {
        if (mp->end < m->start)
            continue;
        if (mp->start >= m->end)
            continue;
        /* Discard is special - it can overlap user because it is gone by
           the time anything occupies specific user reserved space */
        if (strcmp(m->name, "user") == 0 && strcmp(mp->name, ".discard") == 0)
            continue;
        if (strcmp(mp->name, "user") == 0 && strcmp(m->name, ".discard") == 0)
            continue;
        /* Overlap */
        fprintf(stderr, "***WARNING: map overlap between %s and %s\n",
            m->name, mp->name);
        fprintf(stderr, "%04X-%04X v %04X-%04X\n",
            m->start, m->end, mp->start, mp->end);
    }
    m->next = maps;
    maps = m;
}
            
void map_insert(char *name, uint16_t start, uint16_t end)
{
    char c = '?';
    struct map *m = malloc(sizeof(struct map));
    if (m == NULL) {
        fprintf(stderr, "Out of memory.\n");
        exit(1);
    }
    m->start = start;
    m->end = end;
    strncpy(m->name, name, 15);
    m->name[15] = 0;

    map_add(m);

    end = (end + 255) >> 8;
    start >>= 8;

    if (start > end) {
        fprintf(stderr, "Binary overflows address space (%X %X).\n", start, end);
        exit(1);
    }

    if (strcmp(name, ".magic") == 0)
        return;

    if (strcmp(name, "user") == 0)
        c = 'U';
    else if (strcmp(name, "reserve") == 0)
        c = '+';
    else if (strcmp(name, ".vectors") == 0)
        c = '@';
    else if (strcmp(name, ".udata") == 0)
        c = 'U';
    else if (strcmp(name, ".text") == 0)
        c = 'C';
    else if (strcmp(name, ".text1") == 0)
        c = '1';
    else if (strcmp(name, ".text2") == 0)
        c = '2';
    else if (strcmp(name, ".text3") == 0)
        c = '3';
    else if (strcmp(name, ".text4") == 0)
        c = '4';
    else if (strcmp(name, ".video") == 0)
        c = 'V';
    else if (strcmp(name, ".videodata") == 0)
        c = 'v';
    else if (strcmp(name, ".bufpool") == 0)
        c = 'B';
    else if (strcmp(name, ".buffers") == 0)
        c = 'B';
    else if (strcmp(name, ".discard") == 0)
        c = 'X';
    else if (strcmp(name, ".discarddata") == 0)
        c = 'x';
    else if (strcmp(name, ".data") == 0)
        c = 'd';
    else if (strcmp(name, ".bss") == 0)
        c = 'b';
    else if (strcmp(name, ".istack") == 0)
        c = 'I';
    else if (strcmp(name, ".common") == 0)
        c = 'S';
    else if (strcmp(name, ".commondata") == 0)
        c = 's';
    else if (strcmp(name, ".start") == 0)
        c = '!';
    else
        printf("unknown segment '%s'.\n", name);
    memset(map + start, c, end - start);
}

void scan_sections(FILE *fp)
{
    char segname[17];
    unsigned int start, length;
    int x;

    while(get_line(fp) && *linebuf) {
        if (strncmp(linebuf, "Section: ", 9))
            break;
        if ((x = sscanf(linebuf + 9, "%16s %*s load at %x, length %x",
            segname, &start, &length)) != 3) {
            fprintf(stderr," Unable to parse segment entry '%s' (%d).\n", linebuf, x);
            exit(1);
        }
        map_insert(segname, start, start + length - 1);
    }
}

void init_map(void)
{
    memset(map, '#', sizeof(map));
}

void print_map(void)
{
    int r, i;
    for (r = 0; r < 4; r++) {
	for (i = 0; i < 256; i += 4) {
		putchar(map[i + r]);
		if ((i & 0x3C) == 0x3C)
			putchar(' ');
	}
	putchar('\n');
    }
    putchar('\n');
}

void load_info(FILE *fp)
{
    char name[16];
    unsigned int st, en;
    struct map *m;

    while(get_line(fp)) {
        if (*linebuf == '#')
            continue;
        if (sscanf(linebuf, "%15s %x-%x",
            name, &st, &en) != 3) {
                fprintf(stderr, "Unknown info line '%s'.\n", linebuf);
                exit(1);
        }
        map_insert(name, st, en);
    }
}

int main(int argc, char *argv[])
{
    FILE *fp;

    init_map();

    fp = fopen("platform/map.info", "r");
    if (fp) {
        load_info(fp);
        fclose(fp);
    }
    scan_sections(stdin);
    print_map();
    exit(0);
}
