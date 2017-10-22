#include <syscalls.h>
#include <sys/mount.h>

int umount(const char *dev)
{
    return _umount(dev, 0);
}
