#include <stdio.h>
#include <stdlib.h>

static void makelb(const char *p, unsigned n)
{
    FILE *f = fopen(p, "w");
    if (f == NULL) {
        perror(p);
        exit(1);
    }
    fprintf(f, "\n\t.export __ldbyte%d\n\t.code\n", n);
    fprintf(f, "__ldbyte%d:\n", n);
    fprintf(f, "\tld hl,%d\n", n);
    fprintf(f, "\radd hl,sp\n");
    fprintf(f, "\tld l,(hl)\n");
    fprintf(f, "\tret\n");
    fclose(f);
}

static void makesb(const char *p, unsigned n)
{
    FILE *f = fopen(p, "w");
    if (f == NULL) {
        perror(p);
        exit(1);
    }
    fprintf(f, "\n\t.export __stbyte%d\n\t.code\n", n);
    fprintf(f, "__stbyte%d:\n", n);
    fprintf(f, "\tld a,l\n");
    fprintf(f, "\tld hl,%d\n", n);
    fprintf(f, "\radd hl,sp\n");
    fprintf(f, "\tld (hl),a\n");
    fprintf(f, "\tret\n");
    fclose(f);
}


static void makelw(const char *p, unsigned n)
{
    FILE *f = fopen(p, "w");
    if (f == NULL) {
        perror(p);
        exit(1);
    }
    fprintf(f, "\n\t.export __ldword%d\n\t.code\n", n);
    fprintf(f, "__ldword%d:\n", n);
    fprintf(f, "\tld hl,%d\n", n);
    fprintf(f, "\radd hl,sp\n");
    fprintf(f, "\tld a,(hl)\n");
    fprintf(f, "\tinc hl\n");
    fprintf(f, "\tld h,(hl)\n");
    fprintf(f, "\tld l,a\n");
    fprintf(f, "\tret\n");
    fclose(f);
}

static void makesw(const char *p, unsigned n)
{
    FILE *f = fopen(p, "w");
    if (f == NULL) {
        perror(p);
        exit(1);
    }
    fprintf(f, "\n\t.export __stword%d\n\t.code\n", n);
    fprintf(f, "__stword%d:\n", n);
    fprintf(f, "\tex de,hl\n");
    fprintf(f, "\tld hl,%d\n", n);
    fprintf(f, "\radd hl,sp\n");
    fprintf(f, "\tld (hl),e\n");
    fprintf(f, "\tinc hl\n");
    fprintf(f, "\tld (hl),d\n");
    fprintf(f, "\tex de,hl\n");
    fprintf(f, "\tret\n");
    fclose(f);
}

int main(int argc, char *argv[])
{
    char buf[256];
    unsigned i;

    for (i = 1; i < 32; i++) {
        sprintf(buf, "ldword/_%d.s", i);
        makelw(buf, i);
        sprintf(buf, "stword/_%d.s", i);
        makesw(buf, i);
        sprintf(buf, "ldbyte/_%d.s", i);
        makelb(buf, i);
        sprintf(buf, "stbyte/_%d.s", i);
        makesb(buf, i);
    }
}

