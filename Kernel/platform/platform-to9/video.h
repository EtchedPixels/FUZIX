/*
 *	Video modes for the Thomson TO series
 */

#define lgamod	(*(volatile uint8_t *)0xE7DC)

/*
 *	The original TO7 has only COL40 mode. The first 8K of video RAM is pixels the second colour. Only the
 *	low 6bits of colour RAM are implemented. The 7/70 has 8bit colour ram and 16 colour mode (supports the
 *	extra 'pastel' colours. The TO9 adds the extra video modes and palette but doesn't allow the video
 *	base to be set. The TO8 and TO9+ allow the video base to be moved
 */
#define	VID_COL40	0	/* 40 col 16 colour, fg/bg from colour plane for each 8bits of pixel plane */
#define VID_BITMAP	0x21	/* 40 col 4 colour pixel planes (high bit is pixel low colour plane) */
#define VID_PAGE1	0x24	/* 40 col mono pixels from pixel plane */
#define VID_PAGE2	0x25	/* 40 col mono pixels from colour plane */
#define VID_STACK2	0x26	/* 40 col bitmap in pt is colour 2 if set else transparent
                                   40 col bitmap in col provides colour 1/0 */
#define VID_80COL	0x2A	/* 80 col mono interleaved bytes between pixel and colour plane */
#define VID_STACK4	0x3F	/* 20 col 16 colour four layers of pixels with transparency colours 4,3,2,1 bg 0 */
#define VID_BITMAP4B	0x41	/* 40 col 2bpp pixmap byte interleaved between pixel and colour plane */
#define VID_BITMAP16	0x7B	/* 20 col 4bpp pixmap byte interleaved between pixel and colour plane */
#define VID_PALETTE	0xFF	/* Video memory becomes palette access, displays weirdness */


#define vidborder	(*(volatile uint8_t *)0xE7DD)
/* Border in bits 3-0, RAM page to use for vifdeo in 7-6 (0-3) */
