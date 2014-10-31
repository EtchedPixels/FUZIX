/**************************************************
UZI (Unix Z80 Implementation) Utilities:  mkfs.c
***************************************************/

/* This utility creates an UZI file system for a defined block device.
 * the format is:
 *                  mkfs device isize fsize
 * where: device is the logical block on which to install a filesystem
 *        isize is the number of 512-byte blocks (less two reserved for
 *            system use) to use for storing 64-byte i-nodes
 *        fsize is the total number of 512-byte blocks to assign to the
 *            filesystem.  Space available for storage of data is therefore
 *            fsize-isize blocks.
 *
 * Revisions:
 *   Based on UZI280 version (minor changes from original UZI.  HFB
 *   Modified for use under UZI.                                HP
 */

#include <stdio.h>
#include <unix.h>

long lseek(uchar, long, uchar);

int dev;

direct dirbuf[32] = { ROOTINODE, ".", ROOTINODE, ".." };
struct dinode inode[8];
struct filesys fs_tab;

main(argc, argv)
int argc;
char *argv[];
{
    uint16 fsize, isize;
    struct stat statbuf;
    int atoi(), yes(), stat(), open();

    if (argc != 4) {
	fprintf(stderr, "usage: mkfs device isize fsize\n");
	exit(-1);
    }

    if (stat(argv[1], &statbuf) != 0) {
        fprintf(stderr, "mkfs: can't stat %s\n", argv[1]);
        exit(-1);
    }
    
    if ((statbuf.st_mode & F_BDEV) != F_BDEV) {
        fprintf(stderr, "mkfs: %s is not a block device\n", argv[1]);
        exit(-1);
    }

    isize = (uint16) atoi(argv[2]);
    fsize = (uint16) atoi(argv[3]);

    if (fsize < 3 || isize < 2 || isize >= fsize) {
	fprintf(stderr, "mkfs: bad parameter values\n");
	exit(-1);
    }

    printf("Making filesystem on device %s with isize %u fsize %u. Confirm? ",
	   argv[1], isize, fsize);
    if (!yes())
	exit(-1);

    dev = open(argv[1], O_RDWR);
    if (dev < 0) {
        fprintf(stderr, "mkfs: can't open device %s\n", argv[1]);
        exit(-1);
    }

    mkfs(fsize, isize);

    exit(0);
}


mkfs(fsize, isize)
uint16 fsize, isize;
{
    uint16 j;
    char *zeros;
    char *zerobuf();

    /* Zero out the blocks */
    printf("Zeroizing i-blocks...\n");
    zeros = zerobuf();		/* Get a zero filled buffer */

#if 1
    for (j = 0; j < fsize; ++j)
	dwrite(j, zeros);
#else
    for (j = 0; j < isize; ++j)
	dwrite(j, zeros);
#endif

    /* Initialize the super-block */
    fs_tab.s_mounted = SMOUNTED;	/* Magic number */
    fs_tab.s_isize = isize;
    fs_tab.s_fsize = fsize;
    fs_tab.s_nfree = 1;
    fs_tab.s_free[0] = 0;
    fs_tab.s_tfree = 0;
    fs_tab.s_ninode = 0;
    fs_tab.s_tinode = (8 * (isize - 2)) - 2;

    /* Free each block, building the free list */

    printf("Building free list...\n");
    for (j = fsize - 1; j > isize; --j) {
	if (fs_tab.s_nfree == 50) {
	    dwrite(j, (char *) &fs_tab.s_nfree);
	    fs_tab.s_nfree = 0;
	}
	++fs_tab.s_tfree;
	fs_tab.s_free[(fs_tab.s_nfree)++] = j;
    }

    /* The inodes are already zeroed out */
    /* create the root dir */

    inode[ROOTINODE].i_mode = F_DIR | (0777 & MODE_MASK);
    inode[ROOTINODE].i_nlink = 3;
    inode[ROOTINODE].i_size.o_blkno = 0;
    inode[ROOTINODE].i_size.o_offset = 32;
    inode[ROOTINODE].i_addr[0] = isize;

    /* Reserve reserved inode */
    inode[0].i_nlink = 1;
    inode[0].i_mode = ~0;

    printf("Writing initial inode and superblock...\n");

    sync();
    dwrite(2, (char *) inode);
    dwrite(isize, (char *) dirbuf);

    sync();
    /* Write out super block */
    dwrite(1, (char *) &fs_tab);

    sync();
    printf("Done.\n");
}


dwrite(blk, addr)
uint16 blk;
char *addr;
{
    int write();
        
    lseek(dev, blk * 512L, 0);
    write(dev, addr, 512);
}

char *zerobuf()
{
    static char buf[512];
    
    blkclr(buf, 512);
    return buf;
}


int yes()
{
    char line[20];

    if (!fgets(line, sizeof(line), stdin) || (*line != 'y' && *line != 'Y'))
	return (0);

    return (1);
}
