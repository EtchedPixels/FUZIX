#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef uint16_t	blkno_t;

struct filesys {
    uint16_t      s_mounted;
    uint16_t      s_isize;
    uint16_t      s_fsize;
    int16_t       s_nfree;
    blkno_t     s_free[50];
    int16_t       s_ninode;
    uint16_t      s_inode[50];
    uint8_t       s_fmod;
#define FMOD_DIRTY	1
#define FMOD_CLEAN	2
    uint8_t	s_timeh;	/* top bits of time */
    uint32_t      s_time;
    blkno_t     s_tfree;
    uint16_t      s_tinode;
    uint16_t      s_mntpt;
};

#define ROOTINODE 1
#define SMOUNTED 12742   /* Magic number to specify mounted filesystem */
#define SMOUNTED_WRONGENDIAN 50737U   /* byteflipped */

struct dinode {
    uint16_t i_mode;
    uint16_t i_nlink;
    uint16_t i_uid;
    uint16_t i_gid;
    uint32_t    i_size;
    uint32_t   i_atime;
    uint32_t   i_mtime;
    uint32_t   i_ctime;
    blkno_t  i_addr[20];
};               /* Exactly 64 bytes long! */

#define F_REG   0100000
#define F_DIR   040000
#define F_PIPE  010000
#define F_BDEV  060000
#define F_CDEV  020000

#define F_MASK  0170000

struct direct {
        uint16_t   d_ino;
        char     d_name[30];
};

#define BLK_OVERSIZE32	0xFE

#define MAXDEPTH 20	/* Maximum depth of directory tree to search */

/* This checks a filesystem */

static const char no_more_free[] = { "Sorry... No more free blocks." };
static const char block_multi[]= { "Block %u in inode %u value %u multiply allocated. Fix? " };
static const char missing_ind_blk[] = { "Missing indirect block" };
static const char no_memory[] = { "Not enough memory.\n" };
static const char mbidir[] = { "Missing block in directory" };

static int dev = 0;
static struct filesys superblock;
static int dev_fd;
static int error;
static int rootfs;
static int aflag;

static unsigned char *bitmap;
static uint16_t bitmap_size;
static int16_t *linkmap;
static char *daread(uint16_t blk);
static void dwrite(uint16_t blk, char *addr);
static void iread(uint16_t ino, struct dinode *buf);
static void iwrite(uint16_t ino, struct dinode *buf);
static void setblkno(struct dinode *ino, blkno_t num, blkno_t dnum);
static void ckdir(uint16_t inum, uint16_t pnum, char *name);
static void dirread(struct dinode *ino, uint16_t j, struct direct *dentry);
static void dirwrite(struct dinode *ino, uint16_t j, struct direct *dentry);
static void mkentry(uint16_t inum);
static blkno_t blk_alloc0(struct filesys *filesys);
static blkno_t getblkno(struct dinode *ino, blkno_t num);

static void pass1(void);
static void pass2(void);
static void pass3(void);
static void pass4(void);
static void pass5(void);

/* Useful repeated values */
static uint16_t max_inode;

static int yes_noerror(void)
{
    static char buf[16];
    fflush(stdout);
    do {
        if (fgets(buf, 15, stdin) == NULL)
            exit(1);
        if (isupper(*buf))
            *buf = tolower(*buf);
    } while(*buf != 'n' && *buf != 'y');
    return  (*buf == 'y') ? 1 : 0;
}

static int yes(void) {
    int ret = yes_noerror();
    if (ret)
        error |= 1;
    else
        error |= 4;
    return ret;
}

static void progress(void)
{
    static uint8_t progct;
    static uint8_t scalect;
    if (++scalect == 16) {
        scalect = 0;
        printf("%c\010", "-\\|/"[progct++&3]);
        fflush(stdout);
    }
}

static uint8_t bitmask[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

static void bitset(uint16_t b)
{
    uint16_t n = b >> 3;
    if (n >= bitmap_size)
        return;
    bitmap[n] |= bitmask[b & 7];
}

static const uint8_t bmask[] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };

static int bittest(uint16_t b)
{
    uint16_t n = b >> 3;
    if (n >= bitmap_size)
        return 0;
    /* GCC 68HC11 miscompiles the bitmask code we use for speed elsewhere. For
       now bodge around it until we can kick gcc out for the native compiler */
#ifdef mc68hc11        
    b &= 7;
    return !!(bitmap[n] & (1 << b));
#else    
    return (bitmap[n] & bitmask[b & 7]) ? 1 : 0;
#endif    
}

static void panic(const char *s)
{
	fprintf(stderr, "panic: %s\n", s);
	exit(error | 8);
}

static int fd_open(char *name)
{
	struct stat rootst;
	struct stat work;

	dev_fd = open(name, O_RDWR | O_CREAT, 0666);

	if (dev_fd < 0)
		return -1;

	if (stat("/", &rootst) == -1)
	    panic("stat /");
        if (fstat(dev_fd, &work) == -1)
            panic("statfd");

        if (rootst.st_dev == work.st_rdev) {
            puts("Checking root file system.");
            rootfs = 1;
        }

	return 0;
}

int perform_fsck(char *name)
{
    char *buf;

    if (fd_open(name)){
        puts("Cannot open file");
        return 16;
    }

    buf = daread(1);
    memcpy((char *) &superblock, buf, sizeof(struct filesys));

    if (superblock.s_fmod == FMOD_DIRTY) {
        puts("Filesystem was not cleanly unmounted.");
        error |= 1;
    }
    else if (aflag)
        return 0;

    /* Verify the fsize and isize parameters */
    if (superblock.s_mounted == SMOUNTED_WRONGENDIAN) {
        panic("Reversed byte order.\n");
    }

    if (superblock.s_mounted != SMOUNTED) {
        printf("Device %u has invalid magic number %u. Fix? ", dev, superblock.s_mounted);
        if (!yes())
            exit(error|32);
        superblock.s_mounted = SMOUNTED;
        dwrite((blkno_t) 1, (char *) &superblock);
    }
    printf("Device %u has fsize = %u and isize = %u. Continue? ",
            dev, superblock.s_fsize, superblock.s_isize);
    if (!yes_noerror())
        return (error |= 32);

    linkmap = (int16_t *) calloc(8 * superblock.s_isize, sizeof(int16_t));
    /* Not worth worrying about the one spare byte in some cases */
    bitmap_size = (superblock.s_fsize >> 3) + 1;
    bitmap = calloc(bitmap_size, sizeof(char));

    if (!bitmap || !linkmap) {
        fputs(no_memory, stderr);
        return(error |= 8);
    }

    max_inode = 8 * (superblock.s_isize - 2);

    puts("Pass 1: Checking inodes...");
    pass1();

    puts("Pass 2: Rebuilding free list...");
    pass2();

    puts("Pass 3: Checking block allocation...");

    pass3();

    /* We don't need the bitmap but we do need space for path buffers */
    free(bitmap);

    puts("Pass 4: Checking directory entries...");
    pass4();

    puts("Pass 5: Checking link counts...");
    pass5();

    /* If we fixed things, and no errors were left uncorrected */
    if ((error & 69) == 1) {	/* 64 4 1 moust be 1 */
        superblock.s_fmod = FMOD_CLEAN;
        dwrite((blkno_t) 1, (char *) &superblock);
        if (rootfs) {
            error |= 2;
            puts("**** Root filesystem was modified, immediate reboot required.");
            sleep(5);
            /* Sync it all out and reboot */
            uadmin(A_REBOOT, 0, 0);
        }
    }
    free(linkmap);
    close(dev_fd);
    return error;
}

int main(int argc, char *argv[])
{
    if (argc == 3 && strcmp(argv[1], "-a") == 0) {
        aflag = 1;
        argv++;
    } else if(argc != 2) {
        fputs("syntax: fsck-fuzix [-a] [devfile]\n", stderr);
        return 16;
    }
    /* Re-run each time the error code is 'do another run over the disk' */
    do {
        error = 0;
        perform_fsck(argv[1]);
        if (error & 64)
            puts("Rescanning disk");
    }
    while(error & 64);
    exit(error);
}

/*
 *  Pass 1 checks each inode independently for validity, zaps bad block
 *  numbers in the inodes, and builds the block allocation map.
 */

static void pass1(void)
{
    uint16_t n;
    struct dinode ino;
    uint16_t mode;
    blkno_t b;
    blkno_t bno;
    uint16_t icount;
    blkno_t *buf;
    uint32_t bmax;

    icount = 0;

    /* Consider rescanning this basic loop in pass4 so that we don't have
       to keep both bitmap and linkmap around at the same time */
    for (n = ROOTINODE; n < max_inode; ++n) {
        iread(n, &ino);
        progress();
        linkmap[n] = -1;
        if (ino.i_mode == 0)
            continue;

        mode = ino.i_mode & F_MASK;
        /* FIXME: named pipe/socket need autoclear .. */

        /* Check mode */
        if (mode != F_REG && mode != F_DIR && mode != F_BDEV && mode != F_CDEV) {
            printf("Inode %u with mode 0%o is not of correct type. Zap? ",
                    n, ino.i_mode);
            if (yes()) {
                ino.i_mode = 0;
                ino.i_nlink = 0;
                iwrite(n, &ino);
                continue;
            }
        }
        linkmap[n] = 0;
        ++icount;
        /* Check size */

        if ((ino.i_size >> 24) & BLK_OVERSIZE32) {
            printf("Inode %u offset is too large with value of %ld. Fix? ",
                    n, ino.i_size);
            if (yes()) {
                ino.i_size = 0;
                iwrite(n, &ino);
            }
        }
        /* Check blocks and build free block map */
        if (mode == F_REG || mode == F_DIR) {
            /* Check singly indirect blocks */

            for (b = 18; b < 20; ++b) {
                if (ino.i_addr[b] != 0 && 
                    (ino.i_addr[b] < superblock.s_isize ||
                            ino.i_addr[b] >= superblock.s_fsize)) {
                    printf("Inode %u singly ind. blk %u out of range, val = %u. Zap? ",
                            n, b, ino.i_addr[b]);
                    if (yes()) {
                        ino.i_addr[b] = 0;
                        iwrite(n, &ino);
                    }
                }
                if (ino.i_addr[b] != 0 && ino.i_size < 18*512) {
                    printf("Inode %u singly ind. blk %u past end of file, val = %u. Zap? ",
                            n, b, ino.i_addr[b]);
                    if (yes()) {
                        ino.i_addr[b] = 0;
                        iwrite(n, &ino);
                    }
                }
                if (ino.i_addr[b] != 0)
                    bitset(ino.i_addr[b]);
            }

            /* Check the double indirect blocks */
            if (ino.i_addr[19] != 0) {
                buf = (blkno_t *) daread(ino.i_addr[19]);
                for (b = 0; b < 256; ++b) {
                    if (buf[b] != 0 && (buf[b] < superblock.s_isize ||
                                buf[b] >= superblock.s_fsize)) {
                        printf("Inode %u doubly ind. blk %u is out of range, val = %u. Zap? ",
                                n, b, buf[b]);
                        /* 1.4.98 - line split.  HFB */
                        if (yes()) {
                            buf[b] = 0;
                            dwrite(b, (char *) buf);
                        }
                    }
                    if (buf[b] != 0)
                        bitset(buf[b]);
                    progress();
                }
            }
            /* Check the rest */
            bmax = ino.i_size/512;
            for (bno = 0; bno <= bmax ; ++bno) {
                b = getblkno(&ino, bno);
                progress();

                if (b != 0 && (b < superblock.s_isize || b >= superblock.s_fsize)) {
                    printf("Inode %u block %u out of range, val = %u. Zap? ",
                            n, bno, b);
                    if (yes()) {
                        setblkno(&ino, bno, 0);
                        iwrite(n, &ino);
                    }
                }
                if (b != 0)
                    bitset(b);
            }
        }
    }
    /* Fix free inode count in superblock block */
    icount = max_inode - ROOTINODE - icount;
    if (superblock.s_tinode != icount) {
        printf("Free inode count in superblock is %u, should be %u. Fix? ",
                superblock.s_tinode, icount);

        if (yes()) {
            superblock.s_tinode = icount;
            dwrite((blkno_t) 1, (char *) &superblock);
        }
    }
}

/* Clear inode free list, rebuild block free list using bit map. */
static void pass2(void)
{
    blkno_t j;
    blkno_t oldtfree;
    uint16_t imax = superblock.s_isize;

    printf("Rebuild free list? ");
    if (!yes_noerror())
        return;

    error |= 1;
    oldtfree = superblock.s_tfree;

    /* Initialize the superblock-block */

    superblock.s_ninode = 0;
    superblock.s_nfree = 1;
    superblock.s_free[0] = 0;
    superblock.s_tfree = 0;

    /* Free each block, building the free list */

    for (j = superblock.s_fsize - 1; j >= imax; --j) {
        if (bittest(j) == 0) {
            if (superblock.s_nfree == 50) {
                dwrite(j, (char *) &superblock.s_nfree);
                superblock.s_nfree = 0;
                progress();
            }
            superblock.s_tfree++;
            superblock.s_free[superblock.s_nfree++] = j;
        }
    }

    dwrite((blkno_t) 1, (char *) &superblock);

    if (oldtfree != superblock.s_tfree)
        printf("s_tfree was changed to %u from %u.\n",
                superblock.s_tfree, oldtfree);

}

/* Pass 3 finds and fixes multiply allocated blocks. */
static void pass3(void)
{
    uint16_t n;
    struct dinode ino;
    uint16_t mode;
    blkno_t b;
    blkno_t bno;
    blkno_t newno;
    uint16_t bmax = superblock.s_fsize;
    uint32_t nmax;

    memset(bitmap, 0x00, bitmap_size);
    for (n = 0; n < superblock.s_isize; n++)
        bitset(n);

    for (n = ROOTINODE; n < max_inode; ++n) {
        iread(n, &ino);
        progress();

        mode = ino.i_mode & F_MASK;
        if (mode != F_REG && mode != F_DIR)
            continue;

        /* Check singly indirect blocks */

        for (b = 18; b < 20; ++b) {
            if (ino.i_addr[b] != 0) {
                if (bittest(ino.i_addr[b]) != 0) {
                    puts("Indirect b");
                    printf(block_multi + 1, b, n, ino.i_addr[b]);
                    if (yes()) {
                        newno = blk_alloc0(&superblock);
                        if (newno == 0) {
                            puts(no_more_free);
                            error |= 4;
                        } else {
                            dwrite(newno, daread(ino.i_addr[b]));
                            ino.i_addr[b] = newno;
                            iwrite(n, &ino);
                            error |= 64;
                        }
                    }
                } else
                    bitset(ino.i_addr[b]);
            }
        }

        /* Check the rest */
        nmax = ino.i_size/512;
        for (bno = 0; bno <= nmax; ++bno) {
            b = getblkno(&ino, bno);

            if (b != 0) {
                if (bittest(b)) {
                    printf(block_multi,
                            bno, n, b);
                    if (yes()) {
                        newno = blk_alloc0(&superblock);
                        if (newno == 0) {
                            puts(no_more_free);
                            error |= 4;
                        } else {
                            dwrite(newno, daread(b));
                            setblkno(&ino, bno, newno);
                            iwrite(n, &ino);
                            error |= 64;
                        }
                    }
                } else
                    bitset(b);
            }
        }

    }

}

static int depth;

/*
 *  Pass 4 traverses the directory tree, fixing bad directory entries
 *  and finding the actual number of references to each inode.
 */

static void pass4(void)
{
    depth = 0;
    linkmap[ROOTINODE] = 1;
    ckdir(ROOTINODE, ROOTINODE, "/");
//if (depth != 0)
//      panic("Inconsistent depth");
}


/* This recursively checks the directories */

static void ckdir(uint16_t inum, uint16_t pnum, char *name)
{
    struct dinode ino;
    /* We can keep this off the stack, we'd really like to keep the inode
       off it too but that would thrash the disk horribly */
    static struct direct dentry;
    uint16_t j;
    int c;
    uint8_t i;
    int nentries;
    char *ename;

    iread(inum, &ino);
    if ((ino.i_mode & F_MASK) != F_DIR)
        return;
    ++depth;

    if (((uint8_t)ino.i_size) & 31) {
        printf("Directory inode %u has improper length. Fix? ", inum);
        if (yes()) {
            ino.i_size &= ~0x1fUL;
            iwrite(inum, &ino);
            error |= 64;
        }
    }
    nentries = ino.i_size/32;

    /* Each iteration we begin with a new ino/dentry. Each recursion we
       destroy them. Be careful */
    for (j = 0; j < nentries; ++j) {
        dirread(&ino, j, &dentry);
        progress();

        /* TODO: optimization - don't write back if we didn't changee it */
        for (i = 0; i < 30; ++i) if (dentry.d_name[i] == '\0') break;
        for (     ; i < 30; ++i) dentry.d_name[i] = '\0';
        dirwrite(&ino, j, &dentry);

        if (dentry.d_ino == 0)
            continue;

        if (dentry.d_ino < ROOTINODE ||
                dentry.d_ino >= max_inode) {
            printf("Directory entry %s%-1.30s has out-of-range inode %u. Zap? ",
                    name, dentry.d_name, dentry.d_ino);
            if (yes()) {
                dentry.d_ino = 0;
                dentry.d_name[0] = '\0';
                dirwrite(&ino, j, &dentry);
                error |= 64;
                continue;
            }
        }
        if (dentry.d_ino && linkmap[dentry.d_ino] == -1) {
            printf("Directory entry %s%-1.30s points to bogus inode %u. Zap? ",
                    name, dentry.d_name, dentry.d_ino);
            if (yes()) {
                dentry.d_ino = 0;
                dentry.d_name[0] = '\0';
                dirwrite(&ino, j, &dentry);
                continue;
            }
        }
        ++linkmap[dentry.d_ino];

        for (c = 0; c < 30 && dentry.d_name[c]; ++c) {
            if (dentry.d_name[c] == '/') {
                printf("Directory entry %s%-1.30s contains slash. Fix? ",
                        name, dentry.d_name);
                if (yes()) {
                    dentry.d_name[c] = 'X';
                    dirwrite(&ino, j, &dentry);
                }
            }
        }

        if (strncmp(dentry.d_name, ".", 30) == 0 && dentry.d_ino != inum) {
            printf("Dot entry %s%-1.30s points to wrong place. Fix? ",
                    name, dentry.d_name);
            if (yes()) {
                dentry.d_ino = inum;
                dirwrite(&ino, j, &dentry);
                error |= 64;
            }
        }
        if (strncmp(dentry.d_name, "..", 30) == 0 && dentry.d_ino != pnum) {
            printf("DotDot entry %s%-1.30s points to wrong place. Fix? ",
                    name, dentry.d_name);
            if (yes()) {
                dentry.d_ino = pnum;
                dirwrite(&ino, j, &dentry);
                error |= 64;
            }
        }
        if (dentry.d_ino != pnum &&
                dentry.d_ino != inum && depth < MAXDEPTH) {
            /* This is a bad approach. We ought to have some kind of
               growing buffer, but our realloc will end up chopping up
               the pool and stuff I think. sbrk() doesn't help as we've
               got lots of memory in the pool from freeing bitmap that
               won't go back FIXME */
            ename = malloc(strlen(name) + strlen(dentry.d_name) + 2);
            if (ename == NULL) {
                fprintf(stderr, "Not enough memory.\n");
                exit(error |= 8);
            }
            strcpy(ename, name);
            strcat(ename, dentry.d_name);
            strcat(ename, "/");
            ckdir(dentry.d_ino, inum, ename);
            free(ename);
        }
        /* dentry and dino are not valid any more */
    }
    --depth;
}

static void linkmaperr(void)
{
    panic("linkmap");
}

/* Pass 5 compares the link counts found in pass 4 with the inodes. */
static void pass5(void)
{
    uint16_t n;
    struct dinode ino;

    for (n = ROOTINODE; n < max_inode; ++n) {
        iread(n, &ino);
        progress();

        if (ino.i_mode == 0) {
            if (linkmap[n] != -1)
                linkmaperr();
            continue;
        }

        if (linkmap[n] == -1 && ino.i_mode != 0)
            linkmaperr();

        if (linkmap[n] > 0 && ino.i_nlink != linkmap[n]) {
            printf("Inode %u has link count %u should be %u. Fix? ",
                    n, ino.i_nlink, linkmap[n]);
            if (yes()) {
                ino.i_nlink = linkmap[n];
                iwrite(n, &ino);
            }
        }

        if (linkmap[n] == 0) {
            if ((ino.i_mode & F_MASK) == F_BDEV ||
                    (ino.i_mode & F_MASK) == F_CDEV ||
                    (ino.i_size == 0)) {
                printf("Useless inode %u with mode 0%o has become detached. Link count is %u. Zap? ",
                        n, ino.i_mode, ino.i_nlink);
                if (yes()) {
                    ino.i_nlink = 0;
                    ino.i_mode = 0;
                    iwrite(n, &ino);
                    superblock.s_tinode++;
                    dwrite((blkno_t) 1, (char *) &superblock);
                    error |= 64;
                }
            } else {
                printf("Inode %u has become detached. Link count is %u. ",
                        n, ino.i_nlink);
                puts(ino.i_nlink ? "Zap? " : "Fix? ");
                if (yes()) {
                    error |= 64;
                    if (ino.i_nlink == 0) {
                        ino.i_nlink = 0;
                        ino.i_mode = 0;
                        iwrite(n, &ino);
                        superblock.s_tinode++;
                        dwrite((blkno_t) 1, (char *) &superblock);
                    } else {
                        ino.i_nlink = 1;
                        iwrite(n, &ino);
                        mkentry(n);
                    }
                }
            }
        }

    }
}


/* This makes an entry in "lost+found" for inode n */
static void mkentry(uint16_t inum)
{
    struct dinode rootino;
    struct direct dentry;
    uint16_t d;
    uint32_t dmax;

    iread(ROOTINODE, &rootino);
    dmax = rootino.i_size/32;
    for (d = 0; d < dmax; ++d) {
        dirread(&rootino, d, &dentry);
        if (dentry.d_ino == 0 && dentry.d_name[0] == '\0') {
            dentry.d_ino = inum;
            sprintf(dentry.d_name, "l+f%u", inum);
            dirwrite(&rootino, d, &dentry);
            return;
        }
    }
    puts("Sorry... No empty slots in root directory.");
    error |= 4;
}

/*
 *  Getblkno gets a pointer index, and a number of a block in the file.
 *  It returns the number of the block on the disk.  A value of zero
 *  means an unallocated block.
 */

static blkno_t getblkno(struct dinode *ino, blkno_t num)
{
    blkno_t indb;
    blkno_t dindb;
    blkno_t *buf;

    if (num < 18)		/* Direct block */
        return ino->i_addr[num];
    if (num < 256 + 18) {	/* Single indirect */
        indb = ino->i_addr[18];
        if (indb == 0)
            return 0;
        buf = (blkno_t *) daread(indb);
        return buf[num - 18];
    }
    /* Double indirect */
    indb = ino->i_addr[19];
    if (indb == 0)
        return 0;

    buf = (blkno_t *) daread(indb);

    dindb = buf[(num - (18 + 256)) >> 8];
    if (dindb == 0)
        return 0;
    buf = (blkno_t *) daread(dindb);
    return buf[(num - (18 + 256)) & 0x00ff];
}


/*
 *  Setblkno sets the given block number of the given file to the given
 *  disk block number, possibly creating or modifiying the indirect blocks.
 *  A return of zero means there were no blocks available to create an 
 *  indirect block. This should never happen in fsck.
 */
static void setblkno(struct dinode *ino, blkno_t num, blkno_t dnum)
{
    blkno_t indb;
    blkno_t dindb;
    blkno_t *buf;
    /*--    char *zerobuf();--*/


    if (num < 18) {			/* Direct block */
        ino->i_addr[num] = dnum;
    } else if (num < 256 + 18) {	/* Single indirect */
        indb = ino->i_addr[18];
        if (indb == 0)
            panic(missing_ind_blk);

        buf = (blkno_t *) daread(indb);
        buf[num - 18] = dnum;
        dwrite(indb, (char *) buf);
    } else {				/* Double indirect */
        indb = ino->i_addr[19];
        if (indb == 0)
            panic(missing_ind_blk);

        buf = (blkno_t *) daread(indb);
        dindb = buf[(num - (18 + 256)) >> 8];
        if (dindb == 0)
            panic(missing_ind_blk);

        buf = (blkno_t *) daread(dindb);
        buf[(num - (18 + 256)) & 0x00ff] = num;
        dwrite(indb, (char *) buf);
    }
}


/*
 *  blk_alloc0 allocates an unused block.
 *  A returned block number of zero means no more blocks.
 */

/*--- was blk_alloc ---*/
static blkno_t blk_alloc0(struct filesys *filesys)
{
    blkno_t newno;
    blkno_t *buf;
    int16_t j;

    newno = filesys->s_free[--filesys->s_nfree];
    if (!newno) {
        filesys->s_nfree++;
        return 0;
    }

    /* See if we must refill the s_free array */

    if (!filesys->s_nfree) {
        buf = (blkno_t *) daread(newno);
        filesys->s_nfree = buf[0];
        for (j = 0; j < 50; j++) {
            filesys->s_free[j] = buf[j + 1];
        }
    }

    filesys->s_tfree--;

    if (newno < filesys->s_isize || newno >= filesys->s_fsize) {
        puts("Free list is corrupt.  Did you rebuild it?");
        return (0);
    }
    dwrite((blkno_t) 1, (char *) filesys);
    return (newno);
}


static uint16_t lblk = 0;

/* FIXME: we really could do with more buffers on bigger systems (>32K ones) */
static char *daread(uint16_t blk)
{
    static char da_buf[512];

    if (blk == 0)
        panic("rb0");

    if (blk == lblk)
        return da_buf;

    if (lseek(dev_fd, blk * 512L, 0) == -1) {
        perror("lseek");
        exit(1);
    }
    if(read(dev_fd, da_buf, 512) != 512) {
        perror("read");
        exit(1);
    }
    lblk = blk;
    return da_buf;
}

static void dwrite(uint16_t blk, char *addr)
{
    if (lseek(dev_fd, blk * 512L, 0) == -1) {
        perror("lseek");
        exit(1);
    }
    if(write(dev_fd, addr, 512) != 512) {
        perror("write");
        exit(1);
    }
    /* FIXME: why clear this .. it's still valid even if clean */
    lblk = 0;
}

static void iread(uint16_t ino, struct dinode *buf)
{
    struct dinode *addr;

    addr = (struct dinode *) daread((ino >> 3) + 2);
    memcpy(buf, &addr[ino & 7], sizeof(struct dinode));
}

static void iwrite(uint16_t ino, struct dinode *buf)
{
    struct dinode *addr;

    addr = (struct dinode *) daread((ino >> 3) + 2);
    memcpy(&addr[ino & 7], buf, sizeof(struct dinode));
    dwrite((ino >> 3) + 2, (char *) addr);
}

static void dirread(struct dinode *ino, uint16_t j, struct direct *dentry)
{
    blkno_t blkno;
    char *buf;

    blkno = getblkno(ino, (blkno_t) j / 16);
    if (blkno == 0)
        panic(mbidir);
    buf = daread(blkno);
    memcpy(dentry, buf + 32 * (j % 16), 32);
}

static void dirwrite(struct dinode *ino, uint16_t j, struct direct *dentry)
{
    blkno_t blkno;
    char *buf;

    blkno = getblkno(ino, (blkno_t) j / 16);
    if (blkno == 0)
        panic(mbidir);
    buf = daread(blkno);
    memcpy(buf + 32 * (j % 16), dentry, 32);
    dwrite(blkno, buf);
}
