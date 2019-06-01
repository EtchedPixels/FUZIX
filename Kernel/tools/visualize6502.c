#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

static char linebuf[128];

static char map[256];

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

void map_insert(char *name, uint16_t start, uint16_t end)
{
    char c = '?';
    end = (end + 255) >> 8;
    start >>= 8;

    /* Just confuses the map */
    if (strcmp(name, "VECTORS") == 0)
        return;
    /* Fixed preallocation */
    if (strcmp(name, "ZEROPAGE") == 0)
        return;

    if (strncmp(name, "SYS", 3) == 0 && isdigit(name[3]))
        c = name[3];
    else if (strncmp(name, "SEG", 3) == 0)
        c = 'C';
    else if (strcmp(name, "CODE") == 0)
        c = 'C';
    else if (strcmp(name, "DISCARD") == 0)
        c = 'X';
    else if (strcmp(name, "DISCARDDATA") == 0)
        c = 'x';
    else if (strcmp(name, "RODATA") == 0)
        c = 'c';
    else if (strcmp(name, "DATA") == 0)
        c = 'd';
    else if (strcmp(name, "BSS") == 0)
        c = 'b';
    else if (strcmp(name, "STUBS") == 0)
        c = '@';
    else if (strcmp(name, "ISTACK") == 0)
        c = 'I';
    else if (strcmp(name, "COMMONMEM") == 0)
        c = 'S';
    else if (strcmp(name, "COMMONDATA") == 0)
        c = 's';
    else if (strcmp(name, "START") == 0)
        c = '!';
    else
        printf("unknown segment '%s'.\n", name);
    memset(map + start, c, end - start);
}

void parse_map(FILE *fp)
{
    char *name, *startp, *endp;
    uint16_t start, end;
    while(get_line(fp) && *linebuf) {
        name = strtok(linebuf, "\t ");
        startp = strtok(NULL, "\t ");
        endp = strtok(NULL, "\t ");
        if (!name || !startp || !endp) {
            fprintf(stderr, "Unable to parse segment entry.\n");
            exit(1);
        }
        start = strtoul(startp, NULL, 16);
        end = strtoul(endp, NULL, 16);
        map_insert(name, start, end);
    }
}

void find_map(FILE *fp)
{
    while(get_line(fp)) {
        if (strncmp(linebuf, "Segment list:", 13) == 0) {
            get_line(fp);
            get_line(fp);
            get_line(fp);
            parse_map(fp);
            return;
        }
    }
    fprintf(stderr, "No valid segment list found.\n");
    exit(1);
}

void init_map(void)
{
    memset(map, '#', sizeof(map));
    map[0] = 'Z';
    map[1] = 'S';
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

int main(int argc, char *argv[])
{
    init_map();
    find_map(stdin);
    print_map();
    exit(0);
}
