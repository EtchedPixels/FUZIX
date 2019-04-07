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

void platform_idle(void)
{
    __asm
    halt
    __endasm;
}

/* Some of this is discard stuff */

uint8_t platform_param(char *p)
{
    used(p);
    return 0;
}

void do_beep(void)
{
}

void platform_interrupt(void)
{
        uint8_t r = in((uint8_t)vdpport);
        if (r & 0x80) {
  	  kbd_interrupt();
  	  timer_interrupt();
        }
}

void platform_discard(void)
{
    /* Until we tackle the buffers. When we do we will reserve 512 bytes
       at the end (probably FDFE-FFFE (FFFF being magic)) */
    bouncebuffer = (uint8_t *)0xFD00;	/* Hack for now */
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
