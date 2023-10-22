#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <ctype.h>
#include <sys/stat.h>

#include "obj.h"

static char *symbols;
static int numsyms;
static struct objhdr hdr;
static int bigendian;

static char bogus[19] = { "\0OUT OF RANGE\0\0\0\0\0\0" };

static char *symptr(unsigned int n)
{
    if (n < numsyms)
        return symbols + S_ENTRYSIZE * n;
    return bogus;
}

static char *find_symbol(int seg, unsigned short addr)
{
    uint8_t *p = (uint8_t *)symbols;
    int n = 0;
    while(n++ < numsyms) {
        if (((p[NAMELEN + 2] << 8) | p[NAMELEN + 1]) == addr && seg == (p[0] & S_SEGMENT))
            return (char *)p;
        p += S_ENTRYSIZE;
    }
    return NULL;
}

static char *find_symbol_before(int seg, unsigned short addr, unsigned short *ap)
{
    uint8_t *p = (uint8_t *)symbols;
    uint8_t *candidate = 0;
    int n = 0;
    while(n++ < numsyms) {
        uint16_t a = (p[NAMELEN + 2] << 8) | p[NAMELEN + 1];
        if (a <= addr && seg == (p[0] & S_SEGMENT) && !(p[0] & S_UNKNOWN)) {
            *ap = a;
            candidate = p;
        }
        p += S_ENTRYSIZE;
    }
    return (char *)candidate;
}

static int nextbyte_eof(int fd)
{
    uint8_t c;
    int n = read(fd,&c, 1);
    if (n == 1)
        return c;
    if (n == -1) {
        perror("read");
        return 0xFF;
    }
    return -1;
}

static int nextbyte(int fd)
{
    int n = nextbyte_eof(fd);
    if (n == -1)
        fprintf(stderr, "Unexpected EOF in relocation stream.\n");
    return n;
}

static int bytect;
static char relbuf[64];
static char *relptr;
static uint16_t dot;

static void byte(int seg, uint8_t v)
{
    char *p;
    if ((p = find_symbol(seg, dot)) != NULL) {
        if (bytect) {
            printf("\n");
            bytect = 0;
        }
        printf("%.16s:\n", p + 1);
    }
    if (bytect == 0)
        printf("%04X\t", dot);
    printf("%02X ", v);
    bytect++;
    if (bytect == 16) {
        bytect = 0;
        printf("\n");
    }
}
    
static void reloc_tag(const char *p)
{
    *relptr++ = *p;
}

static void reloc_type(const char *p)
{
    memcpy(relbuf + 8, p, strlen(p));
}

static void reloc_seg(int n)
{
    relbuf[6] = "ACDBZXS7???????U"[n];
}

static void reloc_size(int n)
{
    relbuf[5] = "0123456789ABCDEF"[n];
}

static int reloc_word(int fd, int size)
{
    int n;
    if (size == 1)
        n = nextbyte(fd);
    else if (size == 2) {
        if (bigendian) {
            n = nextbyte(fd) << 8;
            n |= nextbyte(fd);
        } else {
            n = nextbyte(fd);
            n |= nextbyte(fd) << 8;
        }
    } else
        fprintf(stderr, "invalid size %d.\n", size);
    return n;
}

static void reloc_value(int fd, int size)
{
    int n = reloc_word(fd, size);
    sprintf(relbuf + 12, "%-5d", n);
}

static void reloc_symbol(int fd, int size)
{
    char *p = relbuf + 12;
    int  n = nextbyte(fd);
    n |= (nextbyte(fd) << 8);
    p += sprintf(p, "%.*s", NAMELEN, symptr(n) + 1);
    n = reloc_word(fd, size);
    if (n)
        sprintf(p, "+0x%04X", n);
}

static void reloc_symvalue(int fd, int seg, int size)
{
    if (size == 2) {
        uint16_t n;
        char *p;
        uint16_t v;
        
        /* FIXME: endianness check from header */
        n = nextbyte(fd) << 8;
        n |= nextbyte(fd);

        p = find_symbol_before(seg, n, &v);
        if (p) {
            if (v)
                sprintf(relbuf + 12, "%.16s+0x%04X", p + 1, n - v );
            else
                sprintf(relbuf + 12, "%.16s", p + 1);
        } else
            sprintf(relbuf + 12, "0x%04X", n);
    } else
        reloc_value(fd, size);
}

static void reloc_init(void)
{
    memset(relbuf, ' ', 32);
    relbuf[32] = 0;
    relptr = relbuf;
}

static void reloc_end(void)
{
    if (bytect) {
        printf("\n");
        bytect = 0;
    }
    printf("%s\n", relbuf);
} 

static int checkdot(int seg, int dot)
{
    /* ABS is special */
    if (dot > hdr.o_size[seg]) {
        fprintf(stderr, "Segment exceeds header size (%04X > %04X).\n",
            dot, hdr.o_size[seg]);
        return 1;
    }
    return 0;
}

static int dump_data(const char *p, int seg, int fd)
{
    int c;
    int size;
    int high;
    
    dot = 0;
    bytect = 0;
    
    while((c = nextbyte_eof(fd)) != -1) {
        if (c != REL_ESC) {
            if (checkdot(seg, dot))
                break;
            byte(seg, c);
            dot++;
            continue;
        }
        c = nextbyte(fd);
        if (c == REL_REL) {
            if (checkdot(seg, dot))
                break;
            byte(seg, REL_ESC);
            dot++;
            continue;
        }
        reloc_init();
        if (c == REL_EOF) {
            reloc_type("END");
            reloc_end();
            printf("\n\n");
            if (dot != hdr.o_size[seg]) {
                fprintf(stderr, "Segment is short (%04X < %04X).\n",
                    dot, hdr.o_size[seg]);
                return 1;
            }
            return 0;
        }
        /* Ok an actual relocation */
        if (c == REL_ORG) {
            reloc_type("ORG");
            /* FIXME: encoded little endian always */
            reloc_value(fd, 2);
            reloc_end();
            continue;
        }
        high = 0;
        if (c == REL_OVERFLOW) {
            reloc_tag("O");
            c = nextbyte(fd);
        }
        if (c == REL_HIGH) {
            reloc_tag("H");
            high = 1;
            c = nextbyte(fd);
        }
        size = ((c & S_SIZE) >> 4) + 1;
        reloc_size(size);

        if (c & REL_SIMPLE) {
            reloc_type("SEG");
            reloc_seg(c & S_SEGMENT);
            reloc_symvalue(fd, c & S_SEGMENT, size);
            reloc_end();
            dot += size - high;
            continue;
        }
        switch(c & REL_TYPE) {
        case REL_PCREL:
            reloc_tag("R");
        case REL_SYMBOL:
            reloc_type("SYM");
            reloc_symbol(fd, size);
            reloc_end();
            dot += size - high;
            continue;  
        }         
    }
    fprintf(stderr, "%s: Unexpected EOF.\n", p);
    return 1;
}

static int load_symbols(int fd, struct objhdr *oh)
{
    int symsize = oh->o_dbgbase - oh->o_symbase;
    symbols = malloc(symsize);
    if (symbols == NULL) {
        fprintf(stderr, "Out of memory.\n");
        exit(1);
    }
    numsyms = symsize / S_ENTRYSIZE;
    if (lseek(fd, oh->o_symbase, 0) < 0)
        return -1;
    if (read(fd, symbols, symsize) != symsize)
        return -1;
    return 0;
}

static int process(const char *p)
{
    int fd = open(p, O_RDONLY);
    int err = 0;
    int i;

    if (fd == -1) {
        perror(p);
        return 1;
    }
    if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr) || hdr.o_magic != MAGIC_OBJ) {
        fprintf(stderr, "%s: not a valid object file.\n", p);
        close(fd);
        return 1;
    }
    if (hdr.o_flags & OF_BIGENDIAN)
        bigendian = 1;

    if (load_symbols(fd, &hdr) < 0) {
        fprintf(stderr, "%s: cannot load symbols.\n", p);
        close(fd);
        return 1;
    }
    for (i = 0; i < OSEG; i++) {
        printf("Segment %d:\n\tSize: %u\n\tOffset: %lu\n", i, hdr.o_size[i],
            (long)hdr.o_segbase[i]);
        if (i == 3) {	/* BSS */
            printf("\n\n");
            continue;
        }
        if (lseek(fd, hdr.o_segbase[i], SEEK_SET) < 0) {
            perror(p);
            continue;
        }
        err |= dump_data(p, i, fd);
    }
    close(fd);
    return err;
}

int main(int argc, char *argv[])
{
    int err = 0;
    while(--argc)
        err |= process(*++argv);
    return err;
}