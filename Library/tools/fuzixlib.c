#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    int fd;
    char buf[16];

    if (argc != 2) {
        fprintf(stderr, "%s [binary].\n", argv[0]);
        exit(1);
    }

    fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror(argv[1]);
        exit(1);
    }
    if (read(fd, buf, 16) != 16) {
        fprintf(stderr, "%s: could not read the header.\n", argv[1]);
        exit(1);
    }
    if (memcmp(buf + 3, "FZX", 3)) {
        fprintf(stderr, "%s: not a fuzix binary.\n", argv[1]);
        exit(1);
    }
    buf[5] = 'L';
    
    if (lseek(fd, 0L, SEEK_SET) < 0) {
        perror(argv[1]);
        exit(1);
    }
    
    if (write(fd, buf, 16) != 16 || close(fd)) {
        fprintf(stderr, "%s: header rewrite failed.\n", argv[1]);
        exit(1);
    }
    return 0;
}
