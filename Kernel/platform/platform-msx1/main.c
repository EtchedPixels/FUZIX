#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <tinydisk.h>
#include <devide_sunrise.h>
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

extern uint8_t *disk_dptr;
extern uint8_t disk_rw;
extern uint32_t disk_lba;

struct blkbuf *bufpool_end = bufpool + NBUFS;

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
    unsigned n = 0;
    /* Until we tackle the buffers. When we do we will reserve 512 bytes
       at the end (probably FDFE-FFFE (FFFF being magic)) */
    bouncebuffer = (uint8_t *)0xFDFE;	/* Hack for now */
    while(bufpool_end + 1 <= (void *)bouncebuffer) {
        memset(bufpool_end, 0, sizeof(*bufpool_end));
        bufpool_end->bf_dev = NO_DEVICE;
        bufpool_end->bf_busy = BF_FREE;
        bufpool_end++;
        n++;
    }
    kprintf("%d buffers added (end %p)\n", n,  bufpool_end);
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
    uint8_t *addr = disk_dptr;
    uint8_t old_user = td_raw;

    /* Shortcut: this range can only occur for a user mode I/O */
    if (addr > (uint8_t *)0x3E00U && addr < (uint8_t *)0x8000U) {
        /* We can't just use tmpbuf because the buffer might be dirty which would
           trigger a recursive I/O and then badness happens */
        disk_dptr = bouncebuffer;
        td_raw = 0;
//      kprintf("bounced do_xfer %p %d:", addr, disk_rw);
        if (disk_rw == 1) {	/* Read */
            if (xferfunc(arg))
                goto fail;
            uput(disk_dptr, addr, 512);
        } else {
            uget(addr, disk_dptr, 512);
            if (xferfunc(arg))
                goto fail;
        }
//      kprintf("bounced done.\n");
        disk_dptr = addr;
        td_raw = old_user;
        return 1;
    }
//  kprintf("do_xfer %d %p..", td_raw, addr);
    if (xferfunc(arg) == 0) {
//      kputs("done.\n");
        return 1;
    }
fail:
    disk_dptr = addr;
    td_raw = old_user;
    return 0;
}
