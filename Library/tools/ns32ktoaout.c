/*
 *	Convert an NS32K a.out binary into something more digestable
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct exec_aout {
    uint32_t	a_midmag;		/* Flags and stuff */
    uint32_t	a_text;			/* Size of text (base is implicitly 0) */
    uint32_t	a_data;			/* Size of data (base varies by type) */
    uint32_t	a_bss;			/* Size of BSS (follows data) */
    uint32_t	a_syms;			/* Size in *bytes* of symbol table */
    uint32_t	a_entry;		/* Address of entry point */
    uint32_t	a_trsize;		/* Size in bytes of text relocation table */
    uint32_t	a_drsize;		/* Size in bytes of data relocation table */
};

#define MID_FUZIXNS32	0x03C0
#define NMAGIC		0410

static struct exec_aout hdr;
static uint8_t *b0, *b1;


static uint8_t *load_block(FILE *fp, off_t base, size_t len)
{
	uint8_t *m = malloc(len + 0x20);
	if (m == NULL) {
	    fprintf(stderr, "Out of memory.\n");
	    exit(1);
        }
        fprintf(stderr, "Loading %X bytes from %X\n", len, base);
        if (fseek(fp, base, SEEK_SET)) {
            fprintf(stderr, "Seek error.\n");
            exit(1);
        }
        if (fread(m + 0x20, len, 1, fp) != 1) {
            fprintf(stderr, "Read error loading binary block.\n");
            exit(1);
        }
        /* Add the header words. We can't do this in crt0 as the toolchain doesn't
           seem able to do link orders and customs ections */
        memset(m, 0, 0x20);
        *(uint32_t *)m = 4096;		/* Stack size */
        return m;
}

/* Write relocation records */
static void relocate_binary(FILE *o)
{
    unsigned n = hdr.a_text + hdr.a_data;
    uint8_t *p0 = b0, *p1 = b1;
    int i;
    unsigned r;

    while(n--) {
        if (*p0 != *p1) {
            if (*p0 != *p1 - 1) {
                fprintf(stderr, "Reloc mismatch @ %06X %d %d\n", p0 - b0, *p0, *p1);
                for (i = -8; i < 8; i++)
                    fprintf(stderr, "%02X|%02X\n", p0[i], p1[i]);
                exit(1);
            }
            /* For now write 32bit addresses of relocations big endian. We'll compact
               them once this works and we can make a similar 68K arrangement for a.out
               binaries proper not flt */
            r = p0 - b0;
            /* 32bit addresses LE: this means we see the mismatch on the second byte
               so adjust accordingly */
            r--;
            fputc(r >> 24, o);
            fputc(r >> 16, o);
            fputc(r >> 8, o);
            fputc(r, o);
            hdr.a_trsize += 4;
        }
        p0++;
        p1++;
    }
}

int main(int argc, char *argv[])
{
    FILE *i0, *i1, *o;

    if (argc != 4) {
        fprintf(stderr, "%s in0 in1 out\n", argv[0]);
        exit(1);
    }
    i0 = fopen(argv[1], "r");
    if (i0 == NULL) {
        perror(argv[1]);
        exit(1);
    }
    i1 = fopen(argv[2], "r");
    if (i1 == NULL) {
        perror(argv[2]);
        exit(1);
    }
    o = fopen(argv[3], "w");
    if (o == NULL) {
        perror(argv[3]);
        exit(1);
    }
    if (fread(&hdr, sizeof(hdr), 1, i0) != 1) {
        fprintf(stderr, "%s: not a valid binary.\n", argv[1]);
        exit(1);
    }
    printf("Text size %x, Data %x, BSS %x, Run from %x\n",
        hdr.a_text, hdr.a_data, hdr.a_bss, hdr.a_entry);

    /* We assume -N (so no magic padding). Change this into two if we decide
       to do alignment in the file. As we've no MMU and fancy map paging we
       don't need alignment features. Note that the first 0x20 bytes of the
       block are cleared as the input binary is 0x20 offset to allow for the
       stub block. */
    b0 = load_block(i0, sizeof(struct exec_aout), hdr.a_text + hdr.a_data);
    b1 = load_block(i1, sizeof(struct exec_aout), hdr.a_text + hdr.a_data);

    hdr.a_trsize = hdr.a_drsize = 0;
    hdr.a_syms = 0;

    /* Copy the 0 based binary into the output up to the data end. We will redo the
       header at the end */
    if (fwrite(&hdr, sizeof(hdr), 1, o) != 1) {
        fprintf(stderr, "%s: write error on header.\n", argv[3]);
        exit(1);
    }
    if (fwrite(b0, hdr.a_text + hdr.a_data, 1, o) != 1) {
        fprintf(stderr, "%s: write error on body.\n", argv[3]);
        exit(1);
    }
    /* Now append relocationr records */
    relocate_binary(o);
    fclose(i0);
    fclose(i1);
    rewind(o);
    hdr.a_midmag = htonl(NMAGIC | (MID_FUZIXNS32 << 16));
    if (fwrite(&hdr, sizeof(hdr), 1, o) != 1) {
        fprintf(stderr, "%s: write error on final header.\n", argv[3]);
        exit(1);
    }
    printf("%d bytes of relocations added.\n", hdr.a_trsize + hdr.a_drsize);
    fclose(o);
    return 0;
}
