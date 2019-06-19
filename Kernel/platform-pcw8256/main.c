#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>

uint16_t ramtop = 0xC000;

/* Kernel is 0-3 screen for now is 4 and bits of 5
   Apps 6,7,8,9,10,11,12,13,14,15 etc
   
   This *will* change once we sort the memory map out sanely so that we are
   only using 4 for kernel/screen  */

void map_init(void)
{
 udata.u_ptab->p_page = 0x8383;
 udata.u_ptab->p_page2 = 0x8383;
}

void pagemap_init(void)
{
 int i;
 for (i = 0x86; i < 0x92; i++)
  pagemap_add(i);
}

/* The uarea is already synched to the stash which is written with the
   process */
uint8_t *swapout_prepare_uarea(ptptr p)
{
//  kprintf("swapout prepare %x\n", p);
  p;
  return NULL;
}

/* The switchin code will move the uarea into the process itself, we just
   need to fix up the u_page pointer */
uint8_t *swapin_prepare_uarea(ptptr p)
{
  p;
//  kprintf("swapin prepare %x now page %d\n", p, p->p_page);
  return NULL;
}

void platform_idle(void)
{
 __asm
  halt
 __endasm;
}

uint8_t platform_param(char *p)
{
    used(p);
    return 0;
}
