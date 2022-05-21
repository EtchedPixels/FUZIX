#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>
#include <devide.h>
#include <z280.h>

uint16_t ramtop = PROGTOP;
uint16_t swap_dev = 0xFFFF;

/*
 *	This routine is called continually when the machine has nothing else
 *	it needs to execute. On a machine with entirely interrupt driven
 *	hardware this could just halt for interrupt.
 */
void plt_idle(void)
{
	/* Disable interrupts so we don't accidentally process a polled tty
	   and interrupt call at once and make a mess */
	irqflags_t irq = di();
	tty_poll();
	/* Restore prior state. */
	irqrestore(irq);
}

/*
 *	This routine is called from the interrupt handler code to process
 *	interrupts. All of the nasty stuff (register saving, bank switching,
 *	reti instructions) is dealt with for you.
 */
void plt_interrupt(void)
{
	tty_poll();
	timer_interrupt();
}

/* This points to the last buffer in the disk buffers. There must be at least
   four buffers to avoid deadlocks. */
struct blkbuf *bufpool_end = bufpool + NBUFS;

/*
 *	We pack discard into the memory image is if it were just normal
 *	code but place it at the end after the buffers. When we finish up
 *	booting we turn everything from the buffer pool to common into
 *	buffers. This blows away the _DISCARD segment.
 */
void plt_discard(void)
{
        /* We start our common block with the vectors as they must be 4K
           aligned */
	uint16_t discard_size = (uint16_t)vectors - (uint16_t)bufpool_end;
	bufptr bp = bufpool_end;

	discard_size /= sizeof(struct blkbuf);

	kprintf("%d buffers added\n", discard_size);

	bufpool_end += discard_size;

	memset( bp, 0, discard_size * sizeof(struct blkbuf) );

	for( bp = bufpool + NBUFS; bp < bufpool_end; ++bp ){
		bp->bf_dev = NO_DEVICE;
		bp->bf_busy = BF_FREE;
	}
}

/****************************************************************************/
/* The innermost part of the transfer routines has to live in common memory */
/* since it must be able to bank switch to the user memory bank.            */
/****************************************************************************/

/* IDE Port I/O: Z280 edition. We know the bank is the usual default 0 here */

COMMON_MEMORY

void devide_read_data(void) __naked
{
    __asm
            ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET) ; blkparam.is_user
            ld hl, (_blk_op+BLKPARAM_ADDR_OFFSET)   ; blkparam.addr
            ld bc, #IDE_REG_DATA                    ; setup port number
                                                    ; and count
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapin
            ld a, (_blk_op+BLKPARAM_SWAP_PAGE)	    ; blkparam.swap_page
            call map_for_swap
            jr swapin
not_swapin:
#endif
            or a                                    ; test is_user
            call nz, map_process_always             ; map user memory first if required
swapin:
;;            inirw                                   ; transfer 256 words
            .db 0xED, 0x92
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
            ld bc, #IDE_REG_DATA                    ; setup port number
                                                    ; and count
#ifdef SWAPDEV
	    cp #2
            jr nz, not_swapout
            ld a, (_blk_op+BLKPARAM_SWAP_PAGE)	    ; blkparam.swap_page
            call map_for_swap
            jr swapout
not_swapout:
#endif
            or a                                    ; test is_user
            call nz, map_process_always             ; else map user memory first if required
swapout:
;;            otirw                                   ; transfer 256 words
            .db 0xED, 0x93
            or a                                    ; test is_user
            ret z                                   ; done if kernel memory transfer
            jp map_kernel                           ; else map kernel then return
    __endasm;
}
