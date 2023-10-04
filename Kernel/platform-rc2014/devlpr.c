#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <printf.h>
#include <printer.h>
#include <blkdev.h>
#include <devppa.h>
#include <devlpr.h>

/*
 *	Parallel printer driver for the MG014 printer interface
 *
 *	This is an 82C55 based interface with the basic status lines
 *	and control lines rather than just ack. Unlike the PC interface
 *	no signals have software invert.
 *
 *	It's also possible to use a Z80 PIO for printing but we don't
 *	currently support that.
 */

static uint8_t have_lp;

#ifdef CONFIG_RC2014_EXTREME
__sfr __banked __at 0x0CB8 lpdata;	/* Port A - data, out */
__sfr __banked __at 0x0DB8 lpstatus;	/* Port B - status, in */
__sfr __banked __at 0x0EB8 lpctrl;	/* Port C control outputs */
__sfr __banked __at 0x0FB8 setup;	/* 82C55 control */
#define PPA_PORT 0x0CB8
#else
__sfr __at 0x0C lpdata;		/* Port A - data, out */
__sfr __at 0x0D lpstatus;	/* Port B - status, in */
__sfr __at 0x0E lpctrl;		/* Port C control outputs */
__sfr __at 0x0F setup;		/* 82C55 control */
#define PPA_PORT 0x0C
#endif

#define MGS_ACK		0x01		/* Acrtive low */
#define MGS_BUSY	0x02
#define MGS_PAPER	0x04
#define MGS_SEL		0x08
#define MGS_FAULT	0x10		/* Active low */

#define MGC_STROBE	0x01		/* All C active low */
#define MGC_AUTOFEED	0x02
#define MGC_INIT	0x04
#define MGC_SELIN	0x08

#define MGC_LED		0x80

/* The status translation is I believe

		PC		MG014
BUSY		7		1
ACK		6		0
PAPER		5		2
SELIN		4		3
ERROR		3		4

    The control lines match but on the PC are inverted except for C2 (Init)
*/

/* Location of printer port */
static uint16_t ppa_port = PPA_PORT;

int lpr_open(uint_fast8_t minor, uint16_t flag)
{
	if (minor == 0 && have_lp)
		return 0;
	udata.u_error = ENODEV;
	return -1;
}

int lpr_close(uint_fast8_t minor)
{
	return 0;
}

static int iopoll(int sofar)
{
	/* Ought to be a core helper for this lot ? */
	if (need_reschedule())
		_sched_yield();
	if (chksigs()) {
		if (sofar)
			return sofar;
		udata.u_error = EINTR;
		return -1;
	}
	return 0;
}

int lpr_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	uint8_t *p = udata.u_base;
	uint8_t *pe = p + udata.u_count;
	uint8_t r;
	int n;

	while (p < pe) {
		/* Printer busy ? */
		while ((r = lpstatus) & 2) {
			if ((n = iopoll(pe - p)) != 0)
				return n;
		}
		lpdata = ugetc(p++);
		/* Drive strobe low/high */
		lpctrl &= 0xFE;
		lpctrl |= 1;
	}
	return pe - p;
}

int lpr_ioctl(uint_fast8_t minor, uarg_t arg, char *ptr)
{
	uint8_t s, r = 0;

	used(ptr);

	if (arg == LPIOCSTAT) {
	        s = lpstatus;
                /* TODO check polarity versus IBM */
		if (!(s & 0x01))
			r = LP_ACK;
		if (s & 0x02)
			r |= LP_BUSY;
		if (!(s & 0x10))
			r |= LP_ERROR;
		if (s & 0x08)
			r |= LP_SELECT;
		if (s & 0x04)
			r |= LP_PAPER;
		return r;
	}
	return -1;
}

/*
 *	ZIP drive glue.
 *
 *	Translate between MG014 and PC bit patterns
 */
 
void ppa_write_ctrl(uint8_t v)
{
	unsigned i;
	/* PC inverts all but C2 on the bus, we don't. Invert them
	   in advance */
	v^= 0x0B;
	lpctrl = v;
	for (i = 0; i < 50; i++);	/* For the moment */
}

uint8_t ppa_read_status(void)
{
	uint8_t r = lpstatus;
	uint8_t s = 0;
	/* Now play musical bits */
	if (r & MGS_ACK)
		s |= 0x40;
	if (!(r & MGS_BUSY))
		s |= 0x80;
	if (r & MGS_PAPER)
		s |= 0x20;
	if (r & MGS_SEL)
		s |= 0x10;
	if (r & MGS_FAULT)
		s |= 0x08;
	return s;
}

void ppa_write_data(uint8_t d)
{
	unsigned i;
	lpdata = d;
	for (i = 0; i < 50; i++);	/* For the moment */
}

/*
 *	Probe - wants splitting into a discard section
 */
void lpr_init(void)
{
	/* Not an 82C55A */
	if (setup != 0x9B)
		return;
	setup = 0x82;		/* A out, C low out */
	kputs("82C55 printer port at 0x0C\n");
	have_lp = 1;
	ppa_init();
}

COMMON_MEMORY

#ifdef CONFIG_RC2014_EXTREME

void ppa_block_read(void) __naked
{
    __asm
	    pop bc
	    pop de
	    pop hl
	    push hl
	    push de
	    push bc
            ld a, (_td_raw)
                                                    ; and count
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapin
            ld a, (_td_page)
            call map_for_swap
            jr doread
not_swapin:
#endif
            or a                                    ; test is_user
            jr z, rd_kernel
            call map_proc_always  	            ; map user memory first if required
            jr doread
rd_kernel:
            call map_buffers
doread:
	    ld b, #0x00				    ; count
	    exx
	    ld de, #0x0F0D			    ; clock toggles
            ld bc, (_ppa_port)	                    ; setup port number
	    inc b
	    inc b				    ; 0x0EB8
            ld hl, #lpxlate4			    ; must be page aligned
	    exx
read_loop:
	    exx
	    out (c),d				    ; 0x0EB8 control to 0x0F
	    dec b
	    in a,(c)				    ; Status 0x0DB8
	    inc b
	    and #0x0f
	    or #0x10				    ; Upper table for hig bits
	    ld l,a
	    ld d,(hl)				    ; Look up table for speed
	    out (c),e				    ; 0x0EB8 control to 0x0D
	    dec b
	    in a,(c)				    ; Same again for lower bits
	    inc b				    ; 0x0DB8
	    and #0x0f
	    ld l,a
	    ld a,(hl)
	    or d
	    ; Put back the clock toggle we have to reuse
	    ld d,#0x0F
	    exx

	    ld (hl),a				    ; Write byte to memory
	    inc hl
	    
	    ; Same again
	    exx
	    out (c),d				    ; 0x0EB8 to 0x0F
	    dec b
	    in a,(c)				    ; Status 0x0DB8
	    inc b
	    and #0x0f
	    or #0x10				    ; Upper table for high bits
	    ld l,a
	    ld d,(hl)				    ; Look up table for speed
	    out (c),e				    ; 0x0EB8 to 0x0D
	    dec b
	    in a,(c)				    ; Same again for lower bits
	    inc b				    ; from 0x0DB8 
	    and #0x0f
	    ld l,a
	    ld a,(hl)
	    or d
	    ; And put the clock toggle back again
	    ld d,#0x0F
	    exx

	    ld (hl),a				    ; Write byte to memory
	    inc hl

	    ; Do it until we have 512 bytes
	    djnz read_loop
	    ld bc,(_ppa_port)
	    inc b
	    inc b
	    ld a,#0x07
	    out (c),a
            jp map_kernel_restore                   ; else map kernel then return
    __endasm;
}

void ppa_block_write(void) __naked
{
    __asm
	    pop bc
	    pop de
	    pop hl
	    push hl
	    push de
	    push bc
            ld a, (_td_raw)
            ld bc, (_ppa_port)	                    ; setup port number
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapout
            ld a, (_td_page)
            call map_for_swap
            jr dowrite
not_swapout:
#endif
            or a                                    ; test is_user
            jr z, wr_kernel
            call map_proc_always                 ; else map user memory first if required
            jr dowrite
wr_kernel:
            call map_buffers
dowrite:
	    ld de,#7
write_loop:
	    outi				    ; Data on bus
	    inc b				    ; Undo outi
	    ld a,#0x05				    ; STROBE|AUTOFEED
	    inc b
	    inc b
	    out	(c),a
	    ld a,e				    ; STROBE (E is 7)
	    out (c),a
	    dec b
	    dec b
	    outi
	    inc b				    ; Undo outi
	    inc b
	    inc b
	    ld a,#0x05
	    out (c),a
	    ld a,e				    ; E is 7
	    out (c),a
	    dec b
	    dec b
	    dec d
	    jr nz, write_loop			    ; 512 times
            jp map_kernel_restore                   ; else map kernel then return
    __endasm;
}

#else

void ppa_block_read(void) __naked
{
    __asm
	    pop bc
	    pop de
	    pop hl
	    push hl
	    push de
	    push bc
            ld a, (_td_raw)
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapin
            ld a, (_td_page)
            call map_for_swap
            jr doread
not_swapin:
#endif
            or a                                    ; test is_user
            jr z, rd_kernel
            call map_proc_always  	            ; map user memory first if required
            jr doread
rd_kernel:
            call map_buffers
doread:
	    ld b, #0x00				    ; count
	    exx
	    ld de, #0x0F0D			    ; clock toggles
            ld bc, (_ppa_port)	                    ; setup port number
	    inc c
	    inc c
            ld hl, #lpxlate4			    ; must be page aligned
	    exx
read_loop:
	    exx
	    out (c),d
	    dec c
	    in a,(c)				    ; Status
	    inc c
	    and #0x0f
	    or #0x10				    ; Upper table for hig bits
	    ld l,a
	    ld d,(hl)				    ; Look up table for speed
	    out (c),e
	    dec c
	    in a,(c)				    ; Same again for lower bits
	    inc c
	    and #0x0f
	    ld l,a
	    ld a,(hl)
	    or d
	    ; Put back the clock toggle we have to reuse
	    ld d,#0x0F
	    exx

	    ld (hl),a				    ; Write byte to memory
	    inc hl
	    
	    ; Same again
	    exx
	    out (c),d
	    dec c
	    in a,(c)				    ; Status
	    inc c
	    and #0x0f
	    or #0x10				    ; Upper table for high bits
	    ld l,a
	    ld d,(hl)				    ; Look up table for speed
	    out (c),e
	    dec c
	    in a,(c)				    ; Same again for lower bits
	    inc c
	    and #0x0f
	    ld l,a
	    ld a,(hl)
	    or d
	    ; And put the clock toggle back again
	    ld d,#0x0F
	    exx

	    ld (hl),a				    ; Write byte to memory
	    inc hl

	    ; Do it until we have 512 bytes
	    djnz read_loop
	    ld bc,(_ppa_port)
	    inc c
	    inc c
	    ld a,#0x07
	    out (c),a
            jp map_kernel_restore                   ; else map kernel then return
    __endasm;
}

void ppa_block_write(void) __naked
{
    __asm
	    pop bc
	    pop de
	    pop hl
	    push hl
	    push de
	    push bc
            ld a, (_td_raw)
            ld bc, (_ppa_port)	                    ; setup port number
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapout
            ld a, (_td_page)
            call map_for_swap
            jr dowrite
not_swapout:
#endif
            or a                                    ; test is_user
            jr z, wr_kernel
            call map_proc_always                 ; else map user memory first if required
            jr dowrite
wr_kernel:
            call map_buffers
dowrite:
	    ld de,#7
write_loop:
	    outi				    ; Data on bus
	    inc b				    ; Undo outi
	    ld a,#0x05				    ; STROBE|AUTOFEED
	    inc c
	    inc c
	    out	(c),a
	    ld a,e				    ; STROBE (E is 7)
	    out(c),a
	    dec c
	    dec c
	    outi
	    inc b				    ; Undo outi
	    inc c
	    inc c
	    ld a,#0x05
	    out(c),a
	    ld a,e				    ; E is 7
	    out(c),a
	    dec c
	    dec c
	    dec d
	    jr nz, write_loop			    ; 512 times
            jp map_kernel_restore                   ; else map kernel then return
    __endasm;
}

#endif
