#include <syscalls.h>
#include <sys/mount.h>
#include <sys/statvfs.h>
#include <errno.h>

int statvfs(const char *path, struct statvfs *vfs)
{
    struct {
        struct _uzifilesys fs;
        uint16_t flags;
    } tmp;
    uint16_t ninode;
    if (_statfs(path, (uint8_t *)&tmp) < 0)
        return -1;
    /* Now munge the data : assuming we know the fs type */ 
    switch(tmp.fs.s_mounted) {
        case 12742:
            break;		/* Mounted Fuzix FS */
        default:
            errno = EINVAL;
            return -1;
    }
    
    ninode = (tmp.fs.s_isize - 2) * 8;
    vfs->f_bsize = 512;
    vfs->f_frsize = 512;
    vfs->f_blocks = tmp.fs.s_fsize - tmp.fs.s_isize;
    vfs->f_bfree = tmp.fs.s_tfree;
    vfs->f_bavail = tmp.fs.s_tfree;
    /* Inodes */
    vfs->f_files = ninode;
    vfs->f_ffree = tmp.fs.s_tinode;
    vfs->f_favail = tmp.fs.s_tinode;
    vfs->f_fsid = tmp.fs.s_mounted;

    vfs->f_flag = 0;
    if (tmp.flags & MS_RDONLY)  
        vfs->f_flag |= ST_RDONLY;
    if (tmp.flags & MS_NOSUID)
        vfs->f_flag |= ST_NOSUID;
    if (tmp.flags & MS_NOEXEC)
        vfs->f_flag |= ST_NOEXEC;
    vfs->f_namemax = 30;
    return 0;
}
