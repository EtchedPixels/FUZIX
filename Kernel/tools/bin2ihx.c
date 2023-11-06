#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

/* Convert a binary file into IHX as needed for the
   Z80 Membership Card boot */

static void phexc(uint8_t c)
{
    putchar("0123456789ABCDEF"[c & 0x0F]); 
}

static void phex2(uint8_t v)
{
    phexc(v >> 4);
    phexc(v & 15);
}

static void phex4(uint16_t v)
{
    phex2(v >> 8);
    phex2(v);
}

static void pline(uint16_t addr, unsigned len, uint8_t *p)
{
    unsigned sum = 0;
    putchar(':');
    phex2(len);
    sum += len;
    phex4(addr);
    sum += addr >> 8;
    sum += addr & 0xFF;
    phex2(0);

    while(len--) {
        sum += *p;
        phex2(*p++);
    }
    phex2((-sum) & 0xFF);
    putchar('\n');
}

static unsigned getval(const char *n, const char *p)
{
    char *t;
    unsigned long r = strtoul(p, &t, 0);
    if (t && *t) {
        fprintf(stderr, "%s: '%s' is not a valid value.\n", n, p);
        exit(1);
    }
    if (r > 0xFFFF) {
        fprintf(stderr, "%s: '%s' is out of range.\n", n, p);
        exit(1);
    }
    return r;
}


int main(int argc, char *argv[])
{
    uint16_t addr;
    uint16_t len;
    uint8_t buf[16];
    int fd;

    if (argc != 4) {
        fprintf(stderr, "%s file start length\n", argv[0]);
        exit(1);
    }

    addr = getval(argv[0], argv[2]);
    len = getval(argv[0], argv[3]);
    if (addr + len < addr) {
        fprintf(stderr, "%s: too large\n", argv[0]);
        exit(1);
    }

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror(argv[1]);
        exit(1);   
    }
    if (lseek(fd, addr, SEEK_SET) < 0) {
        fprintf(stderr, "%s: could not seek to %s\n", argv[1], argv[2]);
        exit(1);
    }
    while(len) {
        unsigned n = 0x10;
        if (n > len)
            n = len;
        n = read(fd, buf, n);
        if (n < 0) {
            perror("read");
            exit(1);
        }
        if (n == 0)
            break;
        pline(addr, n, buf);
        addr += n;
        len -= n;
    }
    close(fd);
    puts(":00000001FF");
    exit(0);
}
