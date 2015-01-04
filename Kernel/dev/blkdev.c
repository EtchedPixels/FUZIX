#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <blkdev.h>
#include <mbr.h>

/*
   Minor numbers

   The kernel identifies our storage devices with an 8-bit minor number.

   The top four bits of the minor identify the drive, allowing a maximum of
   sixteen drives.

   The bottom four bits of the minor identify the partition number. Partition
   zero addresses "the whole drive", with no translation. Due to limitations
   within the kernel only the first 32MB can currently be accessed through
   partition zero.

   Partition zero is intended to be used primarily for writing a partition
   table to the drive, although it can also be used to store a single Fuzix
   filesystem on an unpartitioned disk.

   Partitions 1 through 15 are identified by reading a PC-style MBR partition
   table from the drive. The first four partitions are always the primary
   partitions on the drive, the remainder are logical partitions stored within
   an extended partition.

   To create the required device nodes, use:

       mknod /dev/hda 60660 0
       mknod /dev/hdb 60660 16
       mknod /dev/hdc 60660 32
       mknod /dev/hdd 60660 48
       ... (etc) ...
       mknod /dev/hdp 60660 240

       mknod /dev/hda1 60660 1
       mknod /dev/hda2 60660 2
       mknod /dev/hda3 60660 3
       ... (etc) ...
       mknod /dev/hda15 60660 15

       mknod /dev/hdb1 60660 17
       mknod /dev/hdb2 60660 18
       mknod /dev/hdb3 60660 19
       ... (etc) ...
       mknod /dev/hdb15 60660 31
*/

static blkdev_t blkdev_table[MAX_BLKDEV];

int blkdev_open(uint8_t minor, uint16_t flags)
{
    uint8_t drive, partition;

    flags; /* unused */

    drive = minor >> 4;
    partition = minor & 0x0F;

    if(drive < MAX_BLKDEV){
	if(blkdev_table[drive].drive_lba_count && (partition == 0 || blkdev_table[drive].lba_count[partition-1]))
	    return 0; /* that is a valid minor number */
    }

    /* sorry, can't find it */
    udata.u_error = ENODEV;
    return -1;
}

static int blkdev_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
    void *target;
    uint32_t lba;
    uint8_t partition;
    blkdev_t *blk;

    /* we trust that blkdev_open() has already verified that this minor number is valid */
    blk = &blkdev_table[minor >> 4];
    partition = minor & 0x0F;

    if(rawflag == 0) {
        target = udata.u_buf->bf_data;
        lba = udata.u_buf->bf_blk;
    }else
        goto xferfail;

    if(partition == 0){
	/* partition 0 is the whole disk and requires no translation */
	if(lba >= blk->drive_lba_count)
	    goto xferfail;
    }else{
	/* partitions 1+ require us to add in an offset */
	if(lba >= blk->lba_count[partition-1])
	    goto xferfail;

	lba += blk->lba_first[partition-1];
    }

    if(blk->transfer(blk->drive_number, lba, target, is_read))
	return 1; /* 10/10, would transfer sectors again */

xferfail:
    udata.u_error = EIO;
    return -1;
}

int blkdev_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag; /* not used */
    return blkdev_transfer(minor, true, rawflag);
}

int blkdev_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    flag; /* not used */
    return blkdev_transfer(minor, false, rawflag);
}

void blkdev_add(transfer_function_t transfer, uint8_t drive_number, uint32_t lba_count)
{
    char letter;
    blkdev_t *blk = NULL;

    letter = 'a';
    blk = &blkdev_table[0];

    /* find an empty slot */
    while(blk <= &blkdev_table[MAX_BLKDEV-1]){
	if(blk->drive_lba_count == 0){
	    blk->drive_lba_count = lba_count;
	    blk->drive_number = drive_number;
	    blk->transfer = transfer;

	    /* now we parse the partition table; might need a hook for platforms to
	       parse their platform-specific partition table first? eg P112 has its
	       own partitioning scheme */
	    kprintf("hd%c: ", letter);
	    mbr_parse(blk, letter);
	    kputchar('\n');
	    return;
	}
	blk++;
	letter++;
    }

    kputs("blkdev: full!\n");
}
