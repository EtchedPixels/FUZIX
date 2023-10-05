#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <timer.h>
#include <devide.h>
#include <blkdev.h>

__sfr __at 0x70 ide_dsr;

static uint8_t old_dsr;

void ide_select(uint8_t drive)
{
    if (drive > 1)
        ide_dsr = 0x81;		/* Powered on, second interface */
    else
        ide_dsr = 0x80;		/* Powered on, first interface */
    old_dsr = ide_dsr;
}

static void devide_delay(void)
{
    timer_t timeout;

    timeout = set_timer_ms(25);

    while(!timer_expired(timeout))
       plt_idle();
}

void devide_reset(void)
{
    ide_dsr = 0xC0;		/* Reset */
    devide_delay();
    ide_dsr = old_dsr;
}

void devide_read_data(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld b, #0                                ; setup count
            ld c, #IDE_REG_DATA                     ; setup port number
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapin
            ld a, (_blk_op+BLKPARAM_SWAP_PAGE)	    ; blkparam.swap_page
            call map_for_swap
            jr swapin
not_swapin:
#endif
            or a                                    ; test is_user
            call nz, map_proc_always                ; map user memory first if required
swapin:
            inir                                    ; transfer first 256 bytes
            inir                                    ; transfer second 256 bytes
            or a                                    ; test is_user
            ret z                                   ; done if kernel memory transfer
            jp map_kernel                           ; else map kernel then return
    __endasm;
}

void devide_write_data(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld b, #0                                ; setup count
            ld c, #IDE_REG_DATA                     ; setup port number
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapout
            ld a, (_blk_op+BLKPARAM_SWAP_PAGE)	    ; blkparam.swap_page
            call map_for_swap
            jr swapout
not_swapout:
#endif
            or a                                    ; test is_user
            call nz, map_proc_always                ; else map user memory first if required
swapout:
outloop:
            ;
            ; Invert the order for the latches
            ;
            ld a,(hl)
            inc hl
            ld e,(hl)
            out (c),e
            inc hl
            out (c),a
            djnz outloop
            or a                                    ; test is_user
            ret z                                   ; done if kernel memory transfer
            jp map_kernel                           ; else map kernel then return
    __endasm;
}
