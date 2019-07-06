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
#include <sys/types.h>
#include <sys/super.h>
#include <time.h>


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

#define FILENAME_LEN	30
#define DIR_LEN		32
typedef struct direct {
    uint16_t   d_ino;
    char     d_name[FILENAME_LEN];
} direct;


uint8_t fast=0;     /* flag for fast formatting option */

int dev;

#define ROOTINODE 1       /* Inode # of / for all mounted filesystems. */

direct dirbuf[64] = { {ROOTINODE, "."}, {ROOTINODE, ".."} };
struct dinode inode[8];
struct fuzix_filesys_kernel fs_tab;

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

    fflush(stdout);

    if (!fgets(line, sizeof(line), stdin) || (*line != 'y' && *line != 'Y'))
	return (0);

    return (1);
}

void mkfs(uint16_t fsize, uint16_t isize)
{
    uint16_t j;
    char *zeros;
    time_t t = time(NULL);

    /* Zero out the blocks */
    printf("Clearing blocks ");
    zeros = zerobuf();		/* Get a zero filled buffer */

    if (!fast) {
	    for (j = 0; j < fsize; ++j) {
	            putchar('.');
		    dwrite(j, zeros);
            }
    } else {
	    for (j = 0; j < isize; ++j) {
	            putchar('.');
		    dwrite(j, zeros);
            }
    }

    /* Initialize the super-block */
    fs_tab.s_mounted = SMOUNTED;	/* Magic number */
    fs_tab.s_isize = isize;
    fs_tab.s_fsize = fsize;
    fs_tab.s_nfree = 1;
    fs_tab.s_free[0] = 0;
    fs_tab.s_tfree = 0;
    fs_tab.s_ninode = 0;
    fs_tab.s_tinode = (8 * (isize - 2)) - 2;
    fs_tab.s_time = t;
    fs_tab.s_timeh = (t >> 31) >> 1;	/* Mutter .. C standards .. mutter */

    /* Free each block, building the free list */

    printf("\nBuilding free list...\n");
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

void printopts(void)
{
	fprintf( stderr, "usage: mkfs [options] device isize fsize\n");
	exit(-1);
}


int main(int argc, char *argv[])
{
    uint16_t fsize, isize;
    struct stat statbuf;
    int option;

    while( (option=getopt( argc, argv, "f" ))>0 ){
	    switch( option ){
	    case 'f':
		    fast=1;
		    break;
	    case '?':
	    default:
		    printopts();
	    }
    }

    if (argc-optind < 3) printopts();
    
    
    if (stat(argv[optind], &statbuf) != 0) {
        fprintf(stderr, "mkfs: can't stat %s\n", argv[optind]);
        exit(-1);
    }

    if (!S_ISBLK(statbuf.st_mode)) {
        fprintf(stderr, "mkfs: %s is not a block device\n", argv[optind]);
        exit(-1);
    }

    isize = (uint16_t) atoi(argv[optind+1]);
    fsize = (uint16_t) atoi(argv[optind+2]);

    if (fsize < 3 || isize < 2 || isize >= fsize) {
	fprintf(stderr, "mkfs: bad parameter values\n");
	exit(-1);
    }

    printf("Making filesystem on device %s with isize %u fsize %u. Confirm? ",
	   argv[optind], isize, fsize);
    if (!yes())
	exit(-1);

    dev = open(argv[optind], O_RDWR|O_SYNC);
    if (dev < 0) {
        fprintf(stderr, "mkfs: can't open device %s\n", argv[optind]);
        exit(-1);
    }

    mkfs(fsize, isize);

    exit(0);
}



