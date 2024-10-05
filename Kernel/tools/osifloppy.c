#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

/* 1024 bytes at 0300 */
static char preamble[] = {
    0x22,
    0x00,
    0x04,
};

static char trackbits[] = {
    0x57,
    0x00
};

static char buf[0x0F00];

static unsigned tracksize;

static void calc_checksum(unsigned char *ptr, unsigned size)
{
    unsigned csum = 0;
    while(size--)
        csum += *ptr++;
    *ptr++ = csum;
    *ptr++ = csum >> 8;
}

static void write_pad(unsigned n)
{
    if (write(1, buf, n) != n) {
        perror("write");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    int fd;
    int len;
    unsigned t = 1;

    if (argc != 5) {
        fprintf(stderr, "%s 8|5 6502block z80block kernel\n", argv[0]);
        exit(1);
    }
    if (strcmp(argv[1], "8") == 0)
        tracksize = 0x0F00;
    else if (strcmp(argv[1], "5") == 0)
        tracksize = 0x0900;
    else {
        fprintf(stderr, "%s: unknown disk size '%s'\n", argv[0], argv[1]);
        exit(1);
    }
    fd = open(argv[2], O_RDONLY);
    if (fd == -1) {
        perror(argv[2]);
        exit(1);
    }
    if (lseek(fd, 0x2200, SEEK_SET) == -1) {
        perror("seek");
        exit(1);
    }
    len = read(fd, buf, 512);
    if (len < 0) {
        perror("read");
        exit(1);
    }
    write(1, preamble, sizeof(preamble));
    write(1, buf, 512);
    close(fd);
    fd = open(argv[3], O_RDONLY);
    if (fd == -1) {
        perror(argv[3]);
        exit(1);
    }
    if (lseek(fd, 0x2400, SEEK_SET) == -1) {
        perror("seek");
        exit(1);
    }
    /* FIXME: need to pull payload half from D000 */
    len = read(fd, buf, 0x40);
    if (len < 0) {
        perror("read");
        exit(1);
    }
    write(1, buf, 0x40);
    /* Payload */
    if (lseek(fd, 0xD000, SEEK_SET) == -1) {
        perror("seek");
        exit(1);
    }
    len = read(fd, buf, 0x1C0);
    if (len < 0) {
        perror("read");
        exit(1);
    }
    write(1, buf, 0x1C0);
    close(fd);
    /* We have written 1027 bytes */
    write_pad(tracksize - 1027);

    fd = open(argv[4], O_RDONLY);
    if (fd == -1) {
        perror(argv[4]);
        exit(1);
    }
    while(t < 25) {
        len = read(fd, buf, 2048);
        if (len == -1)
            break;
        if (len < 2048)
            memset(buf + len, 0xAA, 2048 - len);
        calc_checksum((unsigned char *)buf, 2048);
        trackbits[1] = t++;
        if (write(1, trackbits, sizeof(trackbits)) != sizeof(trackbits) ||
            write(1, buf, 2050) != 2050) {
            perror("write");
            exit(1);
        }
        /* We have written 2052 bytes, pad the rest */
        write_pad(tracksize - 2052);
    }
    if (len == -1) {
        perror("read");
        exit(1);
    }
    while(t++ < 77)
        write_pad(tracksize);
}
