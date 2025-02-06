/*-----------------------------------------------------------------------*/
/* Fuzix CH375 USB block device driver                                   */
/* 2025 Warren Toomey                                                    */
/*                                                                       */
/* This one is different from the one in Kernel/dev as it assumes that   */
/* the CH375 device will send interrupts.                                */
/*-----------------------------------------------------------------------*/

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <stdbool.h>
#include <blkdev.h>
#include <devch375.h>

#ifdef CONFIG_CH375

extern uint32_t ch375_read_block(uint8_t *buf, uint32_t lba);
extern uint32_t ch375_write_block(uint8_t *buf, uint32_t lba);

uint_fast8_t devch375_transfer_sector(void)
{
    bool success;
    uint32_t lba= blk_op.lba;

    /* Disable interrupts while we do the transfer */
    /* but keep IRQ5 enabled as we use it */
    irqflags_t irq = di();
    irqrestore(4);	/* IRQ4 is ignored, 5 is OK */

    if (blk_op.is_read) {
	success= ch375_read_block(blk_op.addr, lba);
    } else {
	success= ch375_write_block(blk_op.addr, lba);
    }
    irqrestore(irq);
    return(success);
}

#endif
