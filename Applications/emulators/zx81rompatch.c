#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

static uint8_t rom[9216];

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
    load(rom, "zx81bootup", 9216);
    load(rom, "zx81-base.rom", 8192);
    rom[0] = 0xC3;
    rom[1] = 0x05;
    rom[2] = 0x20;	/* JP 0x2005 (exitfunc) */
    rom[0x02BA] = 0xC9;	/* Ret before the display code */
    rom[0x02BB] = 0x2A;	/* LD HL,(0x2000) (keycode) */
    rom[0x02BC] = 0x00;
    rom[0x02BD] = 0x20;
    rom[0x02BE] = 0xC9;	/* RET */
    save(rom, "zx81.rom", 9216);
    return 0;
}

/* Need to figure out how to deal with the main display loop ?
   - force slow mode by patching 0x021C into a nop nop
   - but then how to deal with the ix returning exit loop stuff ?
 */
