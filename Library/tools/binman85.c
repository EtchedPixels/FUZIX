/*
 *	Minimal helper to do the header fixups for 8085 binaries until we teach ld85
 *	proper Fuzix binary building
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    int fd;
    static uint8_t buf[16];
    if (argc != 2) {
        fprintf(stderr, "%s binary\n", argv[0]);
        exit(1);
    }
    fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror(argv[1]);
        exit(1);
    }
    if (read(fd, buf, 16) != 16) {
        fprintf(stderr, "%s: short read.\n", argv[1]);
        exit(1);
    }
    buf[7]--;
    if (lseek(fd, 0L, 0) < 0 || write(fd, buf, 16) != 16) {
        fprintf(stderr, "%s: unable to rewrite.\n", argv[1]);
        exit(1);
    }
    close(fd);
    return 0;    
}