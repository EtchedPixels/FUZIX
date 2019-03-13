#ifndef __BLKDEV_DOT_H__
#define __BLKDEV_DOT_H__

/* block device drives should call blkdev_add() for each block device found,
   and implement a sector transfer function matching the following prototype. */
typedef uint_fast8_t (*transfer_function_t)(void);
typedef int (*flush_function_t)(void);

/* the following details should be required only by partition parsing code */
#define MAX_PARTITIONS 15                   /* must be at least 4, at most 15 */
typedef struct {
    uint8_t driver_data;                    /* opaque parameter used by underlying driver (should be first) */
    transfer_function_t transfer;           /* function to read and write sectors */
    flush_function_t flush;                 /* flush device cache */
    uint32_t drive_lba_count;               /* count of sectors on raw disk device */
    uint32_t lba_first[MAX_PARTITIONS];     /* LBA of first sector of each partition; 0 if partition absent */
    uint32_t lba_count[MAX_PARTITIONS];     /* count of sectors in each partition; 0 if partition absent */
} blkdev_t;

/* Holds the parameters for the current block operation.
 * Block I/O being single threaded is deep in the design of UZI/FUZIX
 * so let's make good use of every advantage we can from it. */
struct blkparam {
    /* do not change the order without adjusting BLKPARAM_*_OFFSET macros below */
    uint8_t *addr;                          /* address for transfer buffer */
    uint8_t is_user;	                    /* 0: kernel 1: user 2: swap */
    uint8_t swap_page;                      /* page to pass to map_swap */
    blkdev_t *blkdev;                       /* active block device */
    uint32_t lba;                           /* LBA for first sectors to transfer */
    uint16_t nblock;                        /* number of sectors to transfer */
    bool is_read;                           /* true: read sectors, false: write sectors */
};
/* macros that inline assembler code can use to access blkparam fields */
#define BLKPARAM_ADDR_OFFSET    0
#ifdef POINTER32
#define BLKPARAM_IS_USER_OFFSET 4
#else
#define BLKPARAM_IS_USER_OFFSET 2
#endif
#define BLKPARAM_SWAP_PAGE	BLKPARAM_IS_USER_OFFSET + 1

extern struct blkparam blk_op;

/* public interface */
extern blkdev_t *blkdev_alloc(void);
extern void blkdev_scan(blkdev_t *blk, uint_fast8_t flags);
#define SWAPSCAN    0x01
extern int blkdev_open(uint_fast8_t minor, uint16_t flags);
extern int blkdev_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
extern int blkdev_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
extern int blkdev_ioctl(uint_fast8_t minor, uarg_t request, char *data);

#endif
