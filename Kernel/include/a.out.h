/*
 *	Traditional 32bit a.out format
 */

struct exec {
    uint32_t	a_midmag;		/* Flags and stuff */
    uint32_t	a_text;			/* Size of text (base is implicitly 0) */
    uint32_t	a_data;			/* Size of data (base varies by type) */
    uint32_t	a_bss;			/* Size of BSS (follows data) */
    uint32_t	a_syms;			/* Size in *bytes* of symbol table */
    uint32_t	a_entry;		/* Address of entry point */
    uint32_t	a_trsize;		/* Size in bytes of text relocation table */
    uint32_t	a_drsize;		/* Size in bytes of data relocation table */
    /* What follows is counted as part of the text as far as a.out is concerned but
       is replaced by the stub from the kenrel. It's provided as space in the crt0
       for future use (should all be zeros) but not ccopied into the user memory */
    uint32_t	stacksize;
    uint32_t	unused[7];
};

/* Whilst we use a.out our relocation format for executables is not the same. This shouldn't
   matter as executables with relocations are not normal elsewhere */

/* MIDMAG is a 32bit big endian word holding flags << 26 | mid << 16 | magic */


#define NMAGIC	0410			/* Read only text (we only use this right now) */
#define OMAGIC	0407			/* Impure */
#define QMAGIC	0314			/* Compact demand load */
#define ZMAGIC	0413			/* Demand load */

/* There is no canonical list of the 10bit machine identifications but the BSD one is
   kind of the reference. In addition they tend to be about what machine and OS they are for.
   We simply use our own chunk at the end of the range. Nobody uses these as far as I can
   tell and it's not like new users of a.out keep appearing */

#define MID_FUZIXNS32	0x03C0		/* Flat Fuzix ns32k binary */
#define MID_FUZIX68000	0x03D0		/* Flat Fuzix 68K binaries */
#define MID_FUZIX68010	0x03D1
#define MID_FUZIX68020	0x03D2
#define MID_FUZIX68030	0x03D3
#define MID_FUZIX68040	0x03D4
#define MID_FUZIX68060	0x03D6
#define MID_ARMM0	0x03E0		/* 32bit ARM */
#define MID_ARMM4	0x03E4		/* We can deal with ARM26 when we get there */
#define MID_RISCV32	0x03F0
