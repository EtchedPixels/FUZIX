
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

/* This one is used internally to deal with many argumented net
   functions */
struct sockio {
  uint16_t sio_flags;		/* Keep the order so we can use partial */
  uint16_t sio_addr_len;	/* structs for simple cases */
  struct sockaddr_in sio_addr;
};

#define INADDR_ANY		0L
#define INADDR_BROADCAST	0xFFFFFFFFUL
#define INADDR_LOOPBACK		0x7F000001UL
#define IN_LOOPBACK(a)		(((a) >> 24) == 0x7F)

/* Network layer syscalls */
extern arg_t _socket(void);
extern arg_t _listen(void);
extern arg_t _bind(void);
extern arg_t _connect(void);
extern arg_t _accept(void);
extern arg_t _getsockaddrs(void);
extern arg_t _sendto(void);
extern arg_t _recvfrom(void);

/* Hooks for inode.c into the networking */
extern void sock_close(inoptr ino);
extern int netd_sock_read(inoptr ino, uint8_t flag);
extern int is_netd(void);
extern int sock_write(inoptr ino, uint8_t flag);
extern bool issocket(inoptr ino);
