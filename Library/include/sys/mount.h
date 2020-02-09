#ifndef _SYS_MOUNT_H
#define _SYS_MOUNT_H

#define MS_RDONLY	0x01
#define MS_NOSUID	0x02		/* Not implemented yet */
#define MS_NOEXEC	0x04		/* Not implemented yet */
#define MS_REMOUNT	0x80

extern int umount(const char *__target);
extern int remount(const char *__target, int flags);

#endif
