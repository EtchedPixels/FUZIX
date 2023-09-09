#ifndef _OBJ_H
#define _OBJ_H

/* Long name is used for cross building and non 8bit environments. In an
   8bit environment longer names are just too expensive */
#ifdef OBJ_LONGNAME
#define MAGIC_OBJ		0x4D1B
#define MAGIC_OBJ_SWAPPED	0x1B4D
#define NAMELEN			32
#else
#define MAGIC_OBJ		0x3D1A
#define MAGIC_OBJ_SWAPPED	0x1A3D
#define NAMELEN			16
#endif

#define OSEG	8

struct objhdr
{
    uint16_t o_magic;
    uint8_t o_arch;
#define OA_8080		1
#define OA_6502		2
#define OA_DGNOVA	3	/* So I can test PC relative */
#define OA_6800		4
#define OA_Z8		5
#define OA_1802		6
#define OA_TMS9900	7
#define OA_8008		8
#define OA_INS8060	9
#define OA_INS8070	10
#define OA_WARREX	11
#define OA_BYTE		12	/* Bytecode */
    uint8_t o_flags;
#define OF_BIGENDIAN	1
#define OF_WORDMACHINE	2	/* 16bit word addressed */
    uint16_t o_cpuflags;
#define OA_8080_Z80	1
#define OA_8080_Z180	2
#define OA_8080_Z280	4
#define OA_8080_R800	8
#define OA_8080_8085	16

#define OA_6502_BCD	1	/* Uses BCD instructions */
#define OA_6502_NMOS	2	/* Uses NMOS undocumented */
#define OA_6502_65C02	4	/* Uses 65C02 */
#define OA_6502_BITOPS	8	/* Uses extended bit operations */
#define OA_6502_65C816	16	/* 65C816 and relatives */
#define OA_6502_ZPAT0	32	/* Binary Assumes ZP is at 0 */
#define OA_6502_65CE02	64	/* Does anyone really care ? */

#define OA_DGNOVA_MUL	1
#define OA_DGNOVA_FPU	2
#define OA_DGNOVA_NOVA3	4
#define	OA_DGNOVA_NOVA4	8

#define OA_6800_6803	1
#define OA_6800_6303	2
#define OA_6800_68HC11	4

#define OA_TMS9900_9995 1	/* TMS 9995 */

#define OA_WARREX_CPU6	1

    uint16_t o_unused;		/* So it packs right */
    uint32_t o_segbase[OSEG];
    uint16_t o_size[OSEG];
    uint32_t o_symbase;
    uint32_t o_dbgbase;
};

/* This byte introduces a relocation or may be escaped */
#define REL_ESC		0xDA

#define REL_SIZE	0x30	/* 1-4 bytes */
/* If REL_SIMPLE then ... */
#define REL_SIMPLE	0x80	/* relocation to base of segment */
#define REL_SEG		0x0F	/* segment 0-15 */	

/*    followed by the bytes to relocate */

/* Otherwise */
#define REL_TYPE	0x0F
/* 00 is reserved */
#define REL_SPECIAL	0x00	/* REL_REL REL_EOF etc */
#define REL_SYMBOL	0x01	/* Followed by a 2 byte symbol code then 
                                   offset to relocate */
#define	REL_PCREL	0x02	/* Followed by a 2 byte symbol code then
				   a word sized value regardless of reloc size
                                   but the resulting reloc is written to the
                                   size given */

#define REL_REL		(REL_SPECIAL| (0 << 4))	/* 00 */
#define REL_EOF		(REL_SPECIAL| (1 << 4)) /* 10 */
/* Overflow suppresses overflow checking on the relocation that follows (no
   second REL_ESC */
#define REL_OVERFLOW	(REL_SPECIAL| (2 << 4))	/* 20 */
/* Indicate a high byte relocation. Treated as normal until the final absolute
   write where the two bytes of reloc are relocated and the high byte used. If
   used with overflow must follow it directly */
#define REL_HIGH	(REL_SPECIAL| (3 << 4))	/* 30 */
/* Indicate a change in address for ABS areas */
#define REL_ORG		(REL_SPECIAL| (4 << 4))	/* 40 */
/* Relocation modifier block. Not yet implemented. Needed
   for things like B pointers and page addressing (eg 1802 branches) */
#define REL_MOD		(REL_SPECIAL| (5 << 4)) /* 50 */
/* Followed by
   7: highbit (set if mask is high bits used)
   6: error if cannot resolve
   5-0: mask number of bits 0-63 - 0 no mask

   byte 2 (bits 5-3 0 for now)
   7: scale direction
   6: error if cannot resolve
   2-0: scale (1,2,4,8)
 */
#define RELMOD_RELH	0x80
#define RELMOD_RELERR	0x40
#define RELMOD_RELBITS	0x3F

/* symbols and debug are in the format 
    uint8_t flags
    char name[16] (0 terminated if < 16)
    uint16_t data */

#define	S_UNKNOWN	0x80
#define S_PUBLIC	0x40		/* unknown is public .. */
#define S_SEGMENT	0x0F		/* 00 means absolute */
#define S_ANY		0x0F
#define S_SIZE		0x30		/* Size bits 0-3 = 1-4 bytes */

#define S_ENTRYSIZE	(3 + NAMELEN)

/*
 * Segments
 */
#define UNKNOWN		15
#define ABSOLUTE	0		/* Constant, not relocated */
#define CODE		1		/* Relocated versus code */
#define DATA		2		/* Relocated versus data */
#define BSS		3		/* Relocated versus BSS */
#define ZP		4		/* Relocated versus zero page */
#define DISCARD		5		/* Discard for things like kernels */
#define COMMON		6		/* Common for things like kernels */
#define LITERAL		7		/* Literals (mostly a compiler helper) */
/* Special cases 8+ don't exist as real segments */
#define PCREL		14		/* assumed signed */
/* and 15 is 'any' */

#endif
