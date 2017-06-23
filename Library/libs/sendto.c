#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <syscalls.h>

ssize_t sendto(int fd, void *buf, size_t len, int flags, struct sockaddr *addr,
    socklen_t addr_len)
{
    struct _sockio tmp;
    tmp.sio_flags = flags;
    tmp.sio_addr_len = 0;
    if (addr) {
        int len = sizeof(tmp.sio_addr);
        if(addr_len < len)
            len = addr_len;
        tmp.sio_addr_len = len;
        memcpy(tmp.sio_addr, addr, addr_len);
    }
    return _sendto(fd, buf, len, &tmp);
}