/*
 *	Kernel assisted ZX81 emulator
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/graphics.h>

typedef void (*bootstrap)(void *, int fd);

static void oom(void)
{
    write(2, "Out of memory.\n", 15);
    exit(1);
}

static uint8_t ident(void) {
    __asm
        xor	a
        dec	a
        daa
        ld	l,a
    __endasm;
}

__sfr __at 0xFD debug;

/* TODO: load a .p file into the memory map */
int main(int argc, char *argv[])
{
    struct display m, nm;
    uint8_t *x, *p, *pe;
    int fd;

    if (ident() == 0xF9) {
        write(2, "zx80: Z80 CPU required.\n", 24);
        exit(1);
    }

    /* Set the break for the ZX81 memory space */
    if (sbrk(0) >= (uint8_t *)0x7C00 || brk((uint8_t *)0x8000))
        oom();
    /* We need a bit of space for the ROM but this is fine just loaded
       higher up as we've got enough memory */
    x = sbrk(5120);	/* Modified ROM */
    if (x == (void *) -1)
        oom();
    fd = open("/usr/lib/zx80.rom", O_RDONLY);
    if (fd == -1) {
        perror("/usr/lib/zx80.rom");
        exit(1);
    }
    if (read(fd, x, 5120) != 5120) {
        fprintf(stderr, "zx80.rom: short file.\n");
        exit(1);
    }
    close(fd);

    if (argc > 2) {
       write(2, "zx80: invalid arguments.\n",25);
       exit(1);
    }

    if (argc == 2) {
        fd = open(argv[1], O_RDONLY);
        if (fd == -1) {
            perror(argv[1]);
            exit(1);
        }
    } else
        fd = 0;
    /* Should tidy - TODO - hardcoded for RC2014 setup - also error check more */
    ioctl(0, GFXIOC_GETMODE, &m);
    nm.mode = 1;
    ioctl(0, GFXIOC_SETMODE, &nm);

    /* Use some of the working space for the font. We checked there was room
       higher up. We need to borrow 2K for this so use 7800-7FFF */
    p = (uint8_t *)0x7800;
    memset(p, 0xAA, 2048);		/* Helps debug */
    memcpy(p, x + 0x00E00, 512);	/* Chars 0-63 */
    memcpy(p + 1024, x + 0x0E00, 512); /* Inverted as 128-191 */
    p += 1024;
    pe = p + 512;
    while(p < pe)
        *p++ ^= 0xFF;
    /* Load the font */
    if (ioctl(0, VTSETFONT, (uint8_t *)0x7800) != 0) {
        perror("vtsetfont");
        exit(1);
    }
    /* Ok font loaded, display no longer sane so hopefully it worked ! */
    /* Jump into the additional boot strapping in the emulated ROM and then
       into the system */
//    debug = 0x10;
    ((bootstrap)(x + 0x1000))(x, fd);
    /* If we come back here it broke - try and fix the video */
    ioctl(0, GFXIOC_SETMODE, &m);
    write(2, "zx80: start up failed.\n", 23);
    return 1;
}
