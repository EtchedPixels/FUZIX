#include <stdio.h>
#include <stdint.h>
#include <string.h>


const char *regname = "abcdehlm";

void logic(const char *p, unsigned oc)
{
    int i;
    for (i = 0; i <= 7; i++) {
        printf("\t{\t0,\t\"%s%c\",\tTIMPL,\t\t0%03o\t},\n",
            p, regname[i], oc | i);
    }
    oc &= 077;
    /* Immediate form */
    printf("\t{\t0,\t\"%si\",\tTIMM8,\t\t0%03o\t},\n",
        p, oc | 4);
}

void load(void)
{
    int i,j;

    for (i = 0; i <= 7; i++) {
        for (j = 0; j <= 7 ; j++) {
            if (i == 7 && j == 7)
                continue;
            else
                printf("\t{\t0,\t\"l%c%c\",\tTIMPL,\t\t0%03o\t},\n",
                    regname[i], regname[j], 0300 + (i << 3) + j);
        }
    }
    for (i = 0; i <= 7; i++) {
        printf("\t{\t0,\t\"l%ci\",\tTIMM8,\t\t0%03o\t},\n",
            regname[i],
            0006 + (i << 3));
    }
}

static char *condcode[] = {
    "fc",
    "fz",
    "fm",
    "fp",
    "tc",
    "tz",
    "tm",
    "tp"
};

void cond(const char c, unsigned oc)
{
    int i;
    for (i = 0; i < 7; i++) {
        printf("\t{\t0,\t\"%c%s\",\tTBRA,\t\t0%03o\t},\n",
            c, condcode[i], oc | (i << 3));
    }
}

/* And the rest are fixed or inp/oup */

struct opinfo {
    char *name;
    unsigned op;
};

struct opinfo fixed[] = {
     { "rlc", 0002 },
     { "rrc", 0012 },
     { "ral", 0022 },
     { "rar", 0032 },
     { "ret", 0007 },	/* 0x7 */
     { "hlt", 0377 },	/* Also 000 and 001 */
     { NULL, }
};

struct opinfo fixedbr[] = {
     { "jmp", 0104 },	/* 1x4 */
     { "cal", 0106 },	/* 1x6 */
     { NULL, }
};

struct opinfo alu[] = {
    { "ad", 0300 },
    { "ac", 0310 },
    { "su", 0320 },
    { "sb", 0330 },
    { "nd", 0340 },
    { "xr", 0350 },
    { "or", 0360 },
    { "cp", 0370 },
    { NULL, }
};
    
int main(int argc, char *argv[])
{
    struct opinfo *o;

    load();

    o = alu;
    while(o->name) {
        logic(o->name, o->op);
        o++;
    }

    cond('j', 0100);
    cond('c', 0102);
    
    o = fixed;
    while(o->name) {
        printf("\t{\t0,\t\"%s\",\tTIMPL,\t\t0%03o\t},\n",
            o->name, o->op);
        o++;
    }
    o = fixedbr;
    while(o->name) {
        printf("\t{\t0,\t\"%s\",\tTBRA,\t\t0%03o\t},\n",
            o->name, o->op);
        o++;
    }

    /* And finally we have RST */
    printf("\t{\t0,\t\"rst\",\tTRST,\t\t0007\t},\n");
    return 0;
}
