#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <blkdev.h>

typedef struct {
    uint8_t  status;
    uint8_t  chs_first[3];
    uint8_t  type;
    uint8_t  chs_last[3];
    uint32_t lba_first;
    uint32_t lba_count;
} partition_table_entry_t;

#define MBR_ENTRY_COUNT 4
#define MBR_SIGNATURE 0xAA55
typedef struct {
    uint8_t bootcode[446];
    partition_table_entry_t partition[MBR_ENTRY_COUNT];
    uint16_t signature;
} boot_record_t;

void mbr_parse(blkdev_t *blk, char letter)
{
    boot_record_t *br;
    uint8_t i, maxbr = 50;
    uint32_t lba = 0, ep_offset = 0, br_offset = 0;
    uint8_t next = 0;

    /* allocate temporary memory */
    br = (boot_record_t *)tmpbuf();

    do{
	if(!blk->transfer(blk->drive_number, lba, br, true) || br->signature != MBR_SIGNATURE)
	    break;

	/* avoid an infinite loop where extended boot records form a loop */
	if(--maxbr == 0)
	    break;

	if(next < 4 && lba != 0){ 
	    /* we just loaded the first extended boot record */
	    ep_offset = lba;
	    next = 4;
	    kputs("< ");
	}

	br_offset = lba;
	lba = 0;

	for(i=0; i<MBR_ENTRY_COUNT && next < MAX_PARTITIONS; i++){
	    switch(br->partition[i].type){
		case 0:
		    break;
		case 0x05:
		case 0x0f:
		case 0x85:
		    /* Extended boot record, or chained table; in principle a drive should contain
		       at most one extended partition so this code is OK even for parsing the MBR.
		       Chained EBR addresses are relative to the start of the extended partiton. */
		    lba = ep_offset + br->partition[i].lba_first;
		    if(next >= 4)
			break;
		    /* we include all primary partitions but we deliberately knobble the size in 
		       order to prevent catastrophic accidents */
		    br->partition[i].lba_count = 2;
		    /* fall through */
		default:
		    /* Regular partition: In EBRs these are relative to the EBR (not the disk, nor
		       the extended partition) */
		    blk->lba_first[next] = br_offset + br->partition[i].lba_first;
		    blk->lba_count[next] = br->partition[i].lba_count;
		    kprintf("hd%c%d ", letter, 1+next);
		    next++;
	    }
	}
    }while(lba);

    if(next >= 4)
	kputs("> ");

    /* release temporary memory */
    brelse((bufptr)br);
}
