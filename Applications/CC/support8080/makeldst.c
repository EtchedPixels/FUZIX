#include <stdio.h>
#include <stdlib.h>

static void makelb(const char *p, unsigned n)
{
    FILE *f = fopen(p, "w");
    if (f == NULL) {
        perror(p);
        exit(1);
    }
    fprintf(f, "\n\t.export __ldbyte%d\n\n\t.setcpu 8080\n\t.code\n", n);
    fprintf(f, "__ldbyte%d:\n", n);
    fprintf(f, "\tlxi h,%d\n", n);
    fprintf(f, "\tdad sp\n");
    fprintf(f, "\tmov l,m\n");
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
    fprintf(f, "\n\t.export __stbyte%d\n\n\t.setcpu 8080\n\t.code\n", n);
    fprintf(f, "__stbyte%d:\n", n);
    fprintf(f, "\tmov a,l\n");
    fprintf(f, "\tlxi h,%d\n", n);
    fprintf(f, "\tdad sp\n");
    fprintf(f, "\tmov m,a\n");
    fprintf(f, "\tmov l,a\n");
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
    fprintf(f, "\n\t.export __ldword%d\n\n\t.setcpu 8080\n\t.code\n", n);
    fprintf(f, "__ldword%d:\n", n);
    fprintf(f, "\tlxi h,%d\n", n);
    fprintf(f, "\tdad sp\n");
    fprintf(f, "\tmov a,m\n");
    fprintf(f, "\tinx h\n");
    fprintf(f, "\tmov h,m\n");
    fprintf(f, "\tmov l,a\n");
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
    fprintf(f, "\n\t.export __stword%d\n\n\t.setcpu 8080\n\t.code\n", n);
    fprintf(f, "__stword%d:\n", n);
    fprintf(f, "\txchg\n");
    fprintf(f, "\tlxi h,%d\n", n);
    fprintf(f, "\tdad sp\n");
    fprintf(f, "\tmov m,e\n");
    fprintf(f, "\tinx h\n");
    fprintf(f, "\tmov m,d\n");
    fprintf(f, "\txchg\n");
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

