#include <kernel.h>
#include <kdata.h>
#include <netdev.h>

#ifdef CONFIG_NET

struct socket sockets[NSOCKET];
#ifdef BIG_ENDIAN
uint32_t our_address = 0xC0000001;
#else
uint32_t our_address = 0x010000C0;
#endif

#define IN2SOCK(ino)		(sockets + (ino)->c_node.i_addr[0])

static uint16_t nextauto = 5000;

bool issocket(inoptr ino)
{
	if ((ino->c_node.i_mode & F_MASK) == F_SOCK)
		return 1;
	return 0;
}

int sock_write(inoptr ino, uint8_t flag)
{
	struct socket *s = IN2SOCK(ino);
	int r;

	/* FIXME: IRQ protection */
	while(1) {
		if (s->s_state != SS_CONNECTED && s->s_state != SS_CLOSING) {
			/* FIXME: handle shutdown states properly */
			if (s->s_state >= SS_CLOSEWAIT) {
				ssig(udata.u_ptab, SIGPIPE);
				udata.u_error = EPIPE;
			} else
				udata.u_error = EINVAL;
			return -1;
		}
		switch(r = net_write(s, flag)) {
			case -2:
				s->s_iflag |= SI_THROTTLE;
				break;
			case -1:
				return -1;
			default:
				return r;
		}
		if ((s->s_iflag & SI_THROTTLE) &&
			psleep_flags(&s->s_iflag, flag) == -1)
				return -1;
	}
}

int sock_read(inoptr ino, uint8_t flag)
{
	struct socket *s = IN2SOCK(ino);
	return net_read(s, flag);
}

/* Wait to leave a state. This will eventually need interrupt locking etc */
static int sock_wait_leave(struct socket *s, uint8_t flag, uint8_t state)
{
	irqflags_t irq;
	irq = di();
	while (s->s_state == state) {
		if (psleep_flags(s, flag)) {
			irqrestore(irq);
			return -1;
		}
		di();
	}
	irqrestore(irq);
	return 0;
}

/* Wait to enter a state. This will eventually need interrupt locking etc */
static int sock_wait_enter(struct socket *s, uint8_t flag, uint8_t state)
{
	irqflags_t irq;
	irq = di();
	while (s->s_state != state) {
		if (psleep_flags(s, flag)) {
			irqrestore(irq);
			return -1;
		}
	}
	irqrestore(irq);
	return 0;
}

static struct socket *alloc_socket(void)
{
	irqflags_t irq = di();
	regptr struct socket *s = sockets;
	while (s < sockets + NSOCKET) {
		if (s->s_state == SS_UNUSED) {
			s->s_state = SS_INIT;
			s->s_iflag = 0;
			irqrestore(irq);
			return s;
		}
		s++;
	}
	irqrestore(irq);
	return NULL;
}

struct socket *sock_alloc_accept(struct socket *s)
{
	regptr struct socket *n = alloc_socket();
	int sockno;
	if (n == NULL)
		return NULL;

	sockno = n->s_num;
	memcpy(n, s, sizeof(*n));
	n->s_num = sockno;
	n->s_state = SS_ACCEPTING;
	n->s_data = s->s_num;
	return n;
}

void sock_wake_listener(struct socket *s)
{
	wakeup(&sockets[s->s_data]);
}

void sock_close(inoptr ino)
{
	/* For the moment */
	struct socket *s = IN2SOCK(ino);;
	net_close(s);
	/* Dead but not unbound from netd activity yet */
	s->s_state = SS_DEAD;
}

void sock_closed(struct socket *s)
{
	s->s_state = SS_UNUSED;
	s->s_ino->c_node.i_addr[0] = 0;
	s->s_ino->c_flags |= CDIRTY;
}

/*
 *	This chunk of logic wants pushing out into the stack implementation.
 *	We shouldn't have knowledge of the addressing in the core code.
 *
 *	Likewise this means that we need to push autobinding, bind address
 *	checks and socket type checks into the lower stack and only care
 *	about datagram versus stream.
 *
 *	(A good thought experiment would be verifying that you could support
 *	something like AX.25 and a host mode TNC via net_native)
 */
struct socket *sock_find(uint8_t type, uint8_t sv, struct sockaddrs *sa)
{
	regptr struct socket *sp;
	regptr struct sockaddrs *a;

	for (sp = sockets; sp < sockets + NSOCKET; sp++) {
		a = &sp->s_addr[sv];
		if (sp->s_state == SS_UNUSED || sp->s_type != type)
			continue;
		if (a->port != sa->port)
			continue;
		if (a->addr != sa->addr && a->addr != INADDR_ANY)
			continue;
		return sp;
	}
	return NULL;
}


static struct socket *sock_get(int fd, uint8_t *flag)
{
	struct oft *oftp;
	inoptr ino = getinode(fd);
	if (ino == NULLINODE)
		return NULL;
	if (!issocket(ino)) {
		udata.u_error = EINVAL;
		return NULL;
	}
	if (flag) {
		oftp = of_tab + udata.u_files[fd];
		*flag = oftp->o_access;
	}
	return IN2SOCK(ino);
}

static struct socket *sock_pending(struct socket *l)
{
	uint8_t d = l->s_num;
	struct socket *s = sockets;
	while (s < sockets + NSOCKET) {
		if (s->s_state == SS_ACCEPTWAIT && s->s_data == d)
			return s;
		s++;
	}
	return NULL;
}

static int sock_autobind(struct socket *s)
{
	static struct sockaddrs sa;
	sa.addr = INADDR_ANY;
	do {
		sa.port = ntohs(nextauto++);
	} while(sock_find(s->s_type, SADDR_SRC, &sa));
	memcpy(&s->s_addr[SADDR_SRC], &sa, sizeof(sa));
	return net_bind(s);
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

static int sa_put(struct socket *s, int type, struct sockaddr_in *u)
{
	struct sockaddrs *sa = &s->s_addr[type];
	static struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_addr.s_addr = sa->addr;
	sin.sin_port = sa->port;
	sin.sin_family = AF_INET;
	if (u && uput(u, &sin, sizeof(sin)) == -1)
		return -1;
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
	uint8_t type;
	uint8_t pf;
	uint8_t priv;
};

struct sockinfo socktypes[NSOCKTYPE] = {
	{ AF_INET, SOCK_STREAM, IPPROTO_TCP, 0 },
	{ AF_INET, SOCK_DGRAM, IPPROTO_UDP, 0 },
	{ AF_INET, SOCK_RAW, 0, 0 }
};

arg_t make_socket(struct sockinfo *s, struct socket **np)
{
	struct socket *n;
	int8_t uindex;
	int8_t oftindex;
	inoptr ino;

	/* RAW sockets are superuser */
	if (s->priv && esuper())
		return -1;

	if (np)
		n = *np;
	else {
		n = alloc_socket();
		if (n == NULL)
			return -1;
	}
	n->s_type = s - socktypes;	/* Pointer or uint8_t best ? */
	n->s_state = SS_INIT;

	if (net_init(n) == -1)
		goto nosock;

	/* Start by getting the file and inode table entries */
	if ((uindex = uf_alloc()) == -1)
		goto nosock;
	if ((oftindex = oft_alloc()) == -1)
		goto nooft;

	/* We need an inode : FIXME - do we want a pipedev aka Unix ? */
	if (!(ino = i_open(root_dev, 0)))
		goto noalloc;
	/* All good - now set it up */
	ino->c_node.i_mode = F_SOCK | 0777;
	ino->c_node.i_addr[0] = n->s_num;
	ino->c_readers = 1;
	ino->c_writers = 1;

	n->s_ino = ino;

	of_tab[oftindex].o_inode = ino;
	of_tab[oftindex].o_access = O_RDWR;

	udata.u_files[uindex] = oftindex;

	sock_wait_leave(n, 0, SS_INIT);
	if (np)
		*np = n;
	return uindex;

noalloc:
	oft_deref(oftindex);	/* Will call i_deref! */
nooft:
	udata.u_files[uindex] = NO_FILE;
nosock:
	n->s_state = SS_UNUSED;
	return -1;
}

/*******************************************
socket (af, type, pf)            Function 90
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
			if (pfv == 0 || s->pf == 0 || s->pf == pfv)
				return make_socket(s, NULL);
		}
		s++;
	}
	udata.u_error = EAFNOSUPPORT;
	return -1;
}

#undef af
#undef type
#undef pf

/*******************************************
listen(fd, qlen)                 Function 91
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
	return net_listen(s);
}

#undef fd
#undef qlen

/*******************************************
bind(fd, addr, len)              Function 92
int fd;
struct sockaddr_*in addr;
int length;		Unused for now
********************************************/
#define fd   (int16_t)udata.u_argn
#define uaddr (struct sockaddr_in *)udata.u_argn1

arg_t _bind(void)
{
	struct socket *s = sock_get(fd, NULL);
	struct sockaddr_in sin;
	if (s == NULL)
		return -1;
	if (s->s_state != SS_UNCONNECTED)
		return -1;
	if (sa_getlocal(uaddr, &sin) == -1)
		return -1;
	if (sock_find_local(sin.sin_addr.s_addr, sin.sin_port)) {
		udata.u_error = EADDRINUSE;
		return -1;
	}
	s->s_addr[SADDR_SRC].addr = sin.sin_addr.s_addr;
	s->s_addr[SADDR_SRC].port = sin.sin_port;

	return net_bind(s);
}

#undef fd
#undef uaddr

/*******************************************
connect(fd, addr, len)           Function 93
int fd;
struct sockaddr_*in addr;
int length;		Unused for now
********************************************/
#define fd   (int16_t)udata.u_argn
#define uaddr (struct sockaddr_in *)udata.u_argn1

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
		if (sa_getremote(uaddr, &sin) == -1)
			return -1;
		s->s_addr[SADDR_DST].addr = sin.sin_addr.s_addr;
		s->s_addr[SADDR_DST].port = sin.sin_port;
		if (net_connect(s))
			return -1;
		if (sock_wait_leave(s, 0, SS_CONNECTING)) {
			/* API oddity, thanks Berkeley */
			if (udata.u_error == EAGAIN)
				udata.u_error = EINPROGRESS;
			return -1;
		}
		return sock_error(s);
	}
	udata.u_error = EINVAL;
	return -1;
}

#undef fd
#undef uaddr

/*******************************************
accept(fd, addr)                 Function 94
int fd;
struct sockaddr_*in addr;
********************************************/
#define fd   (int16_t)udata.u_argn

/* Note: We don't do address return, the library can handle it */
arg_t _accept(void)
{
	uint8_t flag;
	struct socket *s = sock_get(fd, &flag);
	struct socket *n;
	int8_t nfd;

	if (s == NULL)
		return -1;
	if (s->s_state == SS_LISTENING) {
		udata.u_error = EALREADY;
		return -1;
	}

	/* Needs locking versus interrupts */
	while ((n = sock_pending(s)) == NULL) {
		if (psleep_flags(s, flag))
			return -1;
		if (s->s_error)
			return sock_error(s);
	}
	if ((nfd = make_socket(&socktypes[SOCKTYPE_TCP], &n)) == -1)
		return -1;
	n->s_state = SS_CONNECTED;
	return nfd;
}

#undef fd

/*******************************************
getsockaddrs(fd, type, addr)     Function 95
int fd;
int type;
struct sockaddr_*in addr;
********************************************/
#define fd   (int16_t)udata.u_argn
#define type (uint16_t)udata.u_argn1
#define uaddr (struct sockaddr_in *)udata.u_argn1

arg_t _getsockaddrs(void)
{
	struct socket *s = sock_get(fd, NULL);

	if (s == NULL)
		return -1;
	if (type > 1) {
		udata.u_error = EINVAL;
		return -1;
	}
	return sa_put(s, type, uaddr);
}

#undef fd
#undef type
#undef uaddr

/* FIXME: do we need the extra arg/flags or can we fake it in user */

/*******************************************
sendto(fd, buf, len, addr)       Function 96
int fd;
char *buf;
susize_t len;
struct sockio *addr;	control buffer
********************************************/
#define fd (int16_t)udata.u_argn
#define buf (char *)udata.u_argn1
#define nbytes (susize_t)udata.u_argn2
#define uaddr ((struct sockio *)udata.u_argn3)

arg_t _sendto(void)
{
	regptr struct socket *s = sock_get(fd, NULL);
	struct sockaddr_in sin;
	uint16_t flags;
	uint16_t alen;
	uint16_t err;

	if (s == NULL)
		return -1;

	if (s->s_state == SS_UNCONNECTED) {
		err = sock_autobind(s);
		if (err)
			return err;
	}
	if (s->s_state < SS_BOUND) {
		udata.u_error = EINVAL;
		return -1;
	}
	if (s->s_state != SS_BOUND && s->s_state < SS_CONNECTED) {
		udata.u_error = ENOTCONN;
		return -1;
	}
	flags = ugetw(&uaddr->sio_flags);
	if (flags) {
		udata.u_error = EINVAL;
		return -1;
	}
	alen = ugetw(&uaddr->sio_flags);
	/* Save the address and then just do a 'write' */
	if (s->s_type != SOCKTYPE_TCP && alen) {
		if (s->s_state >= SS_CONNECTING) {
			udata.u_error = EISCONN;
			return -1;
		}
		/* Use the address in atmp */
		s->s_flag |= SFLAG_ATMP;
		if (sa_getremote(&uaddr->sio_addr, &sin) == -1)
			return -1;
		s->s_addr[SADDR_TMP].addr = sin.sin_addr.s_addr;
		s->s_addr[SADDR_TMP].port = sin.sin_port;
	} else {
		s->s_flag &= ~SFLAG_ATMP;
		if (s->s_state < SS_CONNECTED) {
			udata.u_error = EDESTADDRREQ;
			return -1;
		}
	}
	return _write();
}

#undef fd
#undef buf
#undef nbytes
#undef uaddr

/*******************************************
recvfrom(fd, buf, len, addr)     Function 97
int fd;
char *buf;
susize_t len;
struct sockio *addr;	 control buffer
********************************************/
#define fd (int16_t)udata.u_argn
#define buf (char *)udata.u_argn1
#define nbytes (susize_t)udata.u_argn2
#define uaddr ((struct sockio *)udata.u_argn3)

arg_t _recvfrom(void)
{
	struct socket *s = sock_get(fd, NULL);
	int ret;
	uint16_t flags;

	/* FIXME: will need _read redone for banked syscalls */
	if (s == NULL)
		return -1;

	flags = ugetw(&uaddr->sio_flags);
	if (flags) {
		udata.u_error = EINVAL;
		return -1;
	}

	ret = _read();
	if (ret < 0)
		return ret;
	if (sa_put(s, SADDR_TMP, &uaddr->sio_addr))
		return -1;
	return ret;
}

#undef fd
#undef buf
#undef nbytes
#undef uaddr

/*******************************************
shutdown(fd, how)                Function ??
int fd;
int how;
********************************************/
#define fd (int16_t)udata.u_argn
#define how (uint16_t)udata.u_argn1

arg_t _shutdown(void)
{
	struct socket *s = sock_get(fd, NULL);

	if (s == NULL)
		return -1;
	if (how > 2) {
		udata.u_error = EINVAL;
		return -1;
	}
	return net_shutdown(s, how + 1);
}

#undef fd
#undef how


/* FIXME: Move to _discard */

void sock_init(void)
{
	struct socket *s = sockets;
	uint8_t n = 0;
	while (s < sockets + NSOCKET)
		(s++)->s_num = n++;
	netdev_init();
}

#endif
