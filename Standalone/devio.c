#define DEVIO

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>
#include "fuzix_fs.h"

/* Buffer pool management */
/*********************************************************************
The high-level interface is through bread() and bfree().
Bread() is given a device and block number, and a rewrite flag.  If
rewrite is 0, the block is actually read if it is not already in the
buffer pool. If rewrite is set, it is assumed that the caller plans to
rewrite the entire contents of the block, and it will not be read in,
but only have a buffer named after it.

Bfree() is given a buffer pointer and a dirty flag.  If the dirty flag
is 0, the buffer is made available for further use.  If the flag is 1,
the buffer is marked "dirty", and it will eventually be written out to
disk.  If the flag is 2, it will be immediately written out.

Zerobuf() returns a buffer of zeroes not belonging to any device.  It
must be bfree'd after use, and must not be dirty. It is used when a
read() wants to read an unallocated block of a file.

Bufsync() write outs all dirty blocks.

Note that a pointer to a buffer structure is the same as a pointer to
the data.  This is very important.
**********************************************************************/

uint16_t bufclock = 0;         /* Time-stamp counter for LRU */
struct blkbuf bufpool[NBUFS];

char *bread(int dev, blkno_t blk, int rewrite)
{
    register bufptr bp;

/*printf("Reading block %d\n", blk);*/

    bp = bfind (dev, blk);
    if (bp)
    {
        if (bp->bf_busy)
            panic ("want busy block");
        goto done;
    }
    bp = freebuf();
    bp->bf_dev = dev;
    bp->bf_blk = blk;

    /* If rewrite is set, we are about to write over the entire block,
       so we don't need the previous contents */

    ifnot (rewrite)
        if (bdread (bp) == -1)
        {
            udata.u_error = EIO;
            return 0;
        }

/*--    if (rewrite == 2)--*/
/*--        bzero (bp->bf_data, 512);--*/

done:
    bp->bf_busy = 1;
    bp->bf_time = ++bufclock;  /* Time stamp it */
    return (bp->bf_data);
}


void brelse(bufptr bp)
{
/*printf("Releasing block %d (0)\n", bp->bf_blk);*/
    bfree (bp, 0);
}

void bawrite(bufptr bp)
{
/*printf("Releasing block %d (1)\n", bp->bf_blk);*/
    bfree (bp, 1);
}

int bfree(bufptr bp, int dirty)
{
/*printf("Releasing block %d (%d)\n", bp->bf_blk, dirty);*/
    bp->bf_dirty |= dirty;
    bp->bf_busy = 0;

    if (dirty == 2)   /* Extra dirty */
    {
        if (bdwrite (bp) == -1)
            udata.u_error = EIO;
        bp->bf_dirty = 0;
        return (-1);
    }
    return (0);
}


/* This returns a busy block not belonging to any device, with
 * garbage contents.  It is essentially a malloc for the kernel.
 * Free it with brelse()!
 */
char *
tmpbuf ()
{
    bufptr bp;
    bufptr freebuf();

/*printf("Allocating temp block\n");*/
    bp = freebuf();
    bp->bf_dev = -1;
    bp->bf_busy = 1;
    bp->bf_time = ++bufclock;   /* Time stamp it */
    return (bp->bf_data);
}


char *zerobuf (void)
{
    char *b;
    char *tmpbuf();

    b = tmpbuf();
    bzero (b, 512);
    return (b);
}


void bufsync (void)
{
    register bufptr bp;

    for (bp=bufpool; bp < bufpool+NBUFS; ++bp)
    {
        if (bp->bf_dev != -1 && bp->bf_dirty)
        {
            bdwrite (bp);
            if (!bp->bf_busy)
                bp->bf_dirty = 0;
        }
    }
}

#ifndef ASM_BUFIO

bufptr bfind (int dev, blkno_t blk)
{
    register bufptr bp;

    for (bp=bufpool; bp < bufpool+NBUFS; ++bp)
    {
        if (bp->bf_dev == dev && bp->bf_blk == blk)
            return (bp);
    }
    return (NULL);
}


bufptr freebuf(void)
{
    register bufptr bp;
    register bufptr oldest;
    register int oldtime;

    /* Try to find a non-busy buffer and write out the data if it is dirty */
    oldest = NULL;
    oldtime = 0;
    for (bp=bufpool; bp < bufpool+NBUFS; ++bp)
    {
        if (bufclock - bp->bf_time >= oldtime && !bp->bf_busy)
        {
            oldest = bp;
            oldtime = bufclock - bp->bf_time;
        }
    }
    ifnot (oldest)
        panic ("no free buffers");

    if (oldest->bf_dirty)
    {
        if (bdwrite (oldest) == -1)
            udata.u_error = EIO;
        oldest->bf_dirty = 0;
    }
    return (oldest);
}

#endif
        

void bufinit (void)
{
    register bufptr bp;

    for (bp=bufpool; bp < bufpool+NBUFS; ++bp)
    {
        bp->bf_dev = -1;
    }
}


void bufdump (void)
{
    register bufptr j;

    printf ("\ndev\tblock\tdirty\tbusy\ttime clock %d\n", bufclock);
    for (j=bufpool; j < bufpool+NBUFS; ++j)
        printf ("%d\t%u\t%d\t%d\t%u\n",
            j->bf_dev,j->bf_blk,j->bf_dirty,j->bf_busy,j->bf_time);
}


/*********************************************************************
Bdread() and bdwrite() are the block device interface routines.  they
are given a buffer pointer, which contains the device, block number,
and data location.  They basically validate the device and vector the
call.

Cdread() and cdwrite are the same for character (or "raw") devices,
and are handed a device number.  Udata.u_base, count, and offset have
the rest of the data.
**********************************************************************/

int bdread (bufptr bp)
{
//    printf("bdread(fd=%d, block %d)\n", dev_fd, bp->bf_blk);

    udata.u_buf = bp;
    if (lseek(dev_fd, dev_offset + (((int)bp->bf_blk) * 512), SEEK_SET) == -1)
        perror("lseek");
    if(read(dev_fd, bp->bf_data, 512) != 512)
        panic("read() failed");

    return 0;
}


int bdwrite (bufptr bp)
{
    udata.u_buf = bp;

    lseek(dev_fd, dev_offset + (((int)bp->bf_blk) * 512), SEEK_SET);
    if(write(dev_fd, bp->bf_data, 512) != 512)
        panic("write() failed");
    return 0;
}


