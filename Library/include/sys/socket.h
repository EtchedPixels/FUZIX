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

int accept(int __fd, struct sockaddr *__addr, socklen_t *__addrlen);
int bind(int __fd, const struct sockaddr *__addr, socklen_t __addrlen);
int connect(int __fd, const struct sockaddr *__addr, socklen_t __addrlen);
int getpeername(int __fd, struct sockaddr *__addr, socklen_t *__addrlen);
int getsockname(int __fd, struct sockaddr *__addr, socklen_t *__addrlen);
int listen(int __fd, int __num);
ssize_t recv(int __fd, void *__buf, size_t __len, int __flags);
ssize_t recvfrom(int __fd, void *__buf, size_t __len, int __flags,
                  struct sockaddr *__addr, socklen_t *__addrlen);
ssize_t send(int __fd, const void *__buf, size_t __len, int __flags);
ssize_t sendto(int __fd, const void *__buf, size_t __len, int __flags,
                  const struct sockaddr *__addr, socklen_t __addrlen);
int shutdown(int __fd, int __how);
int socket(int __domain, int __type, int __protocol);

/* LP32 */
#if defined(__mc68000__) || defined(__XTENSA_CALL0_ABI__) || defined(__ARM_EABI__)
typedef uint32_t	__uarg_t;
#else
/* LP16 */
typedef	uint16_t	__uarg_t;
#endif

#endif
