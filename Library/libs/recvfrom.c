#include <syscalls.h>
#include <sys/types.h>
#include <sys/socket.h>

ssize_t recvfrom(int fd, void *buf, size_t len, int flags, struct sockaddr *addr, socklen_t *addrlen)
{
    __uarg_t args[7];
    args[0] = 7;
    args[1] = fd;
    args[2] = (__uarg_t)buf;
    args[3] = len;
    args[4] = flags;
    args[5] = (__uarg_t)addr;
    args[6] = (__uarg_t)addrlen;
    return __netcall(args);
}
