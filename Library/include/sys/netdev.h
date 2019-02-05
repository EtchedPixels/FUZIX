#ifndef _DEV_NET_NETDEV_H
#define _DEV_NET_NETDEV_H

#define AF_INET		1

#define SOCK_RAW	1
#define SOCK_DGRAM	2
#define SOCK_STREAM	3

#define IPPROTO_ICMP	1
#define IPPROTO_TCP	6
#define IPPROTO_UDP	17
#define IPPROTO_RAW	255

#include <netinet/in.h>


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


#ifndef NSOCKET
#define NSOCKET 8
#endif


struct sockaddrs {
	uint32_t addr;
	uint16_t port;
};

struct socket
{
	uint8_t s_type;
	uint8_t s_state;
#define SS_UNUSED		0	/* Free slot */
#define SS_INIT			1	/* Initializing state (for IP offloaders) */
#define SS_UNCONNECTED		2	/* Created */
#define SS_BOUND		3	/* Bind or autobind */
#define SS_LISTENING		4	/* Listen called */
#define SS_ACCEPTING		5	/* Accepting in progress */
#define SS_ACCEPTWAIT		6	/* Waiting for accept to harvest */
#define SS_CONNECTING		7	/* Connect initiated */
#define SS_CONNECTED		8	/* Connect has completed */
#define SS_CLOSEWAIT		9	/* Remote has closed */
#define SS_CLOSING		10	/* Protocol close in progress */
#define SS_CLOSED		11	/* Protocol layers done, not close()d */
#define SS_DEAD			12
	/* FIXME: need state for shutdown handling */
	uint8_t s_data;			/* Socket we are an accept() for */
	uint8_t s_error;
	uint8_t s_num;			/* To save expensive maths */
	struct sockaddrs s_addr[3];
#define SADDR_SRC	0
#define SADDR_DST	1
#define SADDR_TMP	2
	uint8_t s_flag;
#define SFLAG_ATMP	1		/* Use SADDR_TMP */
	uint8_t s_iflag;		/* Flags touched in IRQ handlers */
#define SI_SHUTR	1
#define SI_SHUTW	2
#define SI_DATA		4		/* Data is ready */
#define SI_EOF		8		/* At EOF */
#define SI_THROTTLE	16		/* Transmit is throttled */
	void *s_priv;			/* Private pointer for lower layers */
	void *s_ino;
};

#define NSOCKTYPE 3
#define SOCKTYPE_TCP	0
#define SOCKTYPE_UDP	1
#define SOCKTYPE_RAW	2

struct netdevice
{
	uint8_t mac_len;
	const char *name;
	uint16_t flags;
#define IFF_POINTOPOINT		1
};

/* Not applicable in userspace ? */


/* extern struct socket sockets[NSOCKET]; */
/* extern uint32_t our_address; */

/* /\* Network layer syscalls *\/ */
/* extern arg_t _socket(void); */
/* extern arg_t _listen(void); */
/* extern arg_t _bind(void); */
/* extern arg_t _connect(void); */
/* extern arg_t _accept(void); */
/* extern arg_t _getsockaddrs(void); */
/* extern arg_t _sendto(void); */
/* extern arg_t _recvfrom(void); */
/* extern arg_t _shutdown(void); */

/* /\* Hooks for inode.c into the networking *\/ */
/* extern void sock_close(inoptr ino); */
/* extern int sock_read(inoptr ino, uint8_t flag); */
/* extern int sock_write(inoptr ino, uint8_t flag); */
/* extern bool issocket(inoptr ino); */

/* /\* Hooks between the network implementation and the socket layer *\/ */
/* extern int net_init(struct socket *s); */
/* extern int net_bind(struct socket *s); */
/* extern int net_connect(struct socket *s); */
/* extern void net_close(struct socket *s); */
/* extern int net_listen(struct socket *s); */
/* extern arg_t net_read(struct socket *s, uint8_t flag); */
/* extern arg_t net_write(struct socket *s, uint8_t flag); */
/* extern arg_t net_ioctl(uint8_t op, void *p); */
/* extern arg_t net_shutdown(struct socket *s, uint8_t how); /\* bit 0 rd, bit 1 wr *\/ */
/* extern void netdev_init(void); */
/* extern struct socket *sock_find(uint8_t type, uint8_t sv, struct sockaddrs *sa); */
/* extern void sock_init(void); */

/* extern struct netdevice net_dev; */
/* Socket IOCTLS */

#define SIOCGIFADDR      0x4401
#define SIOCSIFADDR      0x4402
#define SIOCGIFMASK      0x4403
#define SIOCSIFMASK      0x4404
#define SIOCGIFHWADDR    0x4405
#define SIOCSIFHWADDR    0x4406
#define SIOCGIFGW        0x4407
#define SIOCSIFGW        0x4408

#define IFNAMSIZ 5

struct ifreq {
	char ifr_name[IFNAMSIZ]; /* Interface name */
	union {
		struct sockaddr ifr_addr;
		struct sockaddr ifr_dstaddr;
		struct sockaddr ifr_broadaddr;
		struct sockaddr ifr_netmask;
		struct sockaddr ifr_hwaddr;
		struct sockaddr ifr_gwaddr; /* belongs in router.h ?*/
		uint8_t         ifr_flags;
		uint16_t        ifr_ifindex;
		uint16_t        ifr_metric;
		uint16_t        ifr_mtu;
	} ifr;
};
#endif
