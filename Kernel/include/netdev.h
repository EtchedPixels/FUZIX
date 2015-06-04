
#define AF_INET		1

#define SOCK_RAW	1
#define SOCK_DGRAM	2
#define SOCK_STREAM	3

#define IPPROTO_ICMP	1
#define IPPROTO_TCP	6
#define IPPROTO_UDP	17
#define IPPROTO_RAW	255

struct in_addr {
  uint32_t s_addr;
};

struct sockaddr_in {
  uint16_t sin_family;
  uint16_t sin_port;
  struct in_addr sin_addr;
  uint8_t sin_zero[8];
};

#define INADDR_ANY		0L
#define INADDR_BROADCAST	0xFFFFFFFFUL
#define INADDR_LOOPBACK		0x7F000001UL
#define IN_LOOPBACK(a)		(((a) >> 24) == 0x7F)
