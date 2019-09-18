/*
 *	Set the boot command line on a device
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

static int root_fd;
static uint8_t buf[512];

int dump_cmd(void)
{
    uint8_t *p = buf + 380;
    unsigned int n = 0;
    if (*p != 0xB5 || p[1] != 0x5E) {
        puts("No command set.");
        return 0;
    }
    p += 2;
    printf("Command: ");
    while(n++ < 64 && *p) {
        putchar(*p);
        p++;
    }
    putchar('\n');
    return 0;
}

int update_cmd(char *argv[])
{
    int l = 512;

    buf[380] = 0xB5;
    buf[381] = 0x5E;

    if (strlen(argv[2]) > 63) {
        fprintf(stderr, "%s: command line '%s' is too long.\n", argv[0], argv[2]);
        return 1;
    }

    strncpy((char *)buf + 382, argv[2], 64);

    if (lseek(root_fd, 0L, SEEK_SET) || (l = write(root_fd, buf, 512)) != 512 || close(root_fd)) {
        if (l == 512)
            fprintf(stderr, "%s: error updating command line.\n", argv[0]);
        else
            perror(argv[1]);
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int l;
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "%s /dev/.. [cmd].\n", argv[0]);
        exit(1);
    }
    root_fd = open(argv[1], argc == 2 ? O_RDONLY : O_RDWR);
    if (root_fd == -1) {
        perror(argv[1]);
        return 1;
    }
    if ((l = read(root_fd, buf, 512)) != 512 || buf[510] != 0x55 || buf[511] != 0xAA) {
        fprintf(stderr, "%s: '%s' does not appear to be a valid partition table.\n",
            argv[0], argv[1]);
        exit(1);
    }
    if (argc == 2)
        return dump_cmd();
    return update_cmd(argv);
}
