#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

static uint8_t rom[5120];

static void load(uint8_t *p, const char *name, int len)
{
    int fd = open(name, O_RDONLY);
    if (fd == -1) {
        perror(name);
        exit(1);
    }
    if (read(fd, p, len) != len) {
        fprintf(stderr, "%s: too short.\n", name);
        exit(1);
    }
    close(fd);
}

static void save(uint8_t *p, const char *name, int len)
{
    int fd = open(name, O_WRONLY|O_CREAT|O_TRUNC, 0600);
    if (fd == -1) {
        perror(name);
        exit(1);
    }
    if (write(fd, p, len) != len) {
        fprintf(stderr, "%s: write error.\n", name);
        exit(1);
    }
    close(fd);
}

int main(int argc, char *argv[])
{
    load(rom, "zx80bootup", 5120);
    load(rom, "zx80-base.rom", 4096);
    rom[0] = 0xC3;
    rom[1] = 0x05;
    rom[2] = 0x10;	/* JP 0x1005 (exitfunc) */
    save(rom, "zx80.rom", 5120);
    return 0;
}

/* Need to figure out how to deal with the main display loop ?
   - force slow mode by patching 0x021C into a nop nop
   - but then how to deal with the ix returning exit loop stuff ?
 */
