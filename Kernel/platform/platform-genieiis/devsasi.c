/*
 *	SASI controller
 *
 *	TODO: is data inverted - unclear
 *
 *	This is a fairly minimal interface but actually does most of the work for us
 *
 *	When we read data the controller pulses ack for us, it also provides select and
 *	reset as pulses. We appear to have to poll REQUEST ourselves
 */

#include <kernel.h>
#include <tinyscsi.h>
#include "plt_scsi.h"

__sfr __at 0x00 sasi_data;
__sfr __at 0x01 sasi_status;
__sfr __at 0x01 sasi_reset;
__sfr __at 0x02 sasi_select;

void scsi_reset(void)
{
    sasi_reset = 1;
    /* Wait a while */
    timer = set_timer_ms(3000);
    while(!timer_expired(timer));
}

int scsi_cmd(uint_fast8_t dev, uint8_t *cmd. uint8_t *data, uint16_t len)
{
    uint16_t n;

    sasi_data = 1 << dev;
    sasi_select = 1;
    while(timeout && !(sasi_status & I_BUSY))
        timeout--;
    if (timeout == 0)
        return 1;
    while(sasi_status & I_BUSY) {
        switch(sasi_status & (I_IO|I_CMD|I_MSG)) {
        case 0:	/* Data to device */
            n = sasi_xfer_out(data, len);
            len -= n;
            data += n;
            break;
        case I_IO: /* Data from device */
            n = sasi_xfer_in(data, len);
            len -= n;
            data += n;
            break;
        case I_CMD: /* Command byte request */
            /* Wait for REQ */
            while(timeout && !(sasi_status & I_REQ));
            if (timeout == 0)
                return 1;
            sasi_data = *cmd++;
            /* This drives ACK low automatically */
            break;
        case I_CMD|I_IO: /* Status */
            if (ptr < scsi_status + 2)
                *ptr++ = sasi_data;
            break;
        default:
            kprintf("scsi%d: bad bus state %2x.\n", sasi_status);
            return 1;
        }
    }    
    /* Completed */
    return 0;
}

/* asm helpers - need to be common for the user mode cases */

COMMON_MEMORY

/* Might be worth optimizing for 512 byte blocks */
static uint16_t sasi_xfer_out(uint8_t *data, uint16_t len) __naked
{
    __asm
        pop bc
        pop de
        pop hl
        push hl
        push de
        push bc
        ; HL is buffer, de len
        ; TODO - map the right thing
        ld a,(_td_raw)
        or	a
        jr	z, xfer_o_k
        dec	a
        jr	z, xfer_o_u
        ld	a,(_td_page)
        call	map_process_a
        jr	xfer_o_k
xfer_o_u:
        call	map_process_always
xfer_o_k:
        push	de
        ld	c,#0x00
outw:
        in	a,(0x01)
        cp	#0x02		; BUSY only 
        jr	z, outw
        cp	#0x03		; BUSY, REQ
        jr	nz, ophase_bad
        ; Controller still wants feeding
        outi
        dec	de
        ld	a,d
        or	e
        jr	nz, outw
ophase_bad:
        pop	hl 
        or	a
        sbc	hl,de		; work out how much we sent
        jp	map_kernel	; and fix the mapping
     
    __endasm; 
}

static uint16_t sasi_xfer_in(uint8_t *data, uint16_t len) __naked
{
    __asm
        pop bc
        pop de
        pop hl
        push hl
        push de
        push bc
        ; HL is buffer, de len
        ; TODO - map the right thing
        ld a,(_td_raw)
        or	a
        jr	z, xfer_i_k
        dec	a
        jr	z, xfer_i_u
        ld	a,(_td_page)
        call	map_process_a
        jr	xfer_i_k
xfer_i_u:
        call	map_process_always
xfer_i_k:
        push de
        ld c,#0x00
inr:
        in a,(0x01)
        cp #0x12		; BUSY IO
        jr z, inr
        cp #0x13		; BUSY IO REQ
        jr nz, iphase_bad
        ; Controller still wants to feed us
        ini
        dec de
        ld a,d
        or e
        jr nz, inr
iphase_bad:
        pop hl 
        or a
        sbc hl,de		; work out how much we read
        jp map_kernel		; and fix the mapping
    __endasm; 
}
