#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <tinysd.h>
#include <z80softspi.h>

uint16_t ramtop = PROGTOP;
uint8_t vtattr_cap;
uint16_t swap_dev = 0xFFFF;
uint8_t clk_irq;		/* Set if the clock can cause interrupts */
uint8_t plt_tick_present;

struct blkbuf *bufpool_end = bufpool + NBUFS;

/* FIXME: missing prototype ? */
extern int strcmp(const char *, const char *);
/* On idle we spin checking for the terminals. Gives us more responsiveness
   for the polled ports */
void plt_idle(void)
{
	irqflags_t irq = di();
	tty_poll();
	kbd_poll();
	sync_clock();
	irqrestore(irq);
}

void do_beep(void)
{
}

__sfr __at 0x21	clk_tenths;
__sfr __at 0x22 clk_secs;
__sfr __at 0x23 clk_tsecs;
__sfr __at 0x2F clk_stat;

uint8_t plt_param(char *p)
{
	if (strcmp(p, "clkint") == 0) {
		clk_irq = 1;
		/* FIXME: do a reset cycle first */
		clk_stat = 0x9;	/* Repeating 0.5 sec - really 0.516.6 sec */
		clk_stat | clk_stat | clk_stat;	/* Enable */
		plt_tick_present = 1;
		return 1;
	}
	used(p);
	return 0;
}

void plt_interrupt(void)
{
	/* We don't have interrupts for the keyboard */
	kbd_poll();
	tty_poll();
	if (clk_irq) {
		if (clk_stat & 0x08) {	/* Check 4 or 8 - need datasheet */
			/* Not ideal but we need to work out how to handle
			   the different clocks gracefully */
			timer_interrupt();
			timer_interrupt();
			timer_interrupt();
			timer_interrupt();
			/* Do we need to read again ? */
		}
	}
}

/*
 *	FIXME: reclaim to end of usable memory
 */
void plt_discard(void)
{
	bufptr bp = bufpool_end;
	extern uint16_t discard_size;

	discard_size /= sizeof(struct blkbuf);

	kprintf("%d buffers reclaimed from discard\n", discard_size);

	bufpool_end += discard_size;	/* Reclaim the discard space */

	memset(bp, 0, discard_size * sizeof(struct blkbuf));
	/* discard_size is in discard so it dies here */
	for (bp = bufpool + NBUFS; bp < bufpool_end; ++bp) {
		bp->bf_dev = NO_DEVICE;
		bp->bf_busy = BF_FREE;
	}
}

/* SD mode - todo */
#ifdef CONFIG_NASCOM_SD
/*
 *	SD card bit bang. For now just a single card to get us going. We
 *	should fix the cs asm to allow for multiple cards
 *
 *	Bits
 *	7: MISO
 *	4: CLK
 *	3: \CS card 0
 *	0: MOSI
 */

/* Port A is 4/5 Port B is 6/7. We use Port B as it usually has free lines even when
   a printer is wired in. We do use bit 0 for speed so the printer may need adjusting */
uint16_t pio_c = 0x07;

void pio_setup(void)
{
    spi_piostate = 0xE0;

    spi_data = 0x01;
    spi_clock = 0x10;

    /* TODO: adjust if add printer support */
    out16(pio_c, 0xCF);		/* Mode 3 */
    out16(pio_c, 0xE6);		/* MISO input, unused as input (so high Z) */
    out16(pio_c, 0x07);		/* No interrupt, no mask */
}

void sd_spi_raise_cs(void)
{
    out16(spi_port, spi_piostate |= 0x08);
}

void sd_spi_lower_cs(void)
{
    spi_piostate &= ~0x08;
    out16(spi_port, spi_piostate);
}

void sd_spi_fast(void)
{
}

void sd_spi_slow(void)
{
}

COMMON_MEMORY

bool sd_spi_receive_sector(uint8_t *ptr) __naked
{
  __asm
    pop de
    pop hl
    push hl
    push de
    ld a, (_td_raw)
#ifdef SWAPDEV
    cp #2
    jr nz, not_swapin
    ld a,(_td_page)
    call map_for_swap
    jr doread
not_swapin:
#endif
    or a
    jr nz, to_user
    call map_buffers
    jr doread
to_user:
    call map_process_always
doread:
    call _sd_spi_rx_sector
    jp map_kernel_restore
  __endasm;
}

bool sd_spi_transmit_sector(uint8_t *ptr) __naked
{
  __asm
    pop de
    pop hl
    push hl
    push de
    ld a, (_td_raw)
#ifdef SWAPDEV
    cp #2
    jr nz, not_swapout
    ld a, (_td_page)
    call map_for_swap
    jr dowrite
not_swapout:
#endif
    or a
    jr nz, from_user
    call map_buffers
    jr dowrite
from_user:
    call map_process_always
dowrite:
    call _sd_spi_tx_sector
    jp map_kernel_restore
  __endasm;
}

#endif
