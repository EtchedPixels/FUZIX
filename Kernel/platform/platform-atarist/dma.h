#ifndef _DMA_H
#define _DMA_H
/*
 *	The Atari ST DMA engine. Most of the I/O is built around this
 *
 *	Based upon EmuTOS
 */

struct dma {
    uint16_t pad0[2];
    uint16_t data;
    uint16_t control;
    uint8_t pad1;
    uint8_t addr_high;
    uint8_t pad2;
    uint8_t addr_med;
    uint8_t pad3;
    uint8_t addr_low;
    uint8_t pad4;
    uint8_t density;	/* Late systems only */
};

#define DMA 	((volatile struct dma *)0xFF8600)

#define DMA_A0		0x0002
#define DMA_A1		0x0004
#define DMA_HDC		0x0008
#define DMA_SCREG	0x0010
#define DMA_NODMA	0x0040
#define DMA_FDC		0x0080
#define DMA_WRBIT	0x0100

#define DMA_OK		0x0001
#define	DMA_SCNOT0	0x0002
#define DMA_DATREQ	0x0004

extern void dma_lock(void);
extern void dma_unlock(void);
extern uint8_t dma_is_locked(void);

extern void set_dma_addr(uint8_t *ptr);
extern uint16_t get_dma_status(void);
extern int dma_wait(uint16_t wait);

#endif
