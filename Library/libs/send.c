#include <sys/types.h>
#include <sys/socket.h>

ssize_t send(int fd, void *buf, size_t len, int flags)
{
    return sendto(fd, buf, len, flags, NULL, 0);
}
