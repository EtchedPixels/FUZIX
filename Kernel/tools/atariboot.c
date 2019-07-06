/*
 *	Make a boot block Atari ST bootable
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>

static uint16_t bootblock[256];

int main(int argc, char *argv[])
{
    int fd;
    uint16_t *p = bootblock;
    uint16_t sum = 0;
    uint16_t word;
    int i;

    if (argc != 2) {
        fprintf(stderr, "%s <file>.\n", argv[0]);
        exit(1);
    }

    fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror(argv[1]);
        exit(1);
    }
    if (read(fd, bootblock, 512) == -1) {
        perror(argv[1]);
        exit(1);
    }

    for (i = 0; i < 255; i++) {
        sum += ntohs(*p);
        p++;
    }
    word = 0x1234 - sum;	/* What do we need to put in the last word ? */
    *p = htons(word);

    p = bootblock;
    sum = 0;

    for (i = 0; i < 256; i++) {
        sum += ntohs(*p);
        p++;
    }
    printf("Sum %04X\n", sum);

    if (lseek(fd, 0L, SEEK_SET) == -1) {
        perror("lseek");
        exit(1);
    }
    if (write(fd, bootblock, 512) != 512 || close(fd)) {
        fprintf(stderr, "%s: write failed.\n", argv[1]);
        exit(1);
    }
    return 0;
}
        