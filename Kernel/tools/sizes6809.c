#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

struct symbol {
    struct symbol *next;
    char name[33];
    unsigned int addr;
};

struct symbol *syms;

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

void insert_symbol(char *name, unsigned int addr)
{
    struct symbol *s = malloc(sizeof(struct symbol));
    struct symbol **n;
    if (s == NULL) {
        fprintf(stderr,"Out of memory.\n");
        exit(1);
    }
    strcpy(s->name, name);
    s->addr = addr;

    n = &syms;
    while(*n) {
        if ((*n)->addr >= addr)
            break;
        n = &((*n)->next);
    }
    s->next = *n;
    *n = s;
}

void scan_symbols(FILE *fp)
{
    char name[33];
    unsigned addr;
    int x;

    while(get_line(fp) && *linebuf) {
        if (strncmp(linebuf, "Section: ", 9))
            break;
    }
    do {
        if ((x = sscanf(linebuf + 8, "%32s %*s = %x", name, &addr)) != 2) {
            fprintf(stderr," Unable to parse segment entry '%s' (%d).\n", linebuf, x);
            exit(1);
        }
        if (*name != 'L' && *name != '\\')
            insert_symbol(name, addr);
    } while(get_line(fp) && *linebuf);
}

void dump_sizes(void) {
    struct symbol *s = syms;
    while(s && s->next) {
        printf("%0d: %-32s\n", s->next->addr - s->addr, s->name);
        s = s->next;
    }
    if (s)
        printf("0000: %-32s End\n", s->name);
}


int main(int argc, char *argv[])
{
    scan_symbols(stdin);
    dump_sizes();
    exit(0);
}
