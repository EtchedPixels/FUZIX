#include <kernel.h>
#include <kdata.h>
#include <netdev.h>
#include <net_native.h>
#include <printf.h>

/* This holds the additional kernel context for the sockets */
static struct sockdata sockdata[NSOCKET];
/* This is the inode of the backing file object */
static inoptr net_ino;

/* Wake all blockers on a socket - used for error cases */
static void wakeup_all(struct socket *s)
{
	wakeup(s);
	wakeup(&s->s_data);
	wakeup(&s->s_iflag);
}

/*
 *	The daemon has sent us a message. Process the message. Note that we
 *	don't have any interrupts in this stack because everything happens
 *	in a process context of some kind
 *
 * Is it worth having a single call take a batch of events ? 
 */
static struct netevent ne;

int netdev_write(void)
{
	struct socket *s;
	struct sockdata *sd;

	/* Grab a message from the service daemon */
	if (net_ino == NULL || udata.u_count != sizeof(ne) || 
		uget(udata.u_base, &ne, sizeof(ne)) == -1 ||
		ne.socket >= NSOCKET) {
		udata.u_error = EINVAL;
		return -1;
	}

	s = sockets + ne.socket;
	sd = sockdata + ne.socket;

	switch (ne.event) {
		/* State change. Wakes up the socket having moved state */
	case NE_NEWSTATE:
		s->s_state = ne.data;
		sd->ret = ne.ret;
		/* A synchronous state change has completed */
		sd->event &= ~NEVW_STATE;
		/* Review select impact for this v wakeup_all */
		wakeup(s);
		break;
		/* Asynchronous state changing event */
	case NE_EVENT:
		s->s_state = ne.data;
		sd->err = ne.ret;
		wakeup_all(s);
		break;
		/* Change an address */
	case NE_SETADDR:
		if (ne.data < 3)
			memcpy(&s->s_addr[ne.data], &ne.info,
			       sizeof(struct sockaddrs));
		break;
		/* Response to creating a socket. Initialize lcn */
	case NE_INIT:
		s->s_state = SS_UNCONNECTED;
		s->s_lcn = ne.data;
		sd->event = 0;
		sd->ret = ne.ret;
		sd->err = 0;
		sd->rbuf = sd->rnext = 0;
		sd->tbuf = sd->tnext = 0;
		wakeup(s);
		break;
		/* Indicator of write room from the network agent */
	case NE_ROOM:
		sd->tbuf = ne.data;
		wakeup(&s->s_iflag);
		break;
		/* Indicator of data from the network agent */
	case NE_DATA:
		sd->rnext = ne.data;	/* More data available */
		memcpy(sd->rlen, &ne.info,
			       sizeof(uint16_t) * NSOCKBUF);
		s->s_iflag |= SI_DATA;
		break;
	default:
		kprintf("netbad %d\n", ne.event);
		udata.u_error = EOPNOTSUPP;
		return -1;
	}
	return udata.u_count;
}

/* When events are pending we simply hand all the structs to the server
   as copies. It can then make any decisions it needs to make */
static int netdev_report(struct sockdata *sd)
{
	uint8_t sn = sd - sockdata;
	struct socket *s = sockets + sn;

	if (uput(sd, udata.u_base, sizeof(*sd)) == -1 ||
		uput(s, udata.u_base + sizeof(*sd), sizeof(*s)) == -1)
		return -1;
	sd->event &= ~NEV_MASK;
	return udata.u_count;
}

/*
 *	Scan the socket table for any socket with a pending event. Shovel
 *	the first one we find at the daemon. We should possibly round-robin
 *	these but it's not clear it's that important
 */
int netdev_read(uint8_t flag)
{
	if (net_ino == NULL || udata.u_count != sizeof(struct sockmsg)) {
		udata.u_error = EINVAL;
		return -1;
	}
	while(1) {
		struct sockdata *sd = sockdata;
		while (sd != sockdata + NSOCKET) {
			if (sd->event)
				return netdev_report(sd);
			sd++;
		}
		if (psleep_flags(&ne, flag))
			return -1;
	}
}		

/*
 *	The ioctl interface at the moment is simply the initialization
 *	function.
 */
static int netdev_ioctl(uarg_t request, char *data)
{
	int16_t fd;

	switch(request) {
		/* Daemon starting up, passing file handle of cache */
		/* FIXME: Check sizes etc are valid via some kind of
		   passed magic hash */
		case NET_INIT:
			if (net_ino) {
				udata.u_error = EBUSY;
				return -1;
			}
			fd = ugetw(data);
			if ((net_ino = getinode(fd)) == NULLINODE)
				return -1;
			i_ref(net_ino);
			return 0;				
	}
	udata.u_error = ENOTTY;
	return -1;
}

/*
 *	On a close of the daemon close down all the sockets we
 *	have opened.
 */
static int netdev_close(void)
{
	struct socket *s = sockets;
	if (net_ino) {
		i_deref(net_ino);
		while (s < sockets + NSOCKET) {
			if (s->s_state != SS_UNUSED) {
				s->s_state = SS_CLOSED;
				wakeup_all(s);
			}
			s++;
		}
	}
	return 0;
}

/*
 *	We have received an event from userspace that requires us to wait
 *	until the network stack performs the relevant state change. Pass
 *	the wanted new state on to the daemon, then wait until our STATEW
 *	flag is cleared by a suitable message.
 */
static int netn_synchronous_event(struct socket *s, uint8_t state)
{
	uint8_t sn = s - sockets;
	struct sockdata *sd = &sockdata[sn];

	sd->event |= NEV_STATE | NEVW_STATE;
	sd->newstate = state;
	wakeup(&ne);

	do {
		psleep(s);
	} while (sd->event & NEVW_STATE);

	udata.u_error = sd->ret;
	return -1;
}

/*
 *	Flag an unsolicited event to the daemon. These are used to
 *	handshake the buffer status.
 */
static void netn_asynchronous_event(struct socket *s, uint8_t event)
{
	uint8_t sn = s - sockets;
	struct sockdata *sd = &sockdata[sn];
	sd->event |= event;
	wakeup(&ne);
}

/*
 *	Queue data to a stream socket. We use the entire buffer space
 *	available as a ring buffer and write bytes to it. We then update
 *	our pointer and poke the daemon to send stuff.
 */
static uint16_t netn_queuebytes(struct socket *s)
{
	arg_t n = udata.u_count;
	arg_t r = 0;
	uint8_t sn = s - sockets;
	struct sockdata *sd = &sockdata[sn];
	uint16_t spc;

	/* Do we have room ? */
	if (sd->tnext == sd->tbuf)
		return 0;

	udata.u_sysio = false;
	udata.u_offset = sn * SOCKBUFOFF + RXBUFOFF + sd->tnext;

	/* Wrapped part of the ring buffer */
	if (n && sd->tnext > sd->tbuf) {
		/* Write into the end space */
		spc = TXBUFSIZ - sd->tnext;
		if (spc < n)
			spc = n;
		udata.u_count = spc;
		/* FIXME: check writei returns and readi returns properly */
		writei(net_ino, 0);
		if (udata.u_error)
			return 0xFFFF;
		sd->tnext += spc;
		n -= spc;
		r = spc;
		/* And wrap */
		if (sd->tnext == TXBUFSIZ)
			sd->tnext = 0;
	}
	/* If we are not wrapped or just did the overflow write lower */
	if (n) {
		spc = sd->tbuf - sd->tnext;
		if (spc < n)
			spc = n;
		udata.u_count = spc;
		udata.u_offset = sn * SOCKBUFOFF + RXBUFOFF + sd->tnext;

		/* FIXME: check writei returns and readi returns properly */
		writei(net_ino, 0);
		if (udata.u_error)
			return 0xFFFF;
		sd->tnext += spc;
		r += spc;
	}
	/* Tell the networkd daemon there is more data in the ring */
	netn_asynchronous_event(s, NEV_WRITE);
	return r;
}

/*
 *	Queue data to a datagram socket. At the moment we use the ring
 *	as a set of fixed sized buffers. That may want changing. We do
 *	however need to work out how to pass an address and size header
 *	in the buffers, while still getting the ring behaviour right if
 *	we changed this as well as avoiding partial writes of a datagram.
 *
 * FIXME: we need to attach an address to getbuf/putbuf cases because we
 * may be using sendto/recvfrom
 */
static uint16_t netn_putbuf(struct socket *s)
{
	uint8_t sn = s - sockets;
	struct sockdata *sd = &sockdata[sn];

	if (udata.u_count > TXPKTSIZ) {
		udata.u_error = EMSGSIZE;
		return 0xFFFF;
	}
	if (sd->tnext == sd->tbuf)
		return 0;

	udata.u_sysio = false;
	udata.u_offset = sn * SOCKBUFOFF + RXBUFOFF + sd->tbuf * TXPKTSIZ;
	/* FIXME: check writei returns and readi returns properly */
	writei(net_ino, 0);
	sd->tlen[sd->tnext++] = udata.u_count;
	if (sd->tnext == NSOCKBUF)
		sd->tnext = 0;
	/* Tell the network stack there is another buffer to consume */
	netn_asynchronous_event(s, NEV_WRITE);
	return udata.u_count;
}

/*
 *	Pull a packet from the receive buffer. We fetch the next ring buffer
 *	slot and then copy as much as is required into the user buffer. This
 *	side also needs to handle addressing better, and may make sense to
 *	use the ring buffer packing. Once done we poke the daemon so it knows
 *	space is freed.
 */
static uint16_t netn_getbuf(struct socket *s)
{
	uint8_t sn = s - sockets;
	struct sockdata *sd = &sockdata[sn];
	arg_t n = udata.u_count;
	arg_t r = 0;
	
	if (sd->rbuf == sd->rnext)
		return 0;
	udata.u_sysio = false;
	udata.u_offset = sn * SOCKBUFOFF + sd->rbuf * RXPKTSIZ;
	udata.u_count = min(udata.u_count, sd->rlen[sd->rbuf++]);
	/* FIXME: check writei returns and readi returns properly */
	readi(net_ino, 0);
	/* FIXME: be smarter when we send this */
	if (sd->rbuf == NSOCKBUF)
		sd->rbuf = 0;
	netn_asynchronous_event(s, NEV_READ);
	return udata.u_count;
}

/*
 *	Pull bytes from the receive ring buffer. We copy as many bytes as
 *	we can to fulfill the user request. Short reads are acceptable if
 *	the buffer contains some data but not enough.
 *	After reading we tell the daemon and it will adjust the TCP window
 *	and send an ack frame as appropriate as well as adjusting its
 *	copy of the ring state
 */
static uint16_t netn_copyout(struct socket *s)
{
	arg_t n = udata.u_count;
	arg_t r = 0;
	uint8_t sn = s - sockets;
	struct sockdata *sd = &sockdata[sn];
	uint16_t spc;

	if (sd->rnext == sd->rbuf)
		return 0;

	udata.u_sysio = false;
	udata.u_offset = sn * SOCKBUFOFF + sd->rbuf;

	/* Wrapped part of the ring buffer */
	if (n && sd->rnext < sd->rbuf) {
		/* Write into the end space */
		spc = RXBUFSIZ - sd->rbuf;
		if (spc < n)
			spc = n;
		udata.u_count = spc;
		/* FIXME: check writei returns and readi returns properly */
		readi(net_ino, 0);
		if (udata.u_error)
			return 0xFFFF;
		sd->rbuf += spc;
		n -= spc;
		r = spc;
		/* And wrap */
		if (sd->rbuf == RXBUFSIZ)
			sd->rbuf = 0;
	}
	/* If we are not wrapped or just did the overflow write lower */
	if (n) {
		spc = sd->rnext - sd->rbuf;
		if (spc < n)
			spc = n;
		udata.u_count = spc;
		/* FIXME: check writei returns and readi returns properly */
		readi(net_ino, 0);
		if (udata.u_error)
			return 0xFFFF;
		sd->rbuf += spc;
		r += spc;
	}
	/* Tell the networkd daemon there is more room in the ring */
	/* FIXME: be smarter when we send this */
	netn_asynchronous_event(s, NEV_READ);
	return r;
	
}


/*
 *	Called from the core network layer when a socket is being
 *	allocated. We can either move the socket to SS_UNCONNECTED,
 *	or error. In our case the daemon will reply with an NE_INIT,
 *	or a state change to set an error.
 *
 *	This call is blocking but the BSD socket API users don't expect
 *	anything to block for long. Blocking here is however needed because
 *	some of the stacks (this one included) are asynchronous to the
 *	OS.
 */
int net_init(struct socket *s)
{
	if (!net_ino) {
		udata.u_error = ENETDOWN;
		return -1;
	}
	return netn_synchronous_event(s, SS_UNCONNECTED);
}

/*
 *	A bind has occurred. This might be a user triggering a bind but it
 *	could also be an autobind.
 *
 *	FIXME: distinguish bind and autobind so we can push address picking
 *	into the stack implementation to cover non IP stacks
 */
int net_bind(struct socket *s)
{
	return netn_synchronous_event(s, SS_BOUND);
}

/*
 *	A listen has been issued by the user. Inform the underlying TCP
 *	stack that it should accept connections on this socket. A stack that
 *	lacks incoming connection support can error instead
 */
int net_listen(struct socket *s)
{
	return netn_synchronous_event(s, SS_LISTENING);
}

/*
 *	A connect has been issued by the user. This message tells the
 *	stack to begin connecting. It should put the socket state into
 *	SS_CONNECTING before returning, or it can error.
 */
int net_connect(struct socket *s)
{
	return netn_synchronous_event(s, SS_CONNECTING);
}

/*
 *	A socket is being closed by the user. Move the socket into a
 *	closed state and free the resources used. If the underlying
 *	implementation has longer lived resources (eg a TCP port moving
 *	into TIME_WAIT) then the socket and internal resources must be
 *	disconnected from one another.
 */
void net_close(struct socket *s)
{
	/* Caution here - the native tcp socket will hang around longer */
	netn_synchronous_event(s, SS_CLOSED);
}

/*
 *	Read or recvfrom a socket. We don't yet handle message addresses
 *	sensible and that needs fixing
 */
arg_t net_read(struct socket *s, uint8_t flag)
{
	uint16_t n = 0;
	uint8_t sn = s - sockets;
	struct sockdata *sd = &sockdata[sn];

	if (sd->err) {
		udata.u_error = sd->err;
		sd->err = 0;
		return -1;
	}
	while (1) {
		if (s->s_state < SS_CONNECTED) {
			udata.u_error = EINVAL;
			return -1;
		}

		if (s->s_type != SOCKTYPE_TCP)
			n = netn_getbuf(s);
		else
			n = netn_copyout(s);
		if (n == 0xFFFF)
			return -1;
		if (n)
			return n;
		s->s_iflag &= ~SI_DATA;
		/* Could do with using timeouts here to be clever for non O_NDELAY so
		   we aggregate data. For now assume a fifo */
		if (psleep_flags(&s->s_iflag, flag))
			return -1;
	}
}

/*
 *	Write or sendto a socket. We don't yet handle message addresses
 *	sensible and that needs fixing
 */
arg_t net_write(struct socket * s, uint8_t flag)
{
	uint16_t n = 0, t = 0;
	uint8_t sn = s - sockets;
	struct sockdata *sd = &sockdata[sn];

	if (sd->err) {
		udata.u_error = sd->err;
		sd->err = 0;
		return -1;
	}

	while (t < udata.u_count) {
		if (s->s_state == SS_CLOSED) {
			udata.u_error = EPIPE;
			ssig(udata.u_ptab, SIGPIPE);
			return -1;
		}
		if (s->s_type != SOCKTYPE_TCP)
			n = netn_putbuf(s);
		else
			n = netn_queuebytes(s);
		/* FIXME: buffer the error in this case */
		if (n == 0xFFFFU)
			return udata.u_count ? (arg_t)udata.u_count : -1;

		t += n;

		if (n == 0) {	/* Blocked */
			netn_asynchronous_event(s, NE_ROOM);
			if (psleep_flags(&s->s_iflag, flag))
				return -1;
		}
	}
	return udata.u_count;
}

/* Gunk we are still making up */
struct netdevice net_dev = {
	0,
	"net0",
	IFF_POINTOPOINT
};

arg_t net_ioctl(uint8_t op, void *p)
{
	used(op);
	used(p);
	return -EINVAL;
}

void netdev_init(void)
{
}
