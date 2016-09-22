#ifndef _SYS_SOCKET_H
#define _SYS_SOCKET_H

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

#endif
