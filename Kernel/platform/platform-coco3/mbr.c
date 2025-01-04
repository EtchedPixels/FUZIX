#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <tinydisk.h>

typedef struct {
	uint8_t flags;
	uint8_t type;
	uint16_t start_h;
	uint32_t start;
	uint16_t len_h;
	uint32_t len;
	uint8_t res[2];
} partition_table_entry_t;



#define MBR_ENTRY_COUNT 14
#define MBR_SIGNATURE 0x43435054
typedef struct {
	uint32_t magic;		// magic no. "CCPT", big endian.
	uint16_t sz_h;		// high word of size
	uint32_t sz;		// low long of size
	uint8_t secz;		// sector size
	uint16_t cyls;		// number of cylindars
	uint16_t heads;		// number of heads
	uint16_t secs;		// number of sectors
	uint16_t crc;		// CRC of table
	uint8_t res[13];	// reserved
	partition_table_entry_t partition[MBR_ENTRY_COUNT];
} boot_record_t;


uint_fast8_t td_plt_setup(uint_fast8_t dev, uint32_t *lba, void *buf)
{
	boot_record_t *br = buf;
	uint8_t i, k = 0;
	uint32_t tstart;
	uint32_t tlen;

	udata.u_block = 0;
	udata.u_nblock = 1;
	udata.u_dptr = (void *) br;

	/* FIX: should also check table's CRC */
	if (td_read(dev << 4, 0, 0) != BLKSIZE || br->magic != MBR_SIGNATURE)
		return 2;		/* Try PC format */

	kprintf("hd%c: ", dev + 'a');
	/* add each entry to blkops struct */
	/*  This adds paritions as it finds good ones? */
	for (i = 0; i < MBR_ENTRY_COUNT; i++) {
		if (!br->partition[i].flags) {
			continue;
		}
		/* if valid entry, then adjust offset/len to blocks */
		else {
			tstart = br->partition[i].start;
			if (!br->secz)
				tstart >>= 1;
			else
				tstart <<= (br->secz - 1);
			*++lba = tstart;
			k++;
			kprintf("hd%c%d ", dev + 'a', k);
			/* TODO: swap on CCPT */
			if (k == MAX_PART)
				break;
		}
	}
	return 1;	/* Found it, don't scan further */
}
