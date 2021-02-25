/* 2015-01-04 Will Sowerbutts */

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
struct blkparam blk_op;

int blkdev_open(uint_fast8_t minor, uint16_t flags)
{
    uint_fast8_t drive, partition;

    used(flags);

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

static int translate_lba(uint_fast8_t minor)
{
    uint_fast8_t partition = minor & 0x0F;
    if(partition == 0){
	/* partition 0 is the whole disk and requires no translation */
	if(blk_op.lba >= blk_op.blkdev->drive_lba_count)
	    return 1;
    } else {
	/* partitions 1+ require us to add in an offset */
	if(blk_op.lba >= blk_op.blkdev->lba_count[partition-1])
	    return 1;

	blk_op.lba += blk_op.blkdev->lba_first[partition-1];
    }
    return 0;
}

static int blkdev_transfer(uint_fast8_t minor, uint_fast8_t rawflag)
{
    uint_fast8_t n;
    uint16_t count = 0;

    /* we trust that blkdev_open() has already verified that this minor number is valid */
    blk_op.blkdev = &blkdev_table[minor >> 4];

    blk_op.is_user = rawflag;
    switch(rawflag){
        case 0:
            /* read single 512-byte sector to buffer in kernel memory */
            break;
        case 1:
            /* read some number of 512-byte sectors directly to user memory */
            if (d_blkoff(BLKSHIFT))
                return -1;
            break;
#ifdef SWAPDEV
        case 2:
            blk_op.swap_page = swappage;
            break;
#endif
        default:
            goto xferfail;
    }
    /* FIXME: these should go away now but we need to make u_block some
       kind of raw blkno_t that can be 32bit optionally */
    blk_op.nblock = udata.u_nblock;
    blk_op.lba = udata.u_block;
    blk_op.addr = udata.u_dptr;

    if (translate_lba(minor))
        goto xferfail;

    while(blk_op.nblock){
        n = blk_op.blkdev->transfer();
        if(n == 0)
            goto xferfail;
        blk_op.nblock -= n;
        count += n;
	blk_op.addr += n * BLKSIZE;
	blk_op.lba += n;
    }

    return count << BLKSHIFT; /* 10/10, would transfer sectors again */
xferfail:
    udata.u_error = EIO;
    return -1;
}

int blkdev_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    used(flag); /* not used */
    blk_op.is_read = true;
    return blkdev_transfer(minor, rawflag);
}

int blkdev_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    used(flag); /* not used */
    blk_op.is_read = false;
    return blkdev_transfer(minor, rawflag);
}

int blkdev_ioctl(uint_fast8_t minor, uarg_t request, char *data)
{
    used(data); /* unused */

    /* we trust that blkdev_open() has already verified that this minor number is valid */
    blk_op.blkdev = &blkdev_table[minor >> 4];

    switch (request)
    {
        case BLKFLSBUF:
        {
            if (blk_op.blkdev->flush)
                return blk_op.blkdev->flush();
            else
                return 0;
        }

#if defined CONFIG_TRIM
        case HDIO_TRIM:
        {
            if (blk_op.blkdev->trim)
            {
                blk_op.lba = ugetw(data);
                if (translate_lba(minor))
                    return -1;
                return blk_op.blkdev->trim();
            }
            else
                return 0;
        }
#endif

	case BLKGETSIZE:
	{
		uint_fast8_t partition = minor & 0x0F;
		uint32_t size = (partition == 0) ? blk_op.blkdev->drive_lba_count : blk_op.blkdev->lba_count[partition-1];
		/* We lack a generic uputl and this at the moment is the only case
		   it's needed so use uput() */
		return uput(&size, data, sizeof(long));
	}
	default:
	    return -1;
    }
}

blkdev_t *blkdev_alloc(void)
{
    blkdev_t *blk = &blkdev_table[0];
    while (blk <= &blkdev_table[MAX_BLKDEV-1]) {
        /* Cheapest to scan for an 8 or 16bit field and to make it start
           the struct */
        if (blk->transfer == NULL)
            return blk;
        blk++;
    }
    kputs("blkdev: full\n");
    return NULL;
}

/* Flags is not used yet but will be needed (eg for swap scans) */
void blkdev_scan(blkdev_t *blk, uint_fast8_t flags)
{
    uint_fast8_t letter = 'a' + (blk - blkdev_table);

    used(flags); /* not used */

    blk_op.blkdev = blk;
    mbr_parse(letter);
    kputchar('\n');
}
