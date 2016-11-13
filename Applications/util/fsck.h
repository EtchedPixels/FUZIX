#define __UZIFS_DOT_H__

#define ROOTINODE 1
#define SMOUNTED 12742   /* Magic number to specify mounted filesystem */
#define SMOUNTED_WRONGENDIAN 50737U   /* byteflipped */

extern int dev_fd;
extern int dev_offset;
int fd_open(char *name);
void panic(char *s);

extern uint16_t swizzle16(uint32_t v);
extern uint32_t swizzle32(uint32_t v);
extern int swizzling;

struct direct {
        uint16_t d_ino;
        char     d_name[30];
};

typedef uint16_t blkno_t;    /* Can have 65536 512-byte blocks in filesystem */

struct dinode {
    uint16_t 	i_mode;
    uint16_t 	i_nlink;
    uint16_t 	i_uid;
    uint16_t 	i_gid;
    uint32_t    i_size;
    uint32_t	i_atime;
    uint32_t	i_mtime;
    uint32_t	i_ctime;
    blkno_t	i_addr[20];
};               /* Exactly 64 bytes long! */

#define F_REG   0100000
#define F_DIR   040000
#define F_PIPE  010000
#define F_BDEV  060000
#define F_CDEV  020000

#define F_MASK  0170000

struct filesys {
    uint16_t    s_mounted;
    uint16_t    s_isize;
    uint16_t    s_fsize;
    int16_t     s_nfree;
    blkno_t     s_free[50];
    int16_t     s_ninode;
    uint16_t    s_inode[50];
    uint8_t     s_fmod;
    uint8_t	s_timeh;	/* top bits of time */
    uint32_t    s_time;
    blkno_t     s_tfree;
    uint16_t    s_tinode;
    uint16_t    s_mntpt;
};
