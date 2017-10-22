#include <syscalls.h>
#include <sys/mount.h>

int remount(const char *target, int flags)
{
    return _umount(target, flags | MS_REMOUNT);
}
