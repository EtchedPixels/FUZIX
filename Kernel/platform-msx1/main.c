#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
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
#if 0
    /* Until we tackle the buffers. When we do we will reserve 512 bytes
       at the end (probably FDFE-FFFE (FFFF being magic)) */
    bouncebuffer = (uint8_t *)0xFD00;	/* Hack for now */
#endif    
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
 *	Our I/O is mostly weird and memory mapped at 0x4000-0x7FFF which due
 *	to the horizontal memory mapper means some ranges cannot be direct
 *	I/O. We use this function as our I/O direct check and veto anything
 *	that would be a problem. udata holds the address and the length is
 *	always 512 bytes
 */

uint8_t direct_io_range(uint16_t dev)
{
        if (udata.u_dptr >= 0x8000)
          return 1;
        if (udata.u_dptr <= 0x3E00)
          return 1;
        return 0;
}
