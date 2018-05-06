/* 2015-01-04 Will Sowerbutts */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <blkdev.h>
#include <config.h>
#include "mbr.h"
#include "gpt.h"

void mbr_parse(char letter)
{
    boot_record_t *br;
    uint8_t i, seen = 0;
    uint32_t ep_offset = 0, br_offset = 0;
    uint8_t next = 0;

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
		    // TODO assert next is zero (unless hybrid...)
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
#ifdef CONFIG_DYNAMIC_SWAP
		    if(t == FUZIX_SWAP)
		    	platform_swap_found(next - 1);
#endif
	    }
	}
	seen++;
    }while(blk_op.lba);

    if(ep_offset && next >= 4)
	kputs("> ");

out:
    /* release temporary memory */
    tmpfree(br);
}
