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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

typedef uint16_t blkno_t;

struct dinode {
    uint16_t i_mode;
    uint16_t i_nlink;
    uint16_t i_uid;
    uint16_t i_gid;
    off_t    i_size;
    uint32_t   i_atime;		/* Breaks in 2038 */
    uint32_t   i_mtime;		/* Need to hide some extra bits ? */
    uint32_t   i_ctime;		/* 24 bytes */
    blkno_t  i_addr[20];
};               /* Exactly 64 bytes long! */

#define FILESYS_TABSIZE 50

struct filesys {
    int16_t       s_mounted;
#define SMOUNTED  12742   /* Magic number to specify mounted filesystem */
    uint16_t      s_isize;
    uint16_t      s_fsize;
    uint16_t      s_nfree;
    blkno_t       s_free[FILESYS_TABSIZE];
    int16_t       s_ninode;
    uint16_t      s_inode[FILESYS_TABSIZE];
    uint8_t       s_fmod;
    uint8_t       s_timeh;	/* bits 32-40: FIXME - wire up */
    uint32_t      s_time;
    blkno_t       s_tfree;
    uint16_t      s_tinode;
    void *        s_mntpt;     /* Mount point */
};

#define FILENAME_LEN	30
#define DIR_LEN		32
typedef struct direct {
    uint16_t   d_ino;
    char     d_name[FILENAME_LEN];
} direct;

int dev;

#define ROOTINODE 1       /* Inode # of / for all mounted filesystems. */

direct dirbuf[64] = { {ROOTINODE, "."}, {ROOTINODE, ".."} };
struct dinode inode[8];
struct filesys fs_tab;

void dwrite(uint16_t blk, char *addr)
{
    if (lseek(dev, blk * 512L, 0) == -1) {
        perror("lseek");
        exit(1);
    }
    if (write(dev, addr, 512) != 512) {
        perror("write");
        exit(1);
    }
}

char *zerobuf(void)
{
    static char buf[512];
    
    memset(buf, 0, 512);
    return buf;
}


int yes(void)
{
    char line[20];

    if (!fgets(line, sizeof(line), stdin) || (*line != 'y' && *line != 'Y'))
	return (0);

    return (1);
}

void mkfs(uint16_t fsize, uint16_t isize)
{
    uint16_t j;
    char *zeros;

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

    inode[ROOTINODE].i_mode = S_IFDIR | 0777;
    inode[ROOTINODE].i_nlink = 3;
    inode[ROOTINODE].i_size = 64;
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

int main(int argc, char *argv[])
{
    uint16_t fsize, isize;
    struct stat statbuf;

    if (argc != 4) {
	fprintf(stderr, "usage: mkfs device isize fsize\n");
	exit(-1);
    }

    if (stat(argv[1], &statbuf) != 0) {
        fprintf(stderr, "mkfs: can't stat %s\n", argv[1]);
        exit(-1);
    }
    
    if (!S_ISBLK(statbuf.st_mode)) {
        fprintf(stderr, "mkfs: %s is not a block device\n", argv[1]);
        exit(-1);
    }

    isize = (uint16_t) atoi(argv[2]);
    fsize = (uint16_t) atoi(argv[3]);

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



