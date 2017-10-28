#define MAGIC_OBJ		0x3D1A
#define MAGIC_OBJ_SWAPPED	0x1A3D

#define OSEG	8

struct objhdr
{
    uint16_t o_magic;
    uint8_t o_arch;
#define OA_Z80		1
    uint8_t o_flags;
    uint16_t o_cpuflags;
#define OA_Z80_Z180	1
#define OA_Z80_Z280	2
#define OA_Z80_R800	4
    uint32_t o_segbase[OSEG];
    uint16_t o_size[OSEG];
    uint32_t o_symbase;
    uint32_t o_dbgbase;
};

/* This byte introduces a relocation or may be escaped */
#define REL_ESC		0xDA
#define REL_REL		0x00	/* DA00 means write in DA */

#define REL_SIZE	0x30	/* 1-4 bytes */
/* If REL_SIMPLE then ... */
#define REL_SIMPLE	0x80	/* relocationto base of segment */
#define REL_SEG		0x0F	/* segment 0-15 */	

/*    followed by the bytes to relocate */

/* Otherwise */
#define REL_TYPE	0x0F
/* 00 is reserved */
#define REL_SYMBOL	0x01
/* 02-0F reserved */
/* Followed by 2 byte number of symbol in symbol table */

/*	followed by the bytes to relocate */


/* symbols and debug are in the format 
    uint8_t flags
    char name[16] (0 terminated if < 16)
    uint16_t data */

#define	S_UNKNOWN	0x80
#define S_PUBLIC	0x40		/* unknown is public .. */
#define S_SEGMENT	0x0F		/* 00 means absolute */

#define S_SIZE		19




