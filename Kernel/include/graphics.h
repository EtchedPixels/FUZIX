#ifndef _GRAPHICS_H
#define _GRAPHICS_H

/* We use the same structure for modes */
struct display {
  uint8_t mode;			/* Mode number */
  uint16_t width, height;	/* Logical display size */
  uint16_t stride, lines;	/* Physical layout */
  uint8_t vstep, hstep;		/* Scrolling step if supported or 0xFF */
  uint8_t format;
#define FMT_MONO_BW	0
#define FMT_MONO_WB	1
#define FMT_COLOUR4	2
#define FMT_COLOUR16	3
#define FMT_TEXT	4	/* Text only mode */
#define FMT_MONO_WB_TILE8 5	/* White on black 8x8 tiled (Amstrad PCW etc) */
#define FMT_6PIXEL_128	6	/* 2x3 tiles from 128 (TRS80 style) */
#define FMT_4PIXEL_128	7	/* 2x2 tiles from 128 (6847 style) */
/* Those sufficiently funky */
#define FMT_SPECTRUM	128
#define FMT_VDP		129	/* VDP graphics engines */
#define FMT_UBEE	130	/* Microbee 6545 modes */
#define FMT_SAM2	131	/* SAM coupe mode 2 (1 pixel high chars with
                                   attribute map) */
#define FMT_TIMEX64	132	/* Weird byte interleaved spectrum like mode */
#define FMT_AMSMONO	133	/* Amstrad mono - weird interleave */
#define FMT_AMS4	134	/* Amstrad 4 colour - ditto */
#define FMT_AMS16	135	/* Amstraid 16 colour - ditto */
#define FMT_8PIXEL_MTX	136	/* 256 characters graphics mode symbols (MTX) */
#define FMT_3BPP_U16	137	/* 5 x 3bpp pixels a word (top bit unused) */
  uint8_t hardware;
#define HW_UNACCEL	1	/* Simple display */
#define HW_VDP_9918A	128	/* Not neccessarily MSX... */
#define HW_VDP_9938	129	/* MSX2 etc */
#define HW_TRS80GFX	130	/* TRS80 model 4 graphics board */
#define HW_HRG1B	131	/* HRG1B for TRS80 and VidoeGenie */
#define HW_MICROLABS	132	/* Microlabs Grafyx */
#define HW_MICROLABS4	133	/* Microlabs Grafyx for Model 4 */
#define HW_LOWE_LE18	134	/* Low Electronics LE-18 */
#define HW_VDP_9958	135	/* VDP9958 MSX2+ etc */
#define HW_EF9345	136	/* Thomson EF9345 */
#define HW_PROPGFX	137	/* RCbus propellor graphics */
  uint16_t features;
#define GFX_MAPPABLE	1	/* Can map into process memory */
#define GFX_PALETTE	2	/* Has colour palette */
#define GFX_OFFSCREEN	4	/* Offscreen memory */
#define GFX_VBLANK	8
#define GFX_MULTIMODE	32	/* Has multiple modes */
#define GFX_PALETTE_SET	64	/* Has settable colour palette */
#define GFX_TEXT	128	/* Console text works in this mode */
  uint16_t memory;		/* Memory size in KB (may be 0 if not relevant) */
  uint16_t commands;
#define GFX_DRAW	1	/* Supports the draw command */
#define GFX_RAW		2	/* Raw command streams to the GPU */
#define GFX_ADRAW	4	/* Supports draw attributes */
#define GFX_CLIP	8	/* Supports clipping */
#define GFX_BLIT	16	/* Supports screen to screen blits */
#define GFX_READ	32	/* Supports reading back a buffer */
#define GFX_AREAD	64	/* Supports reading back an attribute buffer */
#define GFX_PDRAW	128	/* Supports planar draw (draw buffer with a
				   leading plane mask) indicating which planes
				   to run the command on */
#define GFX_WRITE	256	/* Supports writing a buffer */
#define GFX_AWRITE	512	/* Supports writing an attribute buffer */
#define GFX_EXG		1024	/* Simultaenous GFX_READ/GFX_WRITE to swap */
  uint16_t twidth;		/* Character size information */
  uint16_t theight;		/* Characters per line/column */
 /* We may want to add some hardware ones as we hit machines that have them */
};

/* FIXME: need a way to describe/set modes if multiple supported */

struct palette {
  uint8_t n;
  uint8_t r,g,b;
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

#define GFXIOC_GETINFO		0x0300	/* Query display info for this tty */
#define GFXIOC_GETPALETTE	0x0303	/* Get a palette entry */
#define GFXIOC_SETPALETTE	0x0304	/* Set a palette entry */
#define GFXIOC_MAP		0x0305	/* Map into process if supported */
#define GFXIOC_UNMAP		0x0306	/* Unmap from process */
#define GFXIOC_DRAW		0x0307	/* Draw a buffer */
#define GFXIOC_RAW		0x0308	/* GPU direct buffer */
#define GFXIOC_ADRAW		0x0309	/* Draw an attribute buffer */
#define GFXIOC_CLIP		0x030A	/* Set clip rectangle */
#define GFXIOC_BLIT		0x030B	/* Screen to screen blit */
#define GFXIOC_READ		0x030C	/* Read back screen */
#define GFXIOC_AREAD		0x030D	/* Read back attributes */
#define GFXIOC_PDRAW		0x030E	/* Planar draw */
#define GFXIOC_PAN		0x030F	/* Panning */
#define GFXIOC_WAITVB		0x0310	/* Wait for vblank */
#define GFXIOC_GETMODE		0x0311	/* Get info on a mode */
#define GFXIOC_SETMODE		0x0312	/* Set video mode */
#define GFXIOC_WRITE		0x0313	/* Write to screen direct */
#define GFXIOC_AWRITE		0x0314	/* Write to attributes direct */
#define GFXIOC_EXG		0x0315	/* Exchange a block */

/*
 *	VDP specific ioctls: The 0x032X range is reused for each type
 */

struct vdp_rw {		/* Do not touch without changing asm helpers */
  uaddr_t data;
  uint16_t vdpaddr;		/* 0000-3BFF */
  uint8_t lines;
  uint8_t cols;
  uint8_t stride;
};

#define VDPIOC_SETUP		0x0320	/* Set TMS9918A registers */
#define VDPIOC_READ		0x0321	/* Read TMS9918A space */
#define VDPIOC_WRITE		0x0322	/* Write TMS9918A space */

#endif
