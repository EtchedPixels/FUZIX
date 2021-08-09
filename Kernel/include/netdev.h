#ifndef _NETDEV_H
#define _NETDEV_H

#ifdef CONFIG_NET

#define AF_INET		1
#define PF_INET		AF_INET

#define SOCK_RAW	1
#define SOCK_DGRAM	2
#define SOCK_STREAM	3
#define SOCK_SEQPACKET	4

#define IPPROTO_ICMP	1
#define IPPROTO_TCP	6
#define IPPROTO_UDP	17
#define IPPROTO_RAW	255


#define HW_NONE		0	/* e.g. loopback */
#define HW_ETH		1
#define HW_WLAN		2
#define HW_SLIP		3
#define HW_CSLIP	4

struct in_addr {
  uint32_t s_addr;
};

struct sockaddr_in {
  uint16_t sin_family;
  uint16_t sin_port;
  struct in_addr sin_addr;
  uint8_t sin_zero[8];
};

struct sockaddr_hw {
  uint16_t shw_family;
  uint8_t shw_addr[14];
};

#define INADDR_ANY		0L
#define INADDR_BROADCAST	0xFFFFFFFFUL
#define INADDR_LOOPBACK		0x7F000001UL
#define IN_LOOPBACK(a)		(((a) >> 24) == 0x7F)

#ifndef NSOCKET
#define NSOCKET 8
#endif

struct ksockaddr {
	union {
		struct sockaddr_in sin;
		struct sockaddr_hw hw;
		uint16_t family;
	} sa;
};

#define MIN_SOCKADDR		sizeof(uint16_t)
#define MAX_SOCKADDR		sizeof(struct ksockaddr)

struct sockproto
{
	/* Dummy for now */
	uint8_t slot;
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
#define SS_DEAD			12	/* Closed byuser space but not yet
					   free of any stack resources */
	uint8_t s_iflags;
#define SI_SHUTR	1
#define SI_SHUTW	2
#define SI_DATA		4		/* Data is ready */
#define SI_EOF		8		/* At EOF */
#define SI_THROTTLE	16		/* Transmit is throttled */
#define SI_WAIT		32		/* Wait helper */

	uint8_t s_wake;		/* REVIEW */

	/* FIXME: need state for shutdown handling */
	uint8_t s_error;
	uint8_t s_num;			/* To save expensive maths */
	uint8_t s_parent;		/* For accept */
	uint8_t s_class;		/* Class of socket (stream etc) */
	uint16_t s_protocol;		/* Protocol given in socket() */
	struct ksockaddr src_addr;
	uint8_t src_len;
	struct ksockaddr dst_addr;
	uint8_t dst_len;
	inoptr s_ino;			/* Inode back pointer */
	struct sockproto proto;
};

struct udata_net {
	arg_t args[7];			/* Arguments to netcall */
	inoptr inode;			/* Inode bound to socket */
	uint16_t sock;			/* Socket index */
	uint8_t sig;			/* Signal to deliver */
	uint8_t addrlen;		/* Length of the address */
	struct ksockaddr addrbuf;	/* A network address */
};

extern struct socket sockets[NSOCKET];
extern uint32_t our_address;

#define IFNAMSIZ	8
/*
 * Interfaces for network configuration
 */

struct ifreq {
	char ifr_name[IFNAMSIZ];
	union {
		struct ksockaddr ifr_addr;
		struct ksockaddr ifr_dstaddr;
		struct ksockaddr ifr_broadaddr;
		struct ksockaddr ifr_netmask;
		struct ksockaddr ifr_gwaddr;
		struct ksockaddr ifr_hwaddr;
		short ifr_flags;
		int ifr_ifindex;
		int ifr_mtu;
	} ifr_ifru;
};

#define ifr_addr	ifr_ifru.ifr_addr
#define ifr_dstaddr	ifr_ifru.ifr_dstaddr
#define ifr_broadaddr	ifr_ifru.ifr_broadaddr
#define ifr_netmask	ifr_ifru.ifr_netmask
#define ifr_hwaddr	ifr_ifru.ifr_hwaddr
#define ifr_gwaddr	ifr_ifru.ifr_gwaddr
#define ifr_flags	ifr_ifru.ifr_flags
#define ifr_ifindex	ifr_ifru.ifr_ifindex
#define ifr_mtu		ifr_ifru.ifr_mtu

#define IFF_UP		0x0001
#define IFF_BROADCAST	0x0002
#define IFF_LOOPBACK	0x0004
#define IFF_POINTOPOINT	0x0008
#define IFF_RUNNING	0x0010
#define IFF_NOARP	0x0020
#define IFF_PROMISC	0x0040
#define IFF_MULTICAST	0x0080
#define IFF_LINKUP	0x0100

#define SIOCGIFNAME	0x0400
#define SIOCGIFINDEX	0x0401
#define SIOCGIFFLAGS	0x0402
#define SIOCSIFFLAGS	(0x0403|IOCTL_SUPER)
#define SIOCGIFADDR	0x0404
#define SIOCSIFADDR	(0x0405|IOCTL_SUPER)
#define SIOCGIFDSTADDR	0x0406
#define SIOCSIFDSTADDR	(0x0407|IOCTL_SUPER)
#define SIOCGIFBRDADDR	0x0408
#define SIOCSIFBRDADDR	(0x0409|IOCTL_SUPER)
#define SIOCGIFNETMASK	0x040A
#define SIOCSIFNETMASK	(0x040B|IOCTL_SUPER)
#define SIOCGIFHWADDR	0x040C
#define SIOCSIFHWADDR	(0x040D|IOCTL_SUPER)
#define SIOCGIFMTU	0x040E
#define SIOCSIFMTU	(0x040F|IOCTL_SUPER)
/* These two are a Fuzix specific thing. It's much easier to think about
   simple interfaces this way than require 'route'. The native net may
   well require 'route' but can just omit this ioctl pair */
#define SIOCGIFGWADDR	0x0410
#define SIOCSIFGWADDR	(0x0411|IOCTL_SUPER)


/* Network layer syscalls */
extern arg_t _netcall(void);

/* Hooks for inode.c into the networking */
extern int sock_close(inoptr ino);
extern int sock_read(inoptr ino, uint8_t flag);
extern int sock_write(inoptr ino, uint8_t flag);
extern arg_t sock_ioctl(inoptr ino, int req, char *data);
extern uint_fast8_t issocket(inoptr ino);

/* Hooks between the core kernel and networking */
extern void net_free(void);
extern int net_read(void);
extern int net_write(void);
extern int net_close(void);
extern void *net_poll(void);
extern void sock_init(void);
extern int net_syscall(void);
/* Helpers between core and protocol */
extern void net_setup(struct socket *s);
extern void net_inode(void);
extern int net_ioctl(int req, char *data);
extern arg_t sock_ioctl(inoptr ino, int req, char *data);

extern uint8_t sock_wake[NSOCKET];

/* Hooks betweek the networking framework and the implementation */
extern int netproto_socket(void);
extern int netproto_listen(struct socket *s);
extern int netproto_find_local(struct ksockaddr *addr);
extern int netproto_autobind(struct socket *s);
extern int netproto_accept(struct socket *s);
extern int netproto_accept_complete(struct socket *s);
extern int netproto_bind(struct socket *s);
extern int netproto_begin_connect(struct socket *s);
extern struct socket *netproto_sockpending(struct socket *s);
extern arg_t netproto_write(struct socket *s, struct ksockaddr *addr);
extern int netproto_read(struct socket *s);
extern arg_t netproto_shutdown(struct socket *s, uint8_t how);
extern int netproto_close(struct socket *s);
extern void netproto_setup(struct socket *s);
extern void netproto_free(struct socket *s);
extern arg_t netproto_ioctl(struct socket *s, int requ, char *data);

#endif
#endif
