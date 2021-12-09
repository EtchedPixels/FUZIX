#include <kernel.h>
#include <kdata.h>
#include <netdev.h>
#include <net_native.h>
#include <printf.h>

#ifdef CONFIG_NET_NATIVE

/* For now until we work out where this really belongs */
uint8_t sock_wake[NSOCKET]; // TODO: what is the interaction on this?

/*
 *	TODO: support using a malloc pool of out of bank (or flat space)
 *	memory buffers for networking not just file buffers. That way
 *	it's much nicer on big boxes.
 */

/* This holds the additional kernel context for the sockets */
static struct sockdata sockdata[NSOCKET];
/* This is the inode of the backing file object */
static inoptr net_ino;

/* Wake all blockers on a socket - used for error cases */
static void wakeup_all(struct socket *s)
{
	wakeup(s);
	wakeup(&s->s_iflags);
	selwake_dev(4, 65, SELECT_IN);
}

/*
 *	The daemon has sent us a message. Process the message. Note that we
 *	don't have any interrupts in this stack because everything happens
 *	in a process context of some kind
 *
 * Is it worth having a single call take a batch of events ? 
 */
static struct netevent ne;

int netdev_write(uint8_t flags)
{
	regptr struct socket *s;
	regptr struct sockdata *sd;

	used(flags);
	/* Grab a message from the service daemon */
	if (net_ino == NULL || udata.u_count != sizeof(ne) ||
		uget(udata.u_base, &ne, sizeof(ne)) == -1 ||
		ne.socket >= NSOCKET) {
		udata.u_error = EINVAL;
		return -1;
	}

	s = sockets + ne.socket;
	sd = sockdata + s->s_num;
	switch (ne.event) {
		/* Initialize a new socket */
	case NE_INIT:
		sd->lcn = ne.data;
		ne.data = SS_UNCONNECTED;
		/* And fall through */
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
		if (ne.ret)
			s->s_error = ne.ret;
		wakeup_all(s);
		break;
		/* Change an address */
	case NE_SETADDR:
		switch (ne.data) {
		case NE_SETADDR_SRC:
			memcpy(&s->src_addr, &ne.info,
				sizeof(struct ksockaddr));
			break;
		case NE_SETADDR_DST:
			memcpy(&s->dst_addr, &ne.info,
				sizeof(struct ksockaddr));
			break;
		default:
			;
		}
		break;
		/* Indicator of write room from the network agent */
	case NE_ROOM:
		sd->tbuf = ne.data;
		wakeup(&s->s_iflags);
		break;
		/* Indicator of data from the network agent */
	case NE_DATA:
		sd->rnext = ne.data;	/* More data available */
		memcpy(sd->rlen, &ne.info,
			       sizeof(uint16_t) * NSOCKBUF);
		s->s_iflags |= SI_DATA;
		wakeup(&s->s_iflags);
		break;
		/* Remote reset */
	case NE_RESET:
		s->s_iflags |= SI_SHUTW;
		s->s_state = SS_CLOSED;
		/* Remote closed connection */
	case NE_SHUTR:
		s->s_iflags |= SI_SHUTR;
		if (ne.ret)
			s->s_error = ne.ret;
		wakeup_all(s);
		break;
	case NE_UNHOOK:
		if (s->s_state == SS_DEAD){
			sd->event = 0;
			s->s_state = SS_UNUSED;
			/* FIXME: You re-use something you pays the price.
			   Probably should switch to using data 0 as devices do?
			 */
			s->s_ino->c_node.i_nlink = 0;
			s->s_ino->c_flags |= CDIRTY;
		}
		else
			kputs("bad unhook (in use)\n");
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
	struct socket *s = sd->socket;

	if (uput(s, udata.u_base, sizeof(*s)) == -1 ||
		uput(sd, udata.u_base + sizeof(*s), sizeof(*sd)) == -1)
		return -1;
	sd->event &= ~NEV_MASK;
	return udata.u_count;
}

/*
 *	Scan the socket table for any socket with a pending event. Shovel
 *	the first one we find at the daemon. We should possibly round-robin
 *	these but it's not clear it's that important
 */
static struct sockdata *netdev_findevent(void)
{
	struct sockdata *sd = sockdata;
	while (sd != sockdata + NSOCKET) {
		if (sd->event)
			return sd;
		sd++;
	}
	return NULL;
}

int netdev_read(uint8_t flag)
{
	if (net_ino == NULL || udata.u_count != sizeof(struct sockmsg)) {
		udata.u_error = EINVAL;
		return -1;
	}
	while(1) {
		struct sockdata *sd = netdev_findevent();
		if (sd)
			return netdev_report(sd);
		if (psleep_flags(&ne, flag))
			return -1;
	}
}		

/*
 *	The ioctl interface at the moment is simply the initialization
 *	function.
 */
int netdev_ioctl(uarg_t request, char *data)
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
			(void)(i_ref(net_ino));
			return 0;
#ifdef CONFIG_LEVEL_2
		case SELECT_BEGIN:
		case SELECT_TEST:
			/* We are always writable, but may not alway be readable */
			if (*data & SELECT_IN) {
				if (netdev_findevent() == NULL)
					*data &= ~SELECT_IN;
			}
			*data &= SELECT_IN|SELECT_OUT;
#endif
	}
	udata.u_error = ENOTTY;
	return -1;
}

/*
 *	On a close of the daemon close down all the sockets we
 *	have opened.
 */
int netdev_close(uint8_t minor)
{
	regptr struct socket *s = sockets;
	used(minor);
	if (net_ino) {
		i_deref(net_ino);
		net_ino = NULL;
		while (s < sockets + NSOCKET) {
			if (s->s_state != SS_UNUSED) {
				s->s_state = SS_CLOSED;
				s->s_iflags |= SI_SHUTR|SI_SHUTW;
				s->s_error = ENETDOWN;
				wakeup_all(s);
			}
			s++;
		}
	}
	return 0;
}

void netdev_init(void)
{
}

/*
 *	We have received an event from userspace that requires us to wait
 *	until the network stack performs the relevant state change. Pass
 *	the wanted new state on to the daemon, then wait until our STATEW
 *	flag is cleared by a suitable message.
 */
static int netn_synchronous_event(struct socket *s, uint8_t state)
{
	struct sockdata *sd = sockdata + s->s_num;

	sd->event |= NEV_STATE | NEVW_STATE;
	sd->newstate = state;
	wakeup(&ne);
	selwake_dev(4, 65, SELECT_IN);

	do {
	    if( s->s_state == SS_CLOSED || s->s_state == SS_DEAD)
		return -1;
	    psleep_nosig(s);
	} while (sd->event & NEVW_STATE);

	udata.u_error = sd->ret;
	return 0;
}

/*
 *	Flag an unsolicited event to the daemon. These are used to
 *	handshake the buffer status.
 */
static void netn_asynchronous_event(struct socket *s, uint8_t event)
{
	struct sockdata *sd = sockdata + s->s_num;
	sd->event |= event;
	wakeup(&ne);
	selwake_dev(4, 65, SELECT_IN);
}

/* General purpose ring buffer operator. Non re-entrant so use every
   trick of the trade to generate non-shite Z80 code, especially given the
   fact we have a 32bit offset and SDCC. This game saves us 768 bytes over
   a naïve implementation.
   Note: for adding to buffer the meaning of sptr and eptr are switched

*/

static uint16_t sptr, eptr, len;
static uint32_t base;
static void (*op)(inoptr, uint_fast8_t);

static uint16_t ringbop(void)
{
	uarg_t spc;
	uarg_t r = 0;

	udata.u_sysio = false;
	udata.u_error = 0;
	/* Wrapped part of the ring buffer */
	if (len && sptr >= eptr) {
		/* Write into the end space */
		spc = RINGSIZ - sptr;
		if (len < spc)
			spc = len;
		udata.u_count = spc;
		udata.u_offset = base + sptr;
		op(net_ino, 0);
		if (udata.u_error)
			return 0xFFFF;
		sptr += spc;
		len -= spc;
		r = spc;
		/* And wrap */
		if (sptr == RINGSIZ)
			sptr = 0;
	}
	/* If we are not wrapped or just did the overflow write lower */
	if (len) {
		spc = eptr - sptr;
		if( len < spc )
			spc = len;
		udata.u_count = spc;
		udata.u_offset = base + sptr;
		op(net_ino, 0);
		if (udata.u_error)
			return 0xFFFF;
		sptr += spc;
		r += spc;
	}
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
uint16_t netn_putbuf(struct socket *s)
{
	struct sockdata *sd = sockdata + s->s_num;

	if (udata.u_count > TXPKTSIZ) {
		udata.u_error = EMSGSIZE;
		return 0xFFFF;
	}
	/* check for fullness */
	/*  FIXME: giving up 1 of 4 udp buffers to do this check sucks */
	if( ((sd->tnext+1) & (NSOCKBUF - 1)) == sd->tbuf )
	    return 0;

	udata.u_sysio = false;
	udata.u_offset = s->s_num * SOCKBUFOFF + RXBUFOFF + sd->tbuf * TXPKTSIZ;
	/* FIXME: check writei returns and readi returns properly */
	writei(net_ino, 0);
	sd->tlen[sd->tnext++] = udata.u_done;
	if (sd->tnext == NSOCKBUF)
		sd->tnext = 0;
	/* Tell the network stack there is another buffer to consume */
	netn_asynchronous_event(s, NEV_WRITE);
	return udata.u_done;
}

/*
 *	Pull a packet from the receive buffer. We fetch the next ring buffer
 *	slot and then copy as much as is required into the user buffer. This
 *	side also needs to handle addressing better, and may make sense to
 *	use the ring buffer packing. Once done we poke the daemon so it knows
 *	space is freed.
 */
uint16_t netn_getbuf(struct socket *s)
{
	struct sockdata *sd = sockdata + s->s_num;

	if (sd->rbuf == sd->rnext)
		return 0;
	udata.u_sysio = false;
	udata.u_offset = s->s_num * SOCKBUFOFF + sd->rbuf * RXPKTSIZ;
	udata.u_count = min(udata.u_count, sd->rlen[sd->rbuf]);
	/* FIXME: check writei returns and readi returns properly */
	readi(net_ino, 0);
	/* FIXME: be smarter when we send this */
	if (++sd->rbuf == NSOCKBUF)
		sd->rbuf = 0;
	netn_asynchronous_event(s, NEV_READ);
	return udata.u_done;
}

/*
 *	Queue data to a stream socket. We use the entire buffer space
 *	available as a ring buffer and write bytes to it. We then update
 *	our pointer and poke the daemon to send stuff.
 */
uint16_t netn_queuebytes(struct socket *s)
{
	arg_t r = 0U;
	struct sockdata *sd = sockdata + s->s_num;
	uint16_t c;

	len = udata.u_count;

	sptr = sd->tnext;
	eptr = sd->tbuf;
	/* check for fullness */
	if( ((sptr + 1) & (RINGSIZ - 1)) == eptr )
	    return 0;
	/* don't put more than 1 less ring size */
	if( sptr >= eptr )
	    c = RINGSIZ - ( sptr - eptr );
	else
	    c = eptr - sptr;
	c -= 1;
	if( len > c ) len = c;

	base = s->s_num * SOCKBUFOFF + RXBUFOFF;
	op = writei;
	r = ringbop();
	sd->tnext = sptr;

	/* Tell the network daemon there is more data in the ring */
	if (r != 0xFFFF)
		netn_asynchronous_event(s, NEV_WRITE);
	return r;
}


/*
 *	Pull bytes from the receive ring buffer. We copy as many bytes as
 *	we can to fulfill the user request. Short reads are acceptable if
 *	the buffer contains some data but not enough.
 *	After reading we tell the daemon and it will adjust the TCP window
 *	and send an ack frame as appropriate as well as adjusting its
 *	copy of the ring state
 */
uint16_t netn_copyout(struct socket *s)
{
	arg_t r = 0U;
	struct sockdata *sd = sockdata + s->s_num;

	len = udata.u_count;

	sptr = sd->rbuf;
	eptr = sd->rnext;
	if( sptr == eptr )
	    return 0;
	base = s->s_num * SOCKBUFOFF;
	op = readi;
	r = ringbop();
	sd->rbuf = sptr;
	if (r != 0xFFFF)
		netn_asynchronous_event(s, NEV_READ);
	return r;
}

#if 0 // TODO: remove
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
	struct sockdata *sd = sockdata + s->s_num;
	if (!net_ino) {
		udata.u_error = ENETDOWN;
		return -1;
	}
	sd->socket = s;
	sd->event = 0;
	sd->rbuf = sd->rnext = 0;
	sd->tbuf = sd->tnext = 0;
	return netn_synchronous_event(s, SS_UNCONNECTED);
}
#endif

#if 0 // TODO: remove
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
#endif

#if 0 // TODO: remove
/*
 *	A listen has been issued by the user. Inform the underlying TCP
 *	stack that it should accept connections on this socket. A stack that
 *	lacks incoming connection support can error instead
 */
int net_listen(struct socket *s)
{
	return netn_synchronous_event(s, SS_LISTENING);
}
#endif

#if 0 // TODO: remove
/*
 *	A connect has been issued by the user. This message tells the
 *	stack to begin connecting. It should put the socket state into
 *	SS_CONNECTING before returning, or it can error.
 */
int net_connect(struct socket *s)
{
	return netn_synchronous_event(s, SS_CONNECTING);
}
#endif

#if 0 // TODO: colides with in-kernel network.c
/*
 *	A socket is being closed by the user. Move the socket into a
 *	closed state and free the resources used. If the underlying
 *	implementation has longer lived resources (eg a TCP port moving
 *	into TIME_WAIT) then the socket and internal resources must be
 *	disconnected from one another.
 *
 *	Fuzix close() methods are not permitted to block.
 */
void net_close(struct socket *s)
{
	struct sockdata *sd = sockdata + s->s_num;
	/* Caution here - the native tcp socket will hang around longer */
	sd->newstate = SS_DEAD;
	netn_asynchronous_event(s, NEV_STATE|NEVW_STATE);
	/* The stack will see the closed state and then we will
	   progress to SS_DEAD. When the stack is finished with us it
	   will send an UNHOOK message which will do the final resource
	   clean up and allow the socket to be reused */
}
#endif

#if 0 // TODO: colides with in-kernel network.c
/*
 *	Read or recvfrom a socket. We don't yet handle message addresses
 *	sensibly and that needs fixing
 */
arg_t net_read(regptr struct socket *s, uint8_t flag)
{
	uint16_t n = 0;
	struct sockdata *sd = sockdata + s->s_num;

	while (1) {
		/* Partial I/O saves the error for the rext call but
		   returns */
		if (s->s_error) {
			if (n == 0)
				return sock_error(s);
			else
				return n;
		}
		/* FIXME: We should be forced into CLOSED state so is this
		   check actually needed */
	        if (net_ino == NULL) {
			udata.u_error = EPIPE;
			ssig(udata.u_ptab, SIGPIPE);
			return -1;
		}
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
		/* If the socket is moved to closed SI_SHUTR must be set. We
		   don't report an error for a read on a closed socket, we
		   report 0 (EOF) */
		if (n || (s->s_iflags & SI_SHUTR))
		    return n;

		s->s_iflags &= ~SI_DATA;
		/* Could do with using timeouts here to be clever for non O_NDELAY so
		   we aggregate data. For now assume a fifo */
		if (psleep_flags(&s->s_iflags, flag))
			return -1;
	}
}
#endif

#if 0 // TODO: colides with in-kernel network.c
/*
 *	Write or sendto a socket. We don't yet handle message addresses
 *	sensibly and that needs fixing
 */
arg_t net_write(regptr struct socket * s, uint8_t flag)
{
	uint16_t n = 0, t = 0;
	uint16_t l = udata.u_count;
	struct sockdata *sd = sockdata + s->s_num;

	if (sock_error(s))
		return -1;

	while (t < l) {
		udata.u_count = l - t;
		if (s->s_state == SS_CLOSED || (s->s_iflags & SI_SHUTW)) {
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
			return l ? (arg_t)l : -1;

		if (s->s_error){
			if (l == 0)
				return sock_error(s);
			return l;
		}

		t += n;

		if (n == 0) {	/* Blocked */
			if (psleep_flags(&s->s_iflags, flag))
				return -1;
		}
	}
	return l;
}
#endif

#if 0 // TODO: remove
arg_t net_shutdown(struct socket *s, uint8_t flag)
{
	s->s_iflags |= flag;
	wakeup_all(s);
	return 0;
}
#endif

int netproto_socket(void)
{
  // TODO: implement
  return 0;
}

int netproto_find_local(struct ksockaddr *addr)
{
  // TODO: implement
  return -1;
}

int netproto_autobind(struct socket *s)
{
  // TODO: implement
  return 0;
}

int netproto_bind(struct socket *s)
{
  // TODO: implement
  return 0;
}

int netproto_listen(struct socket *s)
{
  // TODO: implement
  s->s_state = SS_LISTENING;
  return 0;
}

int netproto_begin_connect(struct socket *s)
{
  // TODO: implement
  return 0;
}

int netproto_accept_complete(struct socket *s)
{
  // TODO: implement
  return 0;
}

int netproto_read(struct socket *s)
{
  // TODO: implement
  return 0;
}

arg_t netproto_write(struct socket *s, struct ksockaddr *addr)
{
  // TODO: implement
  return 0;
}

struct socket *netproto_sockpending(struct socket *s)
{
  // TODO: implement
  return NULL;
}

void netproto_setup(struct socket *s)
{
  // TODO: implement
}

void netproto_free(struct socket *s)
{
  // TODO: implement
}

int netproto_close(struct socket *s)
{
  // TODO: implement
  return 0;
}

arg_t netproto_shutdown(struct socket *s, uint8_t how)
{
  // TODO: implement
  return 0;
}

arg_t netproto_ioctl(struct socket *s, int op, char *ifr_u /* in user space */)
{
  static struct ifreq ifr;

  if (uget(ifr_u, &ifr, sizeof(struct ifreq)))
    return -1;
  if (op != SIOCGIFNAME && memcmp(ifr.ifr_name, "eth0", 5U)) {
    udata.u_error = ENODEV;
    return -1;
  }
  switch (op) {
  /* Get side */
  case SIOCGIFNAME:
    if (ifr.ifr_ifindex) {
      udata.u_error = ENODEV;
      return -1;
    }
    memcpy(ifr.ifr_name, "eth0", 5U);
    goto copyback;
  case SIOCGIFINDEX:
    ifr.ifr_ifindex = 0; // TODO: is that so?
    goto copyback;
  case SIOCGIFFLAGS:
    // TODO: implement: ifr.ifr_flags =
    goto copyback;
  case SIOCGIFADDR:
    // TODO: implement: ifr.ifr_addr.sa.sin.sin_addr.s_addr =
    goto copy_addr;
  case SIOCGIFBRDADDR:
    // TODO: implement: ifr.ifr_broadaddr.sa.sin.sin_addr.s_addr =
    goto copy_addr;
  case SIOCGIFGWADDR:
    // TODO: implement: ifr.ifr_gwaddr.sa.sin.sin_addr.s_addr =
    goto copy_addr;
  case SIOCGIFNETMASK:
    // TODO: implement: ifr.ifr_netmask.sa.sin.sin_addr.s_addr =
    goto copy_addr;
  case SIOCGIFHWADDR:
    // TODO: implement: memcpy(ifr.ifr_hwaddr.sa.hw.shw_addr, mac_addr, 6U);
    ifr.ifr_hwaddr.sa.hw.shw_family = HW_ETH;
    goto copyback;
  case SIOCGIFMTU:
    // TODO: implement: ifr.ifr_mtu = 1500;
    goto copyback;
    /* Set side */
  case SIOCSIFFLAGS:
    /* Doesn't really do anything ! */
    // TODO: verify?
    return 0;
  case SIOCSIFADDR:
    // TODO: implement: ipa = ifr.ifr_addr.sa.sin.sin_addr.s_addr;
    break;
  case SIOCSIFGWADDR:
    // TODO: implement: iga = ifr.ifr_gwaddr.sa.sin.sin_addr.s_addr;
    break;
  case SIOCSIFNETMASK:
    // TODO: implement: igm = ifr.ifr_netmask.sa.sin.sin_addr.s_addr;
    break;
  case SIOCSIFHWADDR:
    // TODO: implement: memcpy(mac_addr, ifr.ifr_hwaddr.sa.hw.shw_addr, 6U);
    // TODO: propagate it to a driver even if it is doing nothing with it!
    break;
  default:
    udata.u_error = EINVAL;
    return -1;
  }
  return 0;
copy_addr:
  ifr.ifr_addr.sa.sin.sin_family = AF_INET;
copyback:
  return uput(&ifr, ifr_u, sizeof ifr);
}

#endif
