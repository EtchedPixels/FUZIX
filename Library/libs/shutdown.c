#include <syscalls.h>
#include <sys/types.h>
#include <sys/socket.h>

int shutdown(int fd, int how)
{
    __uarg_t args[3];
    args[0] = 8;
    args[1] = fd;
    args[2] = how;
    return __netcall(args);
}
