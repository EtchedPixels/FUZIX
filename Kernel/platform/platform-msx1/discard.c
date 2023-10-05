#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>
#include <msx.h>

#undef DEBUG

extern uint8_t ramtab[], kernel_map[], current_map[], user_map[];
extern uint8_t subslots;
extern uint16_t vdpport;
extern uint8_t vdptype;

void map_init(void)
{
  uint8_t *bp = kernel_map;
  uint8_t *kp;
  uint8_t *rp;
  uint8_t i;
  uint8_t pp;
#ifdef DEBUG
  uint8_t cp;


  cp = bp[5] & 3;

  kprintf("Subslots %2x\n", subslots);
  kprintf("Cartridge in slot %d", cp);
  if (subslots &  (1 << cp))
    kprintf(".%d", bp[cp] & 3);
  kprintf("\nKernel map %2x %2x %2x %2x %2x %2x\n",
    *bp, bp[1], bp[2], bp[3], bp[4], bp[5]);
    
  bp = current_map;
  kprintf("Current map %2x %2x %2x %2x %2x %2x\n",
    *bp, bp[1], bp[2], bp[3], bp[4], bp[5]);

  bp = ramtab;
  kprintf("RAM reported at %2x:%2x %2x:%2x %2x:%2x\n",
    *bp, bp[1], bp[2], bp[3], bp[4], bp[5]);
#endif

  bp = user_map + 1;
  rp = ramtab + 4;
  pp = 0;
  
  memcpy(user_map, kernel_map, 6);

  /* Compute the user mapping from the top down */
  /* TODO: when given a choice of RAM banks for a 16K range pick one that
     is not subslotted */
  for (i = 0; i < 3; i++) {
    /* We found no RAM for this 16K block - fail */
    if (*rp == 0xFF)
      panic("can't find RAM");
    /* This is subslotted. Add this subslot to the subslots we must
       program, and remember the subslot info */
    if (rp[1] != 0xFF) {
      bp[*rp] = (rp[1] << 4) | (rp[1] << 2) | rp[1] | (kernel_map[*rp + 1] & 0xC0);
      user_map[0] |= 1 << *rp;
    }
    kp--;
    pp <<= 2;
    pp |= *rp;
    rp -= 2;
  }
  /* Slot map with the common from the kernel map */
  user_map[5] = pp | (kernel_map[5] & 0xC0);
#ifdef DEBUG
  bp = user_map;
  kprintf("User map %2x %2x %2x %2x %2x %2x\n",
    *bp, bp[1], bp[2], bp[3], bp[4], bp[5]);
#endif
  copy_vectors();
  for (i = 0; i < 4; i++){
    for (pp = 0; pp < 4; pp++) {
      if (devtab[i][pp][1] & 0x8000)
        kprintf("@0x4000: %d.%d: %2x\n", i, pp, devtab[i][pp][1] & 0x7FFF);
    }
  }
  for (i = 0; i < 4; i++){
    for (pp = 0; pp < 4; pp++) {
      if (devtab[i][pp][2] & 0x8000)
        kprintf("@0x8000: %d.%d: %2x\n", i, pp, devtab[i][pp][2] & 0x7FFF);
    }
  }
}
