/*
 *	Pack an 8085 binary using the symbol table data (should actually work
 *	for anything using the Fuzix linker)
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

unsigned int discard;
unsigned int common;
unsigned int commondata;
unsigned int bss;
unsigned int discard_size;
unsigned int common_size;
unsigned int commondata_size;
unsigned int bss_size;

struct match {
    const char *sym;
    unsigned int *val;
};

struct match rules[] = {
    { "__discard", &discard },
    { "__common", &common },
    { "__commondata", &commondata },
    { "__bss", &bss },
    { "__discard_size", &discard_size },
    { "__common_size", &common_size },
    { "__commondata_siz", &commondata_size },
    { "__bss_size", &bss_size },
    { NULL, NULL }
};

static void symbol_line(const char *p)
{
    char name[32];
    unsigned addr;
    char type;
    struct match *m;

    if (sscanf(p, "%04X %c %32s", &addr, &type, name) != 3) {
        fprintf(stderr, "Format error '%s'\n", p);
        exit(1);
    }

    m = rules;
    while(m->sym) {
        if (strcmp(m->sym, name) == 0) {
            *m->val = addr;
            return;
        }
        m++;
    }
}

static void read_symbols(void)
{
    char buf[64];
    while(fgets(buf, 63, stdin) != NULL)
        symbol_line(buf);
}

static uint8_t image[0x10000];

int main(int argc, char *argv[])
{
    unsigned addr;
    unsigned fd;
    unsigned size;

    if (argc != 3) {
        fprintf(stderr, "%s: input output < map\n", argv[0]);
        exit(1);
    }
    read_symbols();
    printf("BSS ends at %04X, packing discard (%04X/%04X) and common (%04X/%04X).\n",
        bss + bss_size, discard, discard_size, common, common_size);
    if (commondata_size)
        printf("Commondata found packing commondata (%04X/%04X)\n",
            commondata, commondata_size);

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror(argv[1]);
        exit(1);
    }
    size = read(fd, image, 0x10000);
    if (size <32768) {	
        fprintf(stderr, "%s: short read ?\n", argv[0]);
        exit(1);
    }
    close(fd);
    addr = bss;
    memmove(image + addr, image + common, common_size);
    addr += common_size;
    if (commondata_size) {
        memmove(image + addr, image + commondata, commondata_size);
        addr += commondata_size;
    }
    memmove(image + addr, image + discard, discard_size);
    addr += discard_size;
    fd = open(argv[2], O_WRONLY|O_TRUNC|O_CREAT, 0600);
    if (fd == -1) {
        perror(argv[2]);
        exit(1);
    }
    if (write(fd, image, addr) != addr) {
        fprintf(stderr, "%s: short write ?\n", argv[2]);
        exit(1);
    }
    close(fd);
    return 0;
}
