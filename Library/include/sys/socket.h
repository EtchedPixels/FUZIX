#ifndef _SYS_SOCKET_H
#define _SYS_SOCKET_H

#include "types.h"

#define AF_INET		1

#define SOCK_RAW	1
#define SOCK_DGRAM	2
#define SOCK_STREAM	3


typedef int socklen_t;
typedef uint16_t sa_family_t;

struct sockaddr {
  sa_family_t sa_family;
  uint8_t sa_data[14];
};

ssize_t recv(int __fd, void *__buf, size_t __len, int __flags);
ssize_t recvfrom(int __fd, void *__buf, size_t __len, int __flags,
                  struct sockaddr *__addr, socklen_t *__addrlen);
ssize_t send(int __fd, const void *__buf, size_t __len, int __flags);
ssize_t sendto(int __fd, const void *__buf, size_t __len, int __flags,
                  const struct sockaddr *__addr, socklen_t __addrlen);

#endif
