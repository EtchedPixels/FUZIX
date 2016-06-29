
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
#include "fuzix_fs.h"

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
int swizzling = 0;

void mkfs(uint16_t fsize, uint16_t isize);
void dwrite(uint16_t blk, char *addr);

struct filesys fs_super;

int main(int argc, char **argv)
{
	uint16_t fsize, isize;

	if (argv[1] && strcmp(argv[1], "-X") == 0) {
		swizzling = 1;
		argv++;
		argc--;
	}
	if (argc != 4) {
		printf("Usage: mkfs [-X] device isize fsize\n");
		return -1;
	}

	if (sizeof(inode) != 512) {
		printf("inode is the wrong size -- %d\n",
		       (int) sizeof(inode));
	}

	isize = (uint16_t) atoi(argv[2]);
	fsize = (uint16_t) atoi(argv[3]);

	if (fsize < 3 || isize < 2 || isize >= fsize) {
		printf("Bad parameter values\n");
		return -1;
	}

	memset(zero512, 0, 512);

	printf("Making filesystem with %s byte order on device %s with fsize = %u and isize = %u.\n",
	       swizzling==0 ? "normal" : "reversed", argv[1], fsize, isize);

	if (fd_open(argv[1])) {
		printf("Can't open device");
		return -1;
	}

	mkfs(fsize, isize);

	return 0;
}

void mkfs(uint16_t fsize, uint16_t isize)
{
	uint16_t j;

	/* Zero out the blocks */

	for (j = 0; j < fsize; ++j)
		dwrite(j, zero512);

	/* Initialize the super-block */

	fs_super.s_mounted = swizzle16(SMOUNTED);	/* Magic number */
	fs_super.s_isize = swizzle16(isize);
	fs_super.s_fsize = swizzle16(fsize);
	fs_super.s_nfree = swizzle16(1);
	fs_super.s_free[0] = 0;
	fs_super.s_tfree = 0;
	fs_super.s_ninode = 0;
	fs_super.s_tinode = swizzle16(8 * (isize - 2) - 2);

	/* Free each block, building the free list */
	for (j = fsize - 1; j >= isize + 1; --j) {
		int n;
		if (swizzle16(fs_super.s_nfree) == 50) {
			dwrite(j, (char *) &fs_super.s_nfree);
			fs_super.s_nfree = 0;
		}

		fs_super.s_tfree =
		    swizzle16(swizzle16(fs_super.s_tfree) + 1);
		n = swizzle16(fs_super.s_nfree);
		fs_super.s_free[n++] = swizzle16(j);
		fs_super.s_nfree = swizzle16(n);
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
}

void dwrite(uint16_t blk, char *addr)
{
	lseek(dev_fd, ((int) blk) * 512, SEEK_SET);
	if (write(dev_fd, addr, 512) != 512) {
		perror("write");
		exit(1);
	}
}
