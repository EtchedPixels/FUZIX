/*
 *	Interpolate the input into a megarom style image for the KC85/1 and
 *	friends. 2K slices at 256 block spacing.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


static uint8_t megarom[256 * 10240];

static void store_mega(uint8_t *ptr, unsigned bank, unsigned base)
{
    unsigned offset = (256 * base + bank) * 2048;
    memcpy(megarom + offset, ptr, 2048);
}

/* Helps for debug - all empty banks are filled with their ID */
static void fill_mega(unsigned bank, unsigned base)
{
    unsigned offset = (256 * base + bank) * 2048;
    memset(megarom + offset, bank, 2048);
}

int main(int argc, char *argv[])
{
    static uint8_t buf[10240];
    int l;
    unsigned bank;

    for(bank = 0; bank < 256; bank++) {
        fill_mega(bank, 0);
        fill_mega(bank, 1);
        fill_mega(bank, 2);
        fill_mega(bank, 3);
        fill_mega(bank, 4);
        bank++;
    }
    bank = 0;
    while((l = read(0, buf, sizeof(buf))) == sizeof(buf))
    {
        store_mega(buf, bank, 0);
        store_mega(buf + 2048, bank, 1);
        store_mega(buf + 4096, bank, 2);
        store_mega(buf + 6144, bank, 3);
        /* Debug aid - stamp the bank tops */
        buf[8192 + 2046] = ~bank;
        buf[8192 + 2047] = bank;
        store_mega(buf + 8192, bank, 4);
        bank++;
    }
    if (l != 0) {
        fprintf(stderr, "megarom: size wrong.\n");
        exit(1);
    }
    fprintf(stderr, "MegaROM assembled: %d banks.\n", bank);
    write(1, megarom, sizeof(megarom));
    return 0;
}
    