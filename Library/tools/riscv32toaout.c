/*
 *	Convert a riscv32 binary image into our a.out format
 *
 *	RISC-V relocations are complicated but providing we keep to a 4K
 *	alignment the 20:12 split in the loads and the like means that
 *	all we have to worry about is LUI and data relocations, both of
 *	which look and work the same because of the layout of LUI.
 *
 *	The only real oddity here is that we link the binary to run
 *	from 0x1000 and 0x2000 as 0x0000 will cause the linker to remove
 *	upper loads and rely upon the sign extend thus changing the binary
 *	layout and size.
 *
 *	TODO: stack size argument
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

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

#define MID_RISCV32	0x03F0
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
        if (fseek(fp, base, SEEK_SET)) {
            fprintf(stderr, "Seek error.\n");
            exit(1);
        }
        fprintf(stderr, "Loading %u bytes from %u\n", (unsigned)len, (unsigned)base);
        if (fread(m + 0x20, len, 1, fp) != 1) {
            fprintf(stderr, "Read error loading binary block.\n");
            exit(1);
        }
        /* Add the header words. We can't do this in crt0 as the toolchain doesn't
           seem able to do link orders and custom sections */
        memset(m, 0, 0x20);
        /* FIXME : configurable */
        *(uint32_t *)m = 16384;		/* Stack size */
        return m;
}

/* Write relocation records */
static void relocate_binary(FILE *o)
{
    int n = hdr.a_text + hdr.a_data;
    uint8_t *p0 = b0, *p1 = b1;
    int i;
    unsigned r;

    while(n-- > 0) {
        uint8_t diff = *p1 - *p0;
        if (diff) {
            /* Second byte of a little endian relocation */
            if (diff != 0x10) {
                fprintf(stderr, "Reloc mismatch %d @ %06X %d %d\n", diff, (unsigned)(p0 - b0), *p0, *p1);
                for (i = -8; i < 8; i++)
                    fprintf(stderr, "%02X|%02X\n", p0[i], p1[i]);
                exit(1);
            }
            *p0 -= 0x10;	/* Shift to 0 base */
            r = p0 - b0;
            r--;	/* Get correct byte start */
            fputc(r, o);
            fputc(r >> 8, o);
            fputc(r >> 16, o);
            fputc(r >> 24, o);
            hdr.a_trsize += 4;
            p0 += 2;
            p1 += 2;
            n -= 2;
        }
        p0++;
        p1++;
    }
}

static unsigned build_header(FILE *m)
{
    unsigned tsize = 0, dsize = 0, bsize = 0, dusize = 0;
    char buf[256];
    char sym[256];
    unsigned addr;
    char type;
    while(fgets(buf, 255, m)) {
        if (sscanf(buf, "%x %c %s", &addr, &type, sym) != 3)
            continue;
        if (strcmp(sym, "__stubs") == 0 && addr != 0x1000) {
            fprintf(stderr, "__stubs: bad base.\n");
            exit(1);
        }
        if (strcmp(sym, "__data_start") == 0)
            tsize = addr;
        if (strcmp(sym, "_edata_unaligned") == 0)
            dusize = addr;
        if (strcmp(sym, "_edata") == 0)
            dsize = addr;
        if (strcmp(sym, "_end") == 0)
            bsize = addr;
        if (strcmp(sym, "_start") == 0)
            hdr.a_entry = addr - 0x1000;
    }
    if (tsize == 0 || dsize == 0 || hdr.a_entry == 0) {
        fprintf(stderr, "Missing symbols.\n");
        exit(1);
    }
    hdr.a_text = tsize - 0x1000;
    hdr.a_data = dsize - tsize;
    hdr.a_bss = bsize - hdr.a_data;
    hdr.a_syms = 0;
    /* Size to load */
    return dusize - 0x1000;
}

int main(int argc, char *argv[])
{
    FILE *i0, *i1, *o, *m;
    unsigned size;

    if (argc != 5) {
        fprintf(stderr, "%s in0 in1 out map\n", argv[0]);
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
    m = fopen(argv[4], "r");
    if (m == NULL) {
        perror(argv[4]);
        exit(1);
    }

    /* Manufacture the header */
    size = build_header(m);
    fclose(m);

    printf("Text size %x, Data %x, BSS %x, Run from %x\n",
        hdr.a_text, hdr.a_data, hdr.a_bss, hdr.a_entry);

    /* We assume -N (so no magic padding). Change this into two if we decide
       to do alignment in the file. As we've no MMU and fancy map paging we
       don't need alignment features. Note that the first 0x20 bytes of the
       block are cleared as the input binary is 0x20 offset to allow for the
       stub block. */
    b0 = load_block(i0, 0, size - 0x20);
    b1 = load_block(i1, 0, size - 0x20);

    hdr.a_trsize = hdr.a_drsize = 0;
    hdr.a_syms = 0;

    /* Stubs */
    hdr.a_text += 0x20;

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
    hdr.a_midmag = htonl(NMAGIC | (MID_RISCV32 << 16));
    if (fwrite(&hdr, sizeof(hdr), 1, o) != 1) {
        fprintf(stderr, "%s: write error on final header.\n", argv[3]);
        exit(1);
    }
/*    printf("%d bytes of relocations added.\n", hdr.a_trsize + hdr.a_drsize); */
    fclose(o);
    return 0;
}
