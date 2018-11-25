#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>

extern uint8_t ramtab[], kernel_map[], current_map[], user_map[];
extern uint8_t subslots;
extern uint16_t vdpport;
extern uint8_t vdptype;

static const char *vdpnametab[] = {
  "TMS9918A",
  "V9938",
  "V9958"
};

void map_init(void)
{
  uint8_t *bp = kernel_map;
  uint8_t *kp;
  uint8_t *rp;
  uint8_t i;
  uint8_t pp;
  const char *vdpname = "??";
  
  if (vdptype < 3)
    vdpname = vdpnametab[vdptype];

  kprintf("VDP %s@%x\n", vdpname, vdpport);  
  kprintf("Subslots %x\n", subslots);

  kprintf("Kernel map %x %x %x %x %x %x\n",
    *bp, bp[1], bp[2], bp[3], bp[4], bp[5]);
    
  bp = current_map;
  kprintf("Current map %x %x %x %x %x %x\n",
    *bp, bp[1], bp[2], bp[3], bp[4], bp[5]);

  bp = ramtab;
  kprintf("RAM reported at %x:%x %x:%x %x:%x\n",
    *bp, bp[1], bp[2], bp[3], bp[4], bp[5]);

  bp = user_map + 1;
  rp = ramtab + 4;
  pp = 0;
  
  memcpy(user_map, kernel_map, 6);

  /* Compute the user mapping from the top down */
  for (i = 0; i < 3; i++) {
    /* We found now RAM for this 16K block - fail */
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
  bp = user_map;
  kprintf("User map %x %x %x %x %x %x\n",
    *bp, bp[1], bp[2], bp[3], bp[4], bp[5]);
}


void platform_discard(void)
{
    /* Until we tackle the buffers */
}

/*
 *	This function is called for partitioned devices if a partition is found
 *	and marked as swap type. The first one found will be used as swap. We
 *	only support one swap device.
 */
void platform_swap_found(uint8_t letter, uint8_t m)
{
  blkdev_t *blk = blk_op.blkdev;
  uint16_t n;
  if (swap_dev != 0xFFFF)
    return;
  letter -= 'a';
  kputs("(swap) ");
  swap_dev = letter << 4 | m;
  n = blk->lba_count[m - 1] / SWAP_SIZE;
  if (n > MAX_SWAPS)
    n = MAX_SWAPS;
  while(n)
    swapmap_init(n--);
}
