#ifndef __BLKDEV_DOT_H__
#define __BLKDEV_DOT_H__

/* block device drives should call blkdev_add() for each block device found,
   and implement a sector transfer function matching the following prototype. */
typedef bool (*transfer_function_t)(uint8_t drive, uint32_t lba, void *buffer, bool read_notwrite);

/* public interface */
void blkdev_add(transfer_function_t transfer, uint8_t drive_number, uint32_t lba_count);
int blkdev_open(uint8_t minor, uint16_t flags);
int blkdev_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
int blkdev_write(uint8_t minor, uint8_t rawflag, uint8_t flag);

/* the following details should be required only by partition parsing code */
#define MAX_PARTITIONS 15		    /* must be at least 4, at most 15 */
typedef struct {
    uint32_t drive_lba_count;		    /* count of sectors on raw disk device */
    uint8_t drive_number;		    /* driver's drive number */
    uint32_t lba_first[MAX_PARTITIONS];	    /* LBA of first sector of each partition; 0 if partition absent */
    uint32_t lba_count[MAX_PARTITIONS];	    /* count of sectors in each partition; 0 if partition absent */
    transfer_function_t transfer;	    /* function to read and write sectors */
} blkdev_t;

#endif
