#include <syscalls.h>
#include <sys/types.h>
#include <sys/socket.h>

int getpeername(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
    __uarg_t args[4];
    args[0] = 9;
    args[1] = fd;
    args[2] = (__uarg_t)addr;
    args[3] = (__uarg_t)addrlen;
    return __netcall(args);
}
