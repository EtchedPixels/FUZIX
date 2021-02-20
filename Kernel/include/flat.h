#ifndef FLAT_H
#define FLAT_H

/* We don't quite use the ucLinux names, we don't want to load a ucLinux binary
 * in error! */
#define FLAT_FUZIX_MAGIC "bFLT"

#define FLAT_VERSION 4

struct binfmt_flat {
	uint8_t magic[4];
	uint32_t rev;
	uint32_t entry;
	uint32_t data_start;
	uint32_t data_end;
	uint32_t bss_end;
	uint32_t stack_size;
	uint32_t reloc_start;
	uint32_t reloc_count;
	uint32_t flags;
	uint32_t filler[6];
};

#define FLAT_FLAG_RAM    0x0001 /* load program entirely into RAM */
#define FLAT_FLAG_GOTPIC 0x0002 /* program is PIC with GOT */
#define FLAT_FLAG_GZIP   0x0004 /* all but the header is compressed */
#define FLAT_FLAG_GZDATA 0x0008 /* only data/relocs are compressed (for XIP) */
#define FLAT_FLAG_KTRACE 0x0010 /* output useful kernel trace for debugging */

#endif

