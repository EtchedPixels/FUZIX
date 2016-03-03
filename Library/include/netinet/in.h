#ifndef _NETINET_IN_H
#define _NETINET_IN_H

#include <sys/socket.h>

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;

struct in_addr {
  uint32_t s_addr;
};

struct sockaddr_in {
  sa_family_t sin_family;
  in_port_t sin_port;
  struct in_addr sin_addr;
  uint8_t sin_zero[8];
};

/* This one is used internally to deal with many argumented net
   functions */
struct __fuzix_sockio {
  uint16_t sio_flags;		/* Keep the order so we can use partial */
  uint16_t sio_addr_len;	/* structs for simple cases */
  struct sockaddr_in sio_addr;
};

#define INADDR_ANY		0L
#define INADDR_BROADCAST	0xFFFFFFFFUL
#define INADDR_LOOPBACK		0x7F000001UL
#define INADDR_NONE	((uint32_t)(-1))

#define IPPROTO_ICMP	1
#define IPPROTO_TCP	6
#define IPPROTO_UDP	17
#define IPPROTO_RAW	255

#endif
