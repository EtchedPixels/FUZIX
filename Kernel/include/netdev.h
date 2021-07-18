#ifndef _NETDEV_H
#define _NETDEV_H

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

#ifndef NSOCKET
#define NSOCKET 8
#endif

struct ksockaddr {
	union {
		struct sockaddr_in sin;
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

	uint8_t wake;		/* REVIEW */

	/* FIXME: need state for shutdown handling */
	uint8_t s_data;			/* Socket we are an accept() for */
	uint8_t s_error;
	uint8_t s_num;			/* To save expensive maths */
	struct ksockaddr src_addr;
	uint8_t src_len;
	struct ksockaddr dst_addr;
	uint8_t dst_len;
	inoptr s_ino;			/* Inode back pointer */
	struct sockproto proto;
};

struct netdevice
{
	uint8_t mac_len;
	const char *name;
	uint16_t flags;
#define IFF_POINTOPOINT		1
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

/* Network layer syscalls */
extern arg_t _netcall(void);

/* Hooks for inode.c into the networking */
extern int sock_close(inoptr ino);
extern int sock_read(inoptr ino, uint8_t flag);
extern int sock_write(inoptr ino, uint8_t flag);
extern bool issocket(inoptr ino);

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
extern int netproto_write(struct socket *s, struct ksockaddr *addr);
extern int netproto_read(struct socket *s);
extern int netproto_shutdown(struct socket *s, uint8_t how);
extern int netproto_close(struct socket *s);
extern void netproto_setup(struct socket *s);
extern void netproto_free(struct socket *s);
#endif
