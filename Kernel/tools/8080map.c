#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct symbol {
    uint32_t addr;
    const char *name;
    uint8_t seg;
    uint8_t ext;
};

#define MAXSYM 8192

static struct symbol *syms[MAXSYM];
static int symnum = 0;

static void new_symbol(const char *name, uint32_t addr, uint8_t seg, uint8_t ext)
{
    struct symbol *s = malloc(sizeof(struct symbol));
    const char *p = strdup(name);
    if (!s || !p) {
        fprintf(stderr, "Out of memory.\n");
        exit(1);
    }
    s->name = p;
    s->addr = addr;
    s->seg = seg;
    s->ext = ext;
    if (symnum == MAXSYM) {
        fprintf(stderr, "Too many symbols!\n");
        exit(1);
    }
    syms[symnum++] = s;
}

static void bad(const char *p)
{
    fprintf(stderr, "Invalid map line: %s", p);
    exit(1);
}

static void load_symbols(FILE *fp)
{
    char buf[256];
    char err[256];

    while(fgets(buf, 255, stdin)) {
        char *a,*s,*t,*n;
        unsigned int addr, seg;

        strcpy(err, buf);
        
        a = strtok(buf, " \t\n");
        if (a == NULL)
            bad(err);
        s = strtok(NULL, " \t\n");
        if (s == NULL)
            bad(err);
        t = strtok(NULL, " \t\n");
        if (t == NULL)
            bad(err);
        n = strtok(NULL, " \t\n");
        if (n == NULL)
            bad(err);

        /* ABS are not interesting */
        if (*s == 'A')
            continue;

        if (*t != '-' && *t != 'E' && *t != 'S' && *t != 'A')
            bad(err);
        if (sscanf(a, "%x", &addr) != 1)
            bad(err);
        seg = 255;
        if (sscanf(s, "%d", &seg) != 1 || seg < 0 || seg > 15)
            bad(err);
        new_symbol(n, addr, seg, *t);
    }
}

static int symcmp(const void *a, const void *b)
{
    struct symbol * const *s1 = a;
    const struct symbol * const *s2 = b;
    if ((*s1)->addr == (*s2)->addr)
        return 0;
    if ((*s1)->addr > (*s2)->addr)
        return 1;
    return -1;
}

static void sort_symbols(void)
{
    qsort(syms, symnum, sizeof(struct symbol *), symcmp);
}

static void dump_map(void)
{
    int i;
    for (i = 0; i < symnum; i++) {
        struct symbol *s = syms[i];
        int size;
        if (i != symnum - 1)
            size = syms[i+1]->addr - s->addr;
        else
            size = 0;
        printf("%-20s %04x %5d   %c    %d\n",
            s->name, s->addr, size, s->ext, s->seg);
    }
}

int main(int argc, char *argv[])
{
    load_symbols(stdin);
    sort_symbols();
    dump_map();
    return 0;
}
