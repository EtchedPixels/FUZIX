#include <kernel.h>
#include <kdata.h>
#include <netdev.h>

#define NSOCKET 8


struct sockaddrs {
	uint32_t laddr;
	uint16_t lport;
	uint32_t daddr;
	uint16_t dport;
};

struct socket
{
	inoptr s_inode;
	uint8_t s_type;
	uint8_t s_error;
	uint8_t s_state;
#define SS_UNUSED		0
#define SS_UNCONNECTED		1
#define SS_BOUND		2
#define SS_LISTENING		3
#define SS_CONNECTING		4
#define SS_CONNECTED		5
#define SS_ACCEPTWAIT		6
	uint8_t s_data;
	struct sockaddrs s_addr;
};

struct socket sockets[NSOCKET];
uint32_t our_address;

static int is_socket(inoptr ino)
{
	if ((ino->c_node.i_mode & F_MASK) == F_SOCK)
		return 1;
	return 0;
}

static int8_t alloc_socket(void)
{
	struct socket *s = sockets;
	while (s < sockets + NSOCKET) {
		if (s->s_state == SS_UNUSED)
			return s - sockets;
		s++;
	}
	return -1;
}

static struct socket *sock_get(int fd, uint8_t *flag)
{
	struct oft *oftp;
	inoptr ino = getinode(fd);
	if (ino == NULLINODE)
		return NULL;
	if (!is_socket(ino)) {
		udata.u_error = EINVAL;
		return NULL;
	}
	if (flag) {
		oftp = of_tab + udata.u_files[fd];
		*flag = oftp->o_access;
	}
	return sockets + ino->c_node.i_nlink;
}

static int sock_pending(struct socket *l)
{
	uint8_t d = l - sockets;
	struct socket *s = sockets;
	while (s < sockets + NSOCKET) {
		if (s->s_state == SS_ACCEPTWAIT && s->s_data == d)
			return s - sockets;
		s++;
	}
	return -1;
}

static int sock_autobind(struct socket *s)
{
	/* TODO */
	s->s_state = SS_BOUND;
	return 0;
}

static struct socket *sock_find_local(uint32_t addr, uint16_t port)
{
	used(addr);
	used(port);
	/* TODO */
	return NULL;
}

static int sa_get(struct sockaddr_in *u, struct sockaddr_in *k)
{
	if (uget(u, k, sizeof(struct sockaddr_in)))
		return -1;
	if (k->sin_family != AF_INET) {
		udata.u_error = EINVAL;
		return -1;
	}
	return 0;
}

static int sa_getlocal(struct sockaddr_in *u, struct sockaddr_in *k)
{
	if (sa_get(u, k))
		return -1;
	if (ntohs(k->sin_port) < 1024 && !super()) {
		udata.u_error = EACCES;
		return -1;
	}
	if (k->sin_addr.s_addr != INADDR_ANY && 
		!IN_LOOPBACK(k->sin_addr.s_addr) &&
		k->sin_addr.s_addr != our_address) {
		udata.u_error = EADDRNOTAVAIL;
		return -1;
	}
	return 0;
}

static int sa_getremote(struct sockaddr_in *u, struct sockaddr_in *k)
{
	if (sa_get(u, k))
		return -1;
	if (ntohs(k->sin_port) >= 512 && ntohs(k->sin_port) < 1024 && !super()) {
		udata.u_error = EACCES;
		return -1;
	}
	return 0;
}

int sock_error(struct socket *s)
{
	udata.u_error = s->s_error;
	s->s_error = 0;
	if (udata.u_error)
		return -1;
	else
		return 0;
}

struct sockinfo {
	uint8_t af;
	uint8_t pf;
	uint8_t type;
	uint8_t priv;
};

#define NSOCKTYPE 3
struct sockinfo socktypes[NSOCKTYPE] = {
	{ AF_INET, SOCK_STREAM, IPPROTO_TCP, 0 },
#define SOCKTYPE_TCP	1
	{ AF_INET, SOCK_DGRAM, IPPROTO_UDP, 0 },
#define SOCKTYPE_UDP	2
	{ AF_INET, SOCK_RAW, 0, 0 }
#define SOCKTYPE_RAW	3
};

arg_t make_socket(struct sockinfo *s, int8_t *np)
{
	int8_t n;
	int8_t uindex;
	int8_t oftindex;
	inoptr ino;

	/* RAW sockets are superuser */
	if (s->priv && esuper())
		return -1;

	/* Start by getting the file and inode table entries */
	if ((uindex = uf_alloc()) == -1)
		return -1;
	if ((oftindex = oft_alloc()) == -1)
		goto nooft;

	if (np)
		n = *np;
	else {
		n = alloc_socket();
		if (n == -1)
			goto noalloc;
	}
	/* We need an inode : FIXME - do we want a pipedev aka Unix ? */
	if (!(ino = i_open(root_dev, 0)))
		goto noalloc;
	/* All good - now set it up */
	/* The nlink cheat needs to be taught to fsck! */
	ino->c_node.i_mode = F_SOCK | 0777;
	ino->c_node.i_nlink = n;	/* Cheat !! */

	of_tab[oftindex].o_inode = ino;
	of_tab[oftindex].o_access = O_RDWR;

	udata.u_files[uindex] = oftindex;

	sockets[n].s_inode = ino;
	sockets[n].s_type = s - socktypes;	/* Pointer or uint8_t best ? */
	sockets[n].s_state = SS_UNCONNECTED;

	if (np)
		*np = n;
	return uindex;

noalloc:
	oft_deref(oftindex);	/* Will call i_deref! */
nooft:
	udata.u_files[uindex] = NO_FILE;
	return -1;
}

/*******************************************
socket (af, type, pf)           Function ??
uint16_t af;
uint16_t type;
uint16_t pf;
********************************************/

#define afv   (uint16_t)udata.u_argn
#define typev (uint16_t)udata.u_argn1
#define pfv   (uint16_t)udata.u_argn2

arg_t _socket(void)
{
	struct sockinfo *s = socktypes;

	/* Find the socket */
	while (s && s < socktypes + NSOCKTYPE) {
		if (s->af == afv && s->type == typev) {
			if (s->pf == 0 || s->pf == pfv)
				return make_socket(s, NULL);
		}
		s++;
	}
	return -EAFNOSUPPORT;
}

#undef af
#undef type
#undef pf

/*******************************************
listen(fd, qlen)           Function ??
int fd;
int qlen;
********************************************/
#define fd   (int16_t)udata.u_argn
#define qlen (int16_t)udata.u_argn1

arg_t _listen(void)
{
	struct socket *s = sock_get(fd, NULL);
	if (s == NULL)
		return -1;
	if (s->s_state == SS_UNCONNECTED && sock_autobind(s))
		return -1;
	if (s->s_type != SOCKTYPE_TCP || s->s_state != SS_BOUND) {
		udata.u_error = EINVAL;
		return -1;	
	}
	/* Call the protocol services */
	s->s_state = SS_LISTENING;
	return 0;
}

/*******************************************
bind(fd, addr)           Function ??
int fd;
struct sockaddr_*in addr;
********************************************/
#define fd   (int16_t)udata.u_argn
#define addr (struct sockaddr_in *)udata.u_argn1

arg_t _bind(void)
{
	struct socket *s = sock_get(fd, NULL);
	struct sockaddr_in sin;
	if (s == NULL)
		return -1;
	if (s->s_state != SS_UNCONNECTED)
		return -1;
	if (sa_getlocal(addr, &sin) == -1)
		return -1;
	if (sock_find_local(sin.sin_addr.s_addr, sin.sin_port)) {
		udata.u_error = EADDRINUSE;
		return -1;
	}
	s->s_state = SS_BOUND;
	return 0;
}

/*******************************************
connect(fd, addr)           Function ??
int fd;
struct sockaddr_*in addr;
********************************************/
#define fd   (int16_t)udata.u_argn
#define addr (struct sockaddr_in *)udata.u_argn1

arg_t _connect(void)
{
	uint8_t flag;
	struct socket *s = sock_get(fd, &flag);
	struct sockaddr_in sin;
	if (s == NULL)
		return -1;
	if (s->s_state == SS_CONNECTING) {
		udata.u_error = EALREADY;
		return -1;
	}
	if (s->s_state == SS_UNCONNECTED && sock_autobind(s))
		return -1;
	
	if (s->s_state == SS_BOUND) {
		if (sa_getremote(addr, &sin) == -1)
			return -1;
		s->s_state = SS_CONNECTING;
		/* Protocol op to kick off */
	}

	do {
		/* FIXME: return EINPROGRESS not EINTR for SS_CONNECTING */
		if (psleep_flags(s, flag))
			return -1;
		/* Protocol state check */
	} while (s->s_state == SS_CONNECTING);
	return sock_error(s);
}

/*******************************************
accept(fd, addr)           Function ??
int fd;
struct sockaddr_*in addr;
********************************************/
#define fd   (int16_t)udata.u_argn

/* Note: We don't do address return, the library can handle it */
arg_t _accept(void)
{
	uint8_t flag;
	struct socket *s = sock_get(fd, &flag);
	int8_t n;
	int8_t nfd;

	if (s == NULL)
		return -1;
	if (s->s_state == SS_LISTENING) {
		udata.u_error = EALREADY;
		return -1;
	}
	
	while ((n = sock_pending(s)) != -1) {
		if (psleep_flags(s, flag))
			return -1;
		if (s->s_error)
			return sock_error(s);
	}
	if ((nfd = make_socket(&socktypes[SOCKTYPE_TCP], &n)) == -1)
		return -1;
	sockets[n].s_state = SS_CONNECTED;
	return nfd;
}

/*******************************************
getsockaddrs(fd, addr)           Function ??
int fd;
struct sockaddr_*in addr;
********************************************/
#define fd   (int16_t)udata.u_argn
#define addrs (uint8_t *)udata.u_argn1

arg_t _getsockaddrs(void)
{
	struct socket *s = sock_get(fd, NULL);

	if (s == NULL)
		return -1;
	return uput(&s->s_addr, addrs, sizeof(struct sockaddrs));
}

