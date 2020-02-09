#ifndef _SYS_STATVFS_H
#define _SYS_STATVFS_H

#include <sys/types.h>

struct statvfs {
    unsigned long f_bsize;
    unsigned long f_frsize;
    fsblkcnt_t f_blocks;
    fsblkcnt_t f_bfree;
    fsblkcnt_t f_bavail;
    fsfilcnt_t f_files;
    fsfilcnt_t f_ffree;
    fsfilcnt_t f_favail;
    unsigned long f_fsid;
    unsigned long f_flag;
    unsigned long f_namemax;
};

#define ST_RDONLY	0x01
#define ST_NOSUID	0x02
#define ST_NOEXEC	0X04

extern int fstatvfs(int, struct statvfs *);
extern int statvfs(const char *, struct statvfs *);

#endif
