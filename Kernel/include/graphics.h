#ifndef _GRAPHICS_H
#define _GRAPHICS_H

/* We use the same structure for modes */
struct display {
  uint16_t width, height;	/* Logical display size */
  uint16_t stride, lines;	/* Physical layout */
  uint8_t vstep, hstep;		/* Scrolling step if supported or 0xFF */
  uint8_t format;
#define FMT_MONO_BW	0
#define FMT_MONO_WB	1
#define FMT_COLOUR4	2
#define FMT_COLOUR16	3
/* Those sufficiently funky */
#define FMT_SPECTRUM	128
#define FMT_VDP		129	/* VDP graphics engines */
  uint8_t hardware;
#define HW_UNACCEL	1	/* Simple display */
#define HW_VDP_9918	128	/* Not neccessarily MSX... */
#define HW_VDP_9938	129
#define HW_TRS80GFX	130	/* TRS80 model 4 graphics board */
  uint16_t features;
#define GFX_MAPPABLE	1	/* Can map into process memory */
#define GFX_PALETTE	2	/* Has colour palette */
#define GFX_OFFSCREEN	4	/* Offscreen memory */
#define GFX_VBLANK	8
#define GFX_ENABLE	16	/* Separate mode we enable/disable */
#define GFX_MULTIMODE	32	/* Has multiple modes */
#define GFX_PALETTE_SET	64	/* Has settable colour palette */
  uint16_t memory;		/* Memory size in KB (may be 0 if not relevant) */
  uint16_t commands;
#define GFX_BLTAL_CG	1	/* Aligned blit CPU to graphics */
#define GFX_BLTAL_GC	2	/* And the reverse */
#define GFX_SETPIXEL	4	/* Driver supports set/clear pixel */
#define GFX_HLINE	8	/* Horizontal line */
#define GFX_VLINE	16	/* Vertical line */
#define GFX_LINE	32	/* Arbitrary line */
#define GFX_BLT_GG	64	/* Screen to screen blit */
#define GFX_BLT_CG	128	/* Unaligned blits */
#define GFX_BLT_GC	256
#define GFX_RECT	512	/* Rectangles */
#define GFX_RAW		1024	/* Raw command streams */
#define GFX_RAWCOPY	1024	/* Raw command stream is copier format */
  uint16_t drawmodes;
#define MODE_XOR	1	/* XOR as well as set/clr */
#define MODE_PATTERN	2	/* 8x8 pattern */
};

/* FIXME: need a way to describe/set modes if multiple supported */

struct palette {
  uint8_t n;
  uint8_t r,g,b;
};

/* Do not fiddle with this struct idly - it has asm users */
struct attribute {
  uint8_t ink, paper;
  uint8_t mode;
#define	GFX_OP_COPY	0
#define GFX_OP_SET	1
#define GFX_OP_CLEAR	2
#define GFX_OP_XOR	3
  uint8_t flags;
};

/* Returned from a successful GFXIOC_MAP */
struct videomap {
  uaddr_t mmio;			/* Memory mapped register base */
  uaddr_t pio;			/* I/O space register base */
  uaddr_t fbmem;		/* Frame buffer memory */
  usize_t fbsize;
  uint8_t mmio_seg;		/* For the 8086 */
  uint8_t fbmem_seg;
  uint8_t spacing;		/* Multiplier for non standard register spacing */
  uint8_t flags;		/* Which maps are valid */
#define MAP_MMIO	1
#define MAP_MMIO_SEG	2
#define MAP_PIO		4
#define MAP_FBMEM	8
#define MAP_FBMEM_SEG	16
#define MAP_FBMEM_SIMPLE 32	/* Normal mapping of linear framebuffer as
				   mode would imply */
};


#define GFX_BUFLEN		64	/* Default buffer length  (128 byte) */

#define GFXIOC_GETINFO		0x0300	/* Query display info for this tty */
#define GFXIOC_ENABLE		0x0301	/* Enter graphics mode */
#define GFXIOC_DISABLE		0x0302	/* Exit graphics mode */
#define GFXIOC_GETPALETTE	0x0303	/* Get a palette entry */
#define GFXIOC_SETPALETTE	0x0304	/* Set a palette entry */
#define GFXIOC_MAP		0x0305	/* Map into process if supported */
#define GFXIOC_UNMAP		0x0306	/* Unmap from process */
#define GFXIOC_SETATTR		0x0307	/* Set the drawing attributes */
#define GFXIOC_SETPIXEL		0x0308	/* Set a pixel */
#define GFXIOC_HLINE		0x0309	/* Horizontal line */
#define GFXIOC_VLINE		0x030A	/* Vertical line */
#define GFXIOC_RECT		0x030B	/* Draw rectangle */
#define GFXIOC_BLTAL_CG		0x030C	/* Blit functions */
#define GFXIOC_BLTAL_GC		0x030D
#define GFXIOC_BLTCG		0x030E
#define GFXIOC_BLTGC		0x030F
#define GFXIOC_BLTGG		0x0310
#define GFXIOC_CMD		0x0311	/* Raw command stream for a VDP */
#define GFXIOC_PAN		0x0312	/* Panning */
#define GFXIOC_WAITVB		0x0313	/* Wait for vblank */
#define GFXIOC_GETPIXEL		0x0314	/* Read a pixel */
#define GFXIOC_GETMODE		0x0315	/* Get info on a mode */
#define GFXIOC_SETMODE		0x0315	/* Set video mode */
#endif
