#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <syscalls.h>

ssize_t recvfrom(int fd, void *buf, size_t len, int flags, struct sockaddr *addr,
    socklen_t *addr_len)
{
    struct _sockio tmp;
    int err;
    tmp.sio_flags = flags;
    tmp.sio_addr_len = 0;
    if (addr) {
        tmp.sio_addr_len = sizeof(tmp.sio_addr);
        if (tmp.sio_addr_len < *addr_len)
            tmp.sio_addr_len = *addr_len;
    }
    err = _recvfrom(fd, buf, len, &tmp);
    if (err == 0 && tmp.sio_addr_len) {
        memcpy(addr, tmp.sio_addr, tmp.sio_addr_len);
        if (addr_len)
            *addr_len = tmp.sio_addr_len;
    }
    return err;
}