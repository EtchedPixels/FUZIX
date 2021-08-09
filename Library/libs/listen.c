#include <syscalls.h>
#include <sys/types.h>
#include <sys/socket.h>

int listen(int fd, int num)
{
    __uarg_t args[3];
    args[0] = 1;
    args[1] = fd;
    args[2] = num;
    return __netcall(args);
}

    