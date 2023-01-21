#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

/* "Encrypt and sign" a TO8 boot block */

static uint8_t bootblock[256];

int main(int agc, char *argv[])
{
    unsigned i;
    uint8_t sum = 0x55;

    if (read(0, bootblock, 256) < 0) {
        perror("bless-to8");
        exit(1);
    }
    for (i = 0; i < 127; i++) {
        sum += bootblock[i];
        bootblock[i]--;
        bootblock[i] ^= 0xFF;
    }
    bootblock[127] = sum;
    if (write(1, bootblock, 256) != 256) {
        perror("bless-to8");
        exit(1);
    }
    return 0;
}
 