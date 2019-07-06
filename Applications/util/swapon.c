#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/swap.h>


static uint8_t buf[512];

int main(int argc, char *argv[])
{
    int fd;
    int n;

    if (argc != 3) {
        fprintf(stderr, "%s [device] [size].\n", argv[0]);
        exit(1);
    }
    n = atoi(argv[2]);
    if (n < 256) {
        fprintf(stderr, "%s: invalid number of blocks.\n", argv[0]);
        exit(1);
    }
    fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror(argv[1]);
        exit(1);
    }
    if (read (fd, buf, 512) != 512) {
        fprintf(stderr, "%s: unable to read '%s'.\n", argv[0], argv[1]);
        exit(1);
    }
    if (buf[510] == 0x55 && buf[511] == 0xAA) {
        fprintf(stderr, "%s: '%s' has a partition table.\n", argv[0], argv[1]);
        exit(1);
    }
    if (read (fd, buf, 512) != 512) {
        fprintf(stderr, "%s: unable to read '%s'.\n", argv[0], argv[1]);
        exit(1);
    }
    if ((buf[0] == 0x31 && buf[1] == 0xC6) ||
            (buf[0] == 0xC6 && buf[1] == 0x31)) {
        fprintf(stderr, "%s: '%s' has a file system.\n", argv[0], argv[1]);
        exit(1);
    }
    if (swapon(fd, n) < 0) {
        perror("swapon");
        exit(1);
    }
    return 0;
}
