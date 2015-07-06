/* 2015-01-04 Will Sowerbutts */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <blkdev.h>

typedef struct {
	uint8_t  flags;
	uint8_t  type;
	uint16_t start_h;
	uint32_t start;
	uint16_t len_h;
	uint32_t len;
	uint8_t  res[2];
} partition_table_entry_t;



#define MBR_ENTRY_COUNT 14
#define MBR_SIGNATURE 0x43435054 
typedef struct {
	uint32_t  magic;  // magic no. "CCPT", big endian.
	uint16_t  sz_h;   // high word of size
	uint32_t  sz;     // low long of size
	uint8_t   secz;   // sector size
	uint16_t  cyls;    // number of cylindars
	uint16_t  heads;  // number of heads
	uint16_t  secs;   // number of sectors
	uint16_t  crc;    // CRC of table
	uint8_t   res[13]; // reserved
	partition_table_entry_t partition[MBR_ENTRY_COUNT];
} boot_record_t ;


void mbr_parse(char letter)
{
	boot_record_t *br;
	uint8_t i,k = 0;
	
	kprintf("hd%c: ", letter);
	
	/* allocate temporary memory */
	br = (boot_record_t *)tmpbuf();
	
	blk_op.is_read = true;
	blk_op.is_user = false;
	blk_op.addr = br;
	blk_op.lba = 0;
	blk_op.nblock = 1;
	
	/* FIX: should also check table's CRC */
	if(!blk_op.blkdev->transfer() || br->magic != MBR_SIGNATURE){
		kputs("No CCPT");
		return;
	}
	
	/* add each entry to blkops struct */
	/*  This adds paritions as it finds good ones? */
	for(i=0; i<MBR_ENTRY_COUNT; i++){
		if( ! br->partition[i].flags ){
			continue;
		}
		else{
			blk_op.blkdev->lba_first[k] = br->partition[i].start;
			blk_op.blkdev->lba_count[k] = br->partition[i].len;
			kprintf("hd%c%d ", letter, k+1);
			k++;
		}
	}
		
	/* release temporary memory */
	brelse((bufptr)br);
}
