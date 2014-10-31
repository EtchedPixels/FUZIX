
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

direct dirbuf[64] = { {ROOTINODE,"."},
                      {ROOTINODE,".."} };
struct dinode inode[8];

void mkfs(uint16_t fsize, uint16_t isize);
void dwrite(uint16_t blk, char *addr);

struct filesys fs_super;

int main(int argc, char **argv)
{
    uint16_t fsize, isize;

    if (argc != 4)
    {
        printf("Usage: mkfs device isize fsize\n");
        return -1;
    }

    if(sizeof(inode) != 512){
        printf("inode is the wrong size -- %d\n", (int)sizeof(inode));
    }

    isize = (uint16_t)atoi(argv[2]);
    fsize = (uint16_t)atoi(argv[3]);

    if (fsize < 3 || isize < 2 || isize >= fsize)
    {
        printf("Bad parameter values\n");
        return -1;
    }

    memset(zero512, 0, 512);

    printf("Making filesystem on device %s with isize %u fsize %u.\n", argv[1], isize, fsize);

    if (fd_open(argv[1]))
    {
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

    for (j=0; j < fsize; ++j)
        dwrite(j, zero512);

    /* Initialize the super-block */

    fs_super.s_mounted = SMOUNTED; /* Magic number */
    fs_super.s_isize =  isize;
    fs_super.s_fsize =  fsize;
    fs_super.s_nfree =  1;
    fs_super.s_free[0] =  0;
    fs_super.s_tfree =  0;
    fs_super.s_ninode = 0;
    fs_super.s_tinode =  8 * (isize-2) - 2;

    /* Free each block, building the free list */
    for (j= fsize-1; j >= isize+1; --j)
    {
        if (fs_super.s_nfree == 50)
        {
            dwrite(j, (char *)&fs_super.s_nfree);
            fs_super.s_nfree = 0;
        }

        ++fs_super.s_tfree;
        fs_super.s_free[(fs_super.s_nfree)++] = j;
    }

    /* The inodes are already zeroed out */
    /* create the root dir */
    inode[ROOTINODE].i_mode = F_DIR | (0777 & MODE_MASK);
    inode[ROOTINODE].i_nlink = 3;
    inode[ROOTINODE].i_size = 64;
    inode[ROOTINODE].i_addr[0] = isize;

    /* Reserve reserved inode */
    inode[0].i_nlink = 1;
    inode[0].i_mode = ~0;

    dwrite(2, (char *)inode);

    dwrite(isize,(char *)dirbuf);

    /* Write out super block */
    dwrite(1,(char *)&fs_super);
}

void dwrite(uint16_t blk, char *addr)
{
    lseek(dev_fd, ((int)blk) * 512, SEEK_SET);
    if (write(dev_fd, addr, 512) != 512) {
        perror("write");
        exit(1);
    }
}
