#include "kernel.h"
#include "printf.h"

#if (BLKSIZE == 400)

/*
 *	File system routines for 400 byte blocks
 *
 *	Inodes are packed 6 per block with trailing space
 *	Directories are 40 bytes per entry (8 wasted)
 *	Files are the full 400 bytes.
 */

/* Return the number of blocks an inode occupies assuming all blocks present */
blkno_t inode_blocks(inoptr i)
{
    return (i->c_node.i_size + 399) / 400;
}

/* Read an inode */
uint_fast8_t breadi(uint16_t dev, uint16_t ino, void *ptr)
{
    struct blkbuf *buf = bread(dev, (ino / 6) + 2, 0);
    if (buf == NULL)
        return 1;
    blktok(ptr, buf, sizeof(struct dinode) * (ino % 6), sizeof(struct dinode));
    brelse(buf);
    return 0;
}

/* Write an inode */
uint_fast8_t bwritei(inoptr ino)
{
    blkno_t blkno = (ino->c_num / 6) + 2;
    struct blkbuf *buf = bread(ino->c_dev, blkno, 0);
    if (buf == NULL)
        return 1;
    blkfromk(&ino->c_node, buf, sizeof(struct dinode) * (ino->c_num % 6),
            sizeof(struct dinode));
    bfree(buf, 2);
    return 0;
}

/*
 * Bmap defines the structure of file system storage by returning
 * the physical block number on a device given the inode and the
 * logical block number in a file.  The block is zeroed if created.
 */
blkno_t bmap(inoptr ip, blkno_t bn, unsigned int rwflg)
{
    int i;
    bufptr bp;
    int j;
    blkno_t nb;
    unsigned div;
    uint16_t dev;

    if(getmode(ip) == MODE_R(F_BDEV))
        return(bn);

    dev = ip->c_dev;

    /* blocks 0..17 are direct blocks
    */
    if(bn < 18) {
        nb = ip->c_node.i_addr[bn];
        if(nb == 0) {
            if(rwflg ||(nb = blk_alloc(dev))==0)
                return(NULLBLK);
            ip->c_node.i_addr[bn] = nb;
            ip->c_flags |= CDIRTY;
        }
        return(nb);
    }

    /* addresses 18 and 19 have single and double indirect blocks.
     * the first step is to determine how many levels of indirection.
     */
    bn -= 18;
    div = 0;
    j = 2;
    if(bn >= 200) {       /* bn >= 200 so double indirect */
        div = 1;
        bn -= 200;
        j = 1;
    }

    /* fetch the address from the inode
     * Create the first indirect block if needed.
     */
    if(!(nb = ip->c_node.i_addr[20-j]))
    {
        if(rwflg || !(nb = blk_alloc(dev)))
            return(NULLBLK);
        ip->c_node.i_addr[20-j] = nb;
        ip->c_flags |= CDIRTY;
    }

    /* fetch through the indirect blocks
    */
    for(; j<=2; j++) {
        bp = bread(dev, nb, 0);
        if (bp == NULL) {
            corrupt_fs(ip->c_dev);
            return 0;
        }
        if (div)
            i = bn / 200;
        else
            i = bn % 200;

        nb = *(blkno_t *)blkptr(bp, (sizeof(blkno_t)) * i, sizeof(blkno_t));
        if (nb)
            brelse(bp);
        else
        {
            if(rwflg || !(nb = blk_alloc(dev))) {
                brelse(bp);
                return(NULLBLK);
            }
            blkfromk(&nb, bp, i * sizeof(blkno_t), sizeof(blkno_t));
            bawrite(bp);
        }
        div = 0;
    }
    return(nb);
}

#endif
