/*
 *	Turn a 4 x 16K banked kernel image into a 64K EPROM
 *
 *	Pack all the other segments away
 *
 *	For now this is hardcoded to suit Tom's SBC but we can generalize
 *	it with options as and when we need to
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

static uint8_t eprom[65536];
static uint8_t ram[65536];

void load_bank(const char *n, uint16_t base)
{
    FILE *f = fopen(n, "r");
    if (f == NULL) {
        perror(n);
        exit(1);
    }
    fread(eprom + base, 16384, 1, f);
    fclose(f);
}

void load_main(void)
{
    FILE *f = fopen("../../common.bin", "r");
    if (f == NULL) {
        perror("../../common.bin");
        exit(1);
    }
    fread(ram, 65536, 1, f);
    fclose(f);
}

void load_banks(void)
{
    load_bank("../../bank1.bin", 0);
    load_bank("../../bank2.bin", 0x4000);
    load_bank("../../bank3.bin", 0x8000);
    load_bank("../../bank4.bin", 0xC000);
}

/* Compress the RAM space into a block we expand. The linker fills with 0xFF
   and we get 00 fills for initialized data etc */

static void noroom(uint16_t left)
{
    fprintf(stderr, "Insufficient data space (%d bytes left to compress).\n", left);
    exit(1);
}

uint16_t compress(uint8_t *in, uint8_t *out, uint16_t ilen, uint16_t omax)
{
    uint8_t c;
    uint16_t n;
    while(ilen) {
        if (*in != 0xFF && *in != 0x00) {
            if (omax-- == 0)
                noroom(ilen);
            *out++ = *in++;
            ilen--;
            continue;
        }
        if (omax < 2)
                noroom(ilen);
        c = *in++;
        ilen--;
        n = 0;
        while(*in == c && ilen) {
            in++;
            n++;
            ilen--;
        }
        if (n > 253) {
            if (omax < 4)
                noroom(ilen);
            omax -= 4;
            *out++ = c;
            *out++ = 254;
            *out++ = n;
            *out++ = n >> 8;
            continue;
        }
        if (omax < 2)
            noroom(ilen);
        omax -= 2;
        *out++ = c;
        *out++ = n;
    }
    if (omax < 2)
        noroom(ilen);
    omax -= 2;
    *out++ = 255;
    *out++ = 255;
    return omax;
}

/* Copy the first 128 bytes into each other bank */
void fix_vectors(void)
{
    memcpy(eprom, ram, 128);
    memcpy(eprom + 16384, ram, 128);
    memcpy(eprom + 32768, ram, 128);
    memcpy(eprom + 49152, ram, 128);
}

void write_eprom(void)
{
    FILE *f = fopen("fuzix.rom", "w");
    if (f == NULL) {
        perror("fuzix.rom");
        exit(1);
    }
    if (fwrite(eprom, 65536, 1, f) != 1 || fclose(f)) {
        fprintf(stderr, "unable to write fuzix.rom.\n");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    int n;

    load_main();
    load_banks();
    
    n = compress(ram + 16384, eprom + 2 * 16384 + 8192, 49152, 8192);
    printf("Compressed RAM image is %d bytes.\n", 8192 - n);
    /* Deal with SDCC linker annoyances */
    fix_vectors();
    write_eprom();
    exit(0);
}
