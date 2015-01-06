#ifndef __BLKDEV_DOT_H__
#define __BLKDEV_DOT_H__

/* block device drives should call blkdev_add() for each block device found,
   and implement a sector transfer function matching the following prototype. */
typedef bool (*transfer_function_t)(uint8_t drive, uint32_t lba, void *buffer, bool read_notwrite);


/* the following details should be required only by partition parsing code */
#define MAX_PARTITIONS 15		    /* must be at least 4, at most 15 */
typedef struct {
    transfer_function_t transfer;	    /* function to read and write sectors */
    uint32_t drive_lba_count;		    /* count of sectors on raw disk device */
    uint32_t lba_first[MAX_PARTITIONS];	    /* LBA of first sector of each partition; 0 if partition absent */
    uint32_t lba_count[MAX_PARTITIONS];	    /* count of sectors in each partition; 0 if partition absent */
    uint8_t drive_number;		    /* driver's drive number */
} blkdev_t;

/* public interface */
/* public interface */
extern blkdev_t *blkdev_alloc(void);
extern void blkdev_scan(blkdev_t *blk, uint8_t flags);
#define SWAPSCAN	0x01
extern int blkdev_open(uint8_t minor, uint16_t flags);
extern int blkdev_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int blkdev_write(uint8_t minor, uint8_t rawflag, uint8_t flag);

#endif
