#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>
#include <msx.h>

/* These are set by the msx startup asm code */
extern uint16_t vdpport;
uint16_t infobits;
uint16_t swap_dev = 0xFFFF;
uint16_t ramtop = 0xC000;
uint8_t machine_type;
uint8_t vdptype;
uint8_t *bouncebuffer;
uint16_t devtab[4][4][3];

void plt_idle(void)
{
    __asm
    halt
    __endasm;
}

/* Some of this is discard stuff */

uint8_t plt_param(char *p)
{
    used(p);
    return 0;
}

void do_beep(void)
{
}

void plt_interrupt(void)
{
        uint8_t r = in((uint8_t)vdpport);
        if (r & 0x80) {
          wakeup(&vdpport);
  	  kbd_interrupt();
  	  timer_interrupt();
        }
}

void plt_discard(void)
{
    /* Until we tackle the buffers. When we do we will reserve 512 bytes
       at the end (probably FDFE-FFFE (FFFF being magic)) */
    bouncebuffer = (uint8_t *)0xFDFE;	/* Hack for now */
}

uint8_t device_find(const uint16_t *romtab)
{
  uint8_t slot, sub;
  while(*romtab) {
    for (slot = 0; slot < 4; slot++) {
      for (sub = 0; sub < 4; sub++) {
        if (*romtab == devtab[slot][sub][1]) {
          if (subslots & (1 << slot))
            return 0x80 | slot | (sub << 2);
          else
            return slot;
        }
      }
    }
    romtab++;
  }
  return 0xFF;
}

/*
 *	They joy of MSX - block I/O bounce handling
 *
 *	General purpose wrapper for blkdev devices with low level MMIO mapped in
 *	the 4000-7FFF range. We have to bounce accesses to 4000-7FFF. This differs
 *	from sane machines with MMIO windows or with I/O space mappings. MSX2 has
 *	the same disease but we are able to use the MSX2 mapper to move banks to
 *	other address ranges, something MSX1 cannot handle.
 */
unsigned blk_xfer_bounced(xferfunc_t xferfunc, uint16_t arg)
{
    uint8_t *addr = blk_op.addr;
    uint8_t old_user = blk_op.is_user;

    /* Shortcut: this range can only occur for a user mode I/O */
    if (addr >= (uint8_t *)0x3E00U && addr < (uint8_t *)0x8000U) {
        /* We can't just use tmpbuf because the buffer might be dirty which would
           trigger a recursive I/O and then badness happens */
        blk_op.addr = bouncebuffer;
        blk_op.is_user = 0;
//        kprintf("bounced do_xfer %p %x:", addr, mask);
        if (blk_op.is_read) {
            if (xferfunc(arg))
                goto fail;
            uput(blk_op.addr, addr, 512);
        } else {
            uget(addr, blk_op.addr, 512);
            if (xferfunc(arg))
                goto fail;
        }
//        kprintf("bounced done.\n");
        blk_op.addr = addr;
        blk_op.is_user = old_user;
        return 1;
    }
//    kprintf("do_xfer %d %p %x..", blk_op.is_user, addr, mask);
    if (xferfunc(arg) == 0) {
//        kputs("done.\n");
        return 1;
    }
fail:
    blk_op.addr = addr;
    blk_op.is_user = old_user;
    return 0;
}
