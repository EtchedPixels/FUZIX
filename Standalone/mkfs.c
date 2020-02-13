
/**************************************************
UZI (Unix Z80 Implementation) Utilities:  mkfs.c
***************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include "fuzix_fs.h"
#include "util.h"

/* This makes a filesystem 
 *
 * example use:
 *   ./mkfs ./blankfs.img 64 4096
 * (this will write a 2MB filesystem with 64 blocks of inodes to ./blankfs.img)
 *
 * */

char zero512[512];

direct dirbuf[64] = {
        { ROOTINODE, "." },
        { ROOTINODE, ".."}
};

struct dinode inode[8];

void mkfs(uint16_t fsize, uint16_t isize, uint8_t shift);
void dwrite(uint16_t blk, char *addr);

union disk {
	struct filesys fs;
	uint8_t zero[512];
} fs_super;

static void usage(void)
{
	printf("Usage: mkfs [-X] [-b blocksize] device isize fsize\n");
	exit(1);
}

static uint8_t validate(uint16_t bsize)
{
	switch(bsize) {
	case 512:
		return 0;
	case 1024:
		return 1;
	case 2048:
		return 2;
	case 4096:
		return 3;
	case 8192:
		return 4;
	case 16384:
		return 5;
	default:
		fprintf(stderr, "mkfs: unsupported block size.\n");
		exit(1);
	}
}

int main(int argc, char **argv)
{
	uint16_t fsize, isize, bsize = 512, shift = 0;
	uint16_t j;
	uint32_t s;
	int opt;
	time_t t = time(NULL);

	while((opt = getopt(argc, argv, "Xb:")) != -1) {
		switch(opt) {
			case 'X':	
				swizzling = 1;
				break;
			case 'b':
				bsize = atoi(optarg);
				shift = validate(bsize);
				break;
			default:
				usage();
		}
	}
	if (argc - optind != 3)
		usage();

	if (sizeof(inode) != 512) {
		printf("inode is the wrong size -- %d\n",
		       (int) sizeof(inode));
	}

	isize = (uint16_t) atoi(argv[optind + 1]);
	fsize = (uint16_t) atoi(argv[optind + 2]);

	if (fsize < 3 || isize < 2 || isize >= fsize) {
		printf("Bad parameter values\n");
		return -1;
	}

	memset(zero512, 0, 512);

	printf("Making %d byte/block filesystem with %s byte order on device %s with fsize = %u and isize = %u.\n",
	       bsize, swizzling==0 ? "normal" : "reversed", argv[optind], fsize, isize);

	if (fd_open(argv[optind], O_CREAT)) {
		printf("Can't open device");
		return -1;
	}

	s = fsize << shift;

	/* Zero out the blocks */
	for (j = 0; j < s; ++j)
		dwrite(j, zero512);

	/* Initialize the super-block */

	fs_super.fs.s_mounted = swizzle16(SMOUNTED);	/* Magic number */
	fs_super.fs.s_isize = swizzle16(isize);
	fs_super.fs.s_fsize = swizzle16(fsize);
	fs_super.fs.s_nfree = swizzle16(1);
	fs_super.fs.s_free[0] = 0;
	fs_super.fs.s_tfree = 0;
	fs_super.fs.s_ninode = 0;
	fs_super.fs.s_tinode = swizzle16(8 * (isize - 2) - 2);
	fs_super.fs.s_shift = shift;
	fs_super.fs.s_time = swizzle32(t);
	fs_super.fs.s_timeh = t >> 32;

	/* Free each block, building the free list. This is done in
	   terms of the block shift, while isize is in 512 byte blocks.
	   Adjust isize so that it's in block terms and references the
	   block after the last inode */
	  
	isize <<= shift;

	/* Don't free the block isize because it's got the / directory in it */
	for (j = fsize - 1; j > isize; --j) {
		int n;
		if (swizzle16(fs_super.fs.s_nfree) == 50) {
			dwrite(j, (char *) &fs_super.fs.s_nfree);
			fs_super.fs.s_nfree = 0;
		}

		fs_super.fs.s_tfree =
		    swizzle16(swizzle16(fs_super.fs.s_tfree) + 1);
		n = swizzle16(fs_super.fs.s_nfree);
		fs_super.fs.s_free[n++] = swizzle16(j);
		fs_super.fs.s_nfree = swizzle16(n);
	}

	/* The inodes are already zeroed out */
	/* create the root dir */
	inode[ROOTINODE].i_mode = swizzle16(F_DIR | (0777 & MODE_MASK));
	inode[ROOTINODE].i_nlink = swizzle16(3);
	inode[ROOTINODE].i_size = swizzle32(64);
	inode[ROOTINODE].i_addr[0] = swizzle16(isize);

	/* Reserve reserved inode */
	inode[0].i_nlink = swizzle16(1);
	inode[0].i_mode = ~0;

	dwrite(2, (char *) inode);

	dirbuf[0].d_ino = swizzle16(dirbuf[0].d_ino);
	dirbuf[1].d_ino = swizzle16(dirbuf[1].d_ino);
	dwrite(isize, (char *) dirbuf);

	/* Write out super block */
	dwrite(1, (char *) &fs_super);
	return 0;
}

void dwrite(uint16_t blk, char *addr)
{
	lseek(dev_fd, ((int) blk) * 512, SEEK_SET);
	if (write(dev_fd, addr, 512) != 512) {
		perror("write");
		exit(1);
	}
}
