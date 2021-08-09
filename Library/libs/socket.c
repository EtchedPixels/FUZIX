#include <syscalls.h>
#include <sys/types.h>
#include <sys/socket.h>

int socket(int domain, int type, int protocol)
{
    __uarg_t args[4];
    args[0] = 0;
    args[1] = domain;
    args[2] = type;
    args[3] = protocol;
    return __netcall(args);
}

    