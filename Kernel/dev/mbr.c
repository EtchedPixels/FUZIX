/* 2015-01-04 Will Sowerbutts */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <blkdev.h>
#include "mbr.h"
#include "gpt.h"

#ifdef CONFIG_DYNAMIC_SWAP
/*
 *	This function is called for partitioned devices if a partition is found
 *	and marked as swap type. The first one found will be used as swap. We
 *	only support one swap device.
 */
static void mbr_swap_found(uint8_t letter, uint8_t m)
{
  blkdev_t *blk = blk_op.blkdev;
  uint16_t n = 0;
  uint32_t off;
  if (swap_dev != 0xFFFF)
    return;
  letter -= 'a';
  kputs("(swap) ");
  swap_dev = letter << 4 | m;
  off = blk->lba_count[m - 1];
  /* Avoid dragging in 32bit divide libraries for one use only */
  while(off >= SWAP_SIZE && n < MAX_SWAPS) {
    off -= SWAP_SIZE;
    n++;
  }
  /* Partition swap is 0 based */
  while(n)
    swapmap_init(--n);
}
#endif

#ifdef CONFIG_DYNAMIC_PAGE
/*
 *	This function is called for partitioned devices if a partition is found
 *	and marked as swap type. The first one found will be used as swap. We
 *	only support one swap device.
 */
static void mbr_swap_found(uint8_t letter, uint8_t m)
{
  blkdev_t *blk = blk_op.blkdev;
  uint16_t n = 0;
  uint32_t off;
  if (swap_dev != 0xFFFF)
    return;
  letter -= 'a';
  kputs("(page) ");
  swap_dev = letter << 4 | m;
  off = blk->lba_count[m - 1];
  pagefile_add_blocks(off);
}
#endif

/* FIXME: Needs to be int so we can call multiple and get the right type */
void mbr_parse(uint_fast8_t letter)
{
    boot_record_t *br;
    uint_fast8_t i, seen = 0;
    uint32_t ep_offset = 0, br_offset = 0;
    uint_fast8_t next = 0;

    kprintf("hd%c: ", letter);

    /* allocate temporary memory */
    br = (boot_record_t *)tmpbuf();

    blk_op.is_read = true;
    blk_op.is_user = false;
    blk_op.addr = (uint8_t *)br;
    blk_op.lba = 0;

    do{
        blk_op.nblock = 1;
        if(!blk_op.blkdev->transfer() || le16_to_cpu(br->signature) != MBR_SIGNATURE){
#ifdef CONFIG_MBR_OFFSET
            if (blk_op.lba == 0) {
                /* failed to find MBR on block 0. Go round again but this time
                   look at the fall-back location for this badly-behaved media
                */
                blk_op.lba = CONFIG_MBR_OFFSET;
                continue;
            }
#endif
	    break;
        }

#ifndef BOOTDEVICE
        /* Valid first MBR, look for boot command string */
        if (seen == 0) {
		if (le16_to_cpu(br->cmdflag) == MBR_BOOT_CMD)
			set_boot_line((const char *)br->cmdline);
	}
#endif

	/* avoid an infinite loop where extended boot records form a loop */
	if(seen >= 50)
	    break;

	if(seen == 1){
	    /* we just loaded the first extended boot record */
	    ep_offset = blk_op.lba;
	    next = 4;
	    kputs("< ");
	}

	br_offset = blk_op.lba;
        blk_op.lba = 0;

	for(i=0; i<MBR_ENTRY_COUNT && next < MAX_PARTITIONS; i++){
	    uint8_t t = br->partition[i].type_chs_last[0];
	    switch(t) {
#ifdef CONFIG_GPT
		case MBR_GPT_PROTECTED_TYPE:
		    /* TODO assert next is zero (unless hybrid...) */
		    parse_gpt((uint8_t *) br, i);
		    goto out;
#endif
		case 0:
		    break;
		case 0x05:
		case 0x0f:
		case 0x85:
		    /* Extended boot record, or chained table; in principle a drive should contain
		       at most one extended partition so this code is OK even for parsing the MBR.
		       Chained EBR addresses are relative to the start of the extended partiton. */
		    blk_op.lba = ep_offset + le32_to_cpu(br->partition[i].lba_first);
		    if(next >= 4)
			break;
		    /* we include all primary partitions but we deliberately knobble the size in
		       order to prevent catastrophic accidents */
		    br->partition[i].lba_count = cpu_to_le32(2L);
		    /* fall through */
		default:
		    /* Regular partition: In EBRs these are relative to the EBR (not the disk, nor
		       the extended partition) */
		    blk_op.blkdev->lba_first[next] = br_offset + le32_to_cpu(br->partition[i].lba_first);
		    blk_op.blkdev->lba_count[next] = le32_to_cpu(br->partition[i].lba_count);
		    next++;
		    kprintf("hd%c%d ", letter, next);
#if defined(CONFIG_DYNAMIC_SWAP) || defined(CONFIG_DYNAMIC_PAGE)
		    if(t == FUZIX_SWAP)
			mbr_swap_found(letter, next);
#endif
	    }
	}
	seen++;
    }while(blk_op.lba);

    if(ep_offset && next >= 4)
	kputs("> ");

#ifdef CONFIG_GPT
out:
#endif
    /* release temporary memory */
    tmpfree(br);
}

