/*
 *	First draft of an implementation of the low level half of the
 *	network code for a WizNet device such as the 5200.
 *
 *	This isn't working code of any kind but a sketch of it to check
 *	the abstractions we currently have look workable.
 */

/* TODO 
  - server socket fixes (we should allocate one channel to the listening
    socket and when it connects allocate another one. That means we need to
    use to break the 1:1 mapping between wiznet and OS socket numbering
  - handle address collision interrupt
 */

static uint8_t irqmask;

#define RX_MASK 	0x1FFF
#define TX_MASK		0x1FFF

/* Core helpers: platform supplies wiz_bread{_u} and wiz_bwrite{_u} */

/* Might be worth inlining these on some platforms - need to think about
   that carefully */

static uint8_t wiz_readb(uint16_t offset)
{
	uint8_t n;
	wiz_bread(offset, &n, 1);
	return n;
}

static uint16_t wiz_readw(uint16_t offset)
{
	uint16_t n;
	wiz_bread(offset, &n, 2);
	return n;
}

static uint32_t wiz_readl(uint16_t offset)
{
	uint32_t n;
	wiz_bread(offset, &n, 4);
	return n;
}

static void wiz_writeb(uint16_t offset, uint8_t n)
{
	wiz_bwrite(offset, &n, 1);
}

static void wiz_writew(uint16_t offset, uint16_t n)
{
	wiz_bwrite(offset, &n, 2);
}

#ifdef BIG_ENDIAN
#define wiz_writew_n(a,b)	wiz_writew(a,b)
#define wiz_readw_n(a)		wiz_readw(a)
#else

static void wiz_writew_n(uint16_t offset, uint16_t n)
{
	n = htons(n);
	wiz_bwrite(offset, &n, 2);
}

static uint16_t wiz_readw_n(uint16_t offset)
{
	uint16_t n;
	wiz_bread(offset, &n, 2);
	return ntohs(n);
}
#endif


/* FIXME: look for ways to fold these four together */
static void wiz_queue(uint16_t i, uint16_t n, uint8_t * p)
{
	uint16_t dm = wiz_readw(Sn_TX_WR0 + i) & TX_MASK;
	uint16_t tx_base = 0x8000 + (i << 3);	/* i is already << 8 */

	if (dm + n >= TX_MASK) {
		uint16_t us = TX_MASK + 1 - dm;
		wiz_bwrite(dm + tx_base, p, us);
		wiz_bwrite(tx_base, p + us, n - us);
	} else
		wiz_bwrite(dm + tx_base, p, n);
}

static void wiz_queue_u(uint16_t i, uint16_t n, uint8_t * p)
{
	uint16_t dm = wiz_readw_n(Sn_TX_WR0 + i) & TX_MASK;
	uint16_t tx_base = 0x8000 + (i << 3);	/* i is already << 8 */

	if (dm + n >= TX_MASK) {
		uint16_t us = TX_MASK + 1 - dm;
		wiz_bwrite_u(dm + tx_base, p, us);
		wiz_bwrite_u(tx_base, p + us, n - us);
	} else
		wiz_bwrite_u(dm + tx_base, p, n);
}

static void wiz_dequeue(uint16_t i, uint16_t n, uint8_t * p)
{
	uint16_t dm = wiz_readw_n(Sn_RX0 + i) & RX_MASK;
	uint16_t rx_base = 0xC000 + (i << 3);	/* i is already << 8 */

	if (dm + n >= RX_MASK) {
		uint16_t us = RX_MASK + 1 - dm;
		wiz_bread(dm + rx_base, p, us);
		wiz_bread(rx_base, p + us, n - us);
	} else
		wiz_bread(dm + rx_base, p, n);
}

static void wiz_dequeue_u(uint16_t i, uint16_t n, uint8_t * p)
{
	uint16_t dm = wiz_readw_n(Sn_RX0 + i) & RX_MASK;
	uint16_t rx_base = 0xC000 + (i << 3);	/* i is already << 8 */

	if (dm + n >= RX_MASK) {
		uint16_t us = RX_MASK + 1 - dm;
		wiz_bread_u(dm + rx_base, p, us);
		wiz_bread_u(rx_base, p + us, n - us);
	} else
		wiz_bread_u(dm + rx_base, p, n);
}

static void wiz_wakeall(struct socket *s)
{
	wakeup(s);
	wakeup(&s->s_iflag);
	wakeup(&s->s_data);
}

static void wiz_eof(struct socket *s)
{
	s->s_iflag |= SI_EOF;
	wiz_wakeall(s);
}

/*
 *	Process interrupts from the WizNet device
 */
void wiz_event_s(uint8_t i)
{
	struct socket *s = &sockets[i];
	uint16_t stat = wiz_readw(Sn_IR + (i << 8));	/* BE read of reg pair */

	if (stat & 0x800) {
		/* Transmit completed: window re-open. We can allow more
		   data to flow from the user */
		s->s_iflag &= ~SI_THROTTLE;
		wiz_writeb(Sn_IR + (i << 8), 0x08);	/* Clear the flag down */
		wakeup(&s->s_data);
	}
	if (stat & 0x400) {
		/* Receive wake: Poke the user in case they are reading */
		s->s_iflag |= SI_DATA;
		wiz_writeb(Sn_IR + (i << 8), 0x04);	/* Clear the flag down */
		wakeup(&s->s_iflag);
	}
	if (stat & 0x200) {
		/* Disconnect: Just kill our host socket. Not clear if this
		   is right or we need to drain data first */
		s->state = SS_CLOSED;
		wiz_eof(s);
	}
	if (stat & 0x100) {
		/* Connect: Move into connected state */
		if (s->s_state == SS_CONNECTING) {
			s->s_state = SS_CONNECTED;
			wakeup(s);
		}
	}
	/* ??? return if high bits set here ?? */
	switch (stat & 0xFF) {
	case 0:		/* SOCK_CLOSED */
		if (s->s_state != SS_CLOSED && s->s_state != SS_UNUSED) {
			s->s_state = SS_CLOSED;
			if (s->s_state != SS_CLOSING) {
				s->s_error = ECONNRESET;	/* Sort of a guess */
				wiz_wakeall(s);
			} else
				wakeup(s);
		}
		irqmask &= ~(1 << i);
		wiz_writeb(Sn_IMR, irqmask);
		break;
	case 0x13:		/* SOCK_INIT */
		break;
	case 0x14:		/* SOCK_LISTEN */
		break;
	case 0x17:		/* SOCK_ESTABLISHED */
		if (s->s_state == SS_CONNECTING) {
			s->s_state == SS_CONNECTED;
			wakeup(s);
		} else if (s->s_state == SS_LISTEN) {
			/* TODO: We actually have to split the association between
			   wiznet and host sockets as we have multiple wiznet sockets
			   in the accept queue and a listener that is not really a
			   wiznet socket.. */
		}
		break;
	case 0x1C:		/* SOCK_CLOSE_WAIT */
		if (s->s_state == SS_CONNECTED
		    || s->s_state == SS_CONNECTING)
			s->s_state == SS_CLOSEWAIT;
		wiz_eof(s);
		if (s->s_state == SS_ACCEPTWAIT) {
			/* HUM ??? */
		}
		break;
	case 0x22:		/* SOCK_UDP */
	case 0x32:		/* SOCK_IPRAW */
	case 0x42:		/* SOCK_MACRAW */
		/* Socket has been created */
		s->s_state = SS_UNCONNECTED;
		wakeup(s);
		break;
	}
}

void wiz_event(void)
{
	uint8_t irq;
	uint8_t i = 0;


	/* Polling cases */
	irq = wiz_readb(IR2);
	if (irq == 0)
		return;

	while (irq) {
		if (irq & 1)
			wiz_event_s(i);
		irq >>= 1;
		i++;
		s++;
	}
}

void wiz_poll(void)
{
	if (irqmask)
		wiz_event();
}

/* State management for creation of a socket. If need be allocate the socket
   on the IP offload device. May block */
int net_init(struct socket *s)
{
	s->s_state = SS_UNCONNECTED;
	return 0;
}

/* Bind a socket to an address. May block */
int net_bind(struct socket *s)
{
	uint16_t i = s - sockets;
	uint8_t r = SOCK_INIT;

	switch (s->sock_type) {
	case SOCKTYPE_TCP:
		wiz_writeb(Sn_MR + (i << 8), 0x01);	/* TCP */
		wiz_writew(Sn_PORT0 + (i << 8), s->s_addr[SADDR_SRC].port);
		break;
	case SOCKTYPE_UDP:
		wiz_writeb(Sn_MR + (i << 8), 0x02);	/* UDP */
		wiz_writew(Sn_PORT0 + (i << 8), s->s_addr[SADDR_SRC].port);
		r = SOCK_UDP;
		break;
	case SOCKTYPE_RAW:
		wiz_writeb(Sn_PROTO + (i << 8), s->s_addr[ADDR_SRC].port);	/* hack */
		wiz_writeb(Sn_MR + (i << 8), 0x03);	/* RAW */
		r = SOCK_IPRAW;
	}
	wiz_writeb(Sn_CR + (i << 8), OPEN);

	if (wiz_readb(Sn_SR + (i << 8)) != r) {
		udata.u_error = EADDRINUSE;	/* Something broke ? */
		return -1;
	}
	irqmask |= 1 << i;
	wiz_writeb(Sn_IMR, irqmask);
	s->s_state = SS_BOUND;
	/* Do we need to delay the SS_BOUND until the chip interrupts ? */
	return 0;
}

/* Start a socket listening. This expects BSD unix like semantics so you
   may well need to map between host and offload sockets. May block. */

/* This isn't quite right - we need to allocate a free socket and use it
   as the listener and keep doing so when one comes in. The listening socket
   in fact has no real binding to the wiz one - fixme and use s->s_lcn for
   channel mapping */
int net_listen(struct socket *s)
{
	uint16_t i = s - sockets;

	i <<= 8;

	wiz_writeb(Sn_CR + i, LISTEN);
	if (wiz_readb(Sn_SR + i) != SOCK_LISTEN) {
		udata.u_error = EPROTO;	/* ??? */
		return -1;
	}
	s->s_state = SS_LISTENING;
	return 0;
}

/* Start connecting to a remote host. Should not block. Return SS_CONNECTING
   state if in progress, SS_CONNECTED if done immediately, other states for
   error fails. */
int net_connect(struct socket *s)
{

	if (s->s_type == SOCKTYPE_TCP) {
		uint16_t i;
		i = s - sockets;
		i <<= 8;
		wiz_writel(Sn_DIRP0 + i, s->s_addr[SADDR_DST].addr);
		wiz_writew(Sn_DPORT0 + i, s->s_addr[SADDR_DST].port);
		wiz_writeb(Sn_NR + i, CONNECT);
		s->s_state = SS_CONNECTING;
	} else {
		/* UDP/RAW - note have to do our own filtering for 'connect' */
		s->s_state = SS_CONNECTED;
	}
	return 0;
}

/* Close down a socket - preferably politely */
void net_close(struct socket *s)
{
	uint16_t i = s - sockets;

	if (s->s_type == SOCKTYPE_TCP) {
		wiz_writeb(Sn_CR + (i << 8), DISCON);
		s->s_state = SS_CLOSING;
	} else {
		wiz_clear_int(i);
		wiz_writeb(Sn_CR + (i << 8), CLOSE);
		s->s_state = SS_UNINIT;
	}
}

arg_t net_read(struct socket *s, uint8_t flag)
{
	uint16_t n;
	uint16_t r;
	uint16_t i = s - sockets;

	i <<= 8;

	/* FIXME: IRQ protection */
	/* Wait for data - push int core code ? */
	while ((s->s_iflag & SI_DATA) == 0) {
		/* See if we have lost the link */
		if (s->s_state < SS_CONNECTED) {
			udata.u_error = EINVAL;
			return -1;
		}
		/* Check for an EOF (covers post close cases too) */
		if (s->s_iflag & SI_EOF)
			return 0;
		/* Needs irq protection */
		if (psleep_flags(&s->iflag, flag))
			return -1;
		/* Keep waiting until we get the right state */
		/* Bytes available */
		n = wiz_readw_n(Sn_RX_RSR + i);
		if (n)
			s->s_iflag |= SI_DATA;
	}
	switch (s->s_type) {
	case SOCKTYPE_RAW:
	case SOCKTYPE_UDP:
		/* UDP comes with a header */
		wiz_dequeue(i, 4, (uint8_t *) & s->s_addr[SADDR_TMP].addr);
		if (s->s_type == SOCKTYPE_UDP)
			wiz_dequeue(i, 2,
				    (uint8_t *) & s->s_addr[SADDR_TMP].
				    port);
		wiz_dequeue(i, 2, (uint8_t *) & n);	/* Actual packet size */
		n = ntohs(n);	/* Big endian on device */
		/* Fall through */
	case SOCKTYPE_TCP:
		/* Bytes to consume */
		r = min(n, udata.u_count);
		/* Now dequeue some bytes into udata.u_base */
		wiz_dequeue_u(i, r, udata.u_base);
		/* For datagrams we always discard the entire frame */
		wiz_writew_n(Sn_TX_WR0 + i, wiz_readw_n(Sn_TX_WR0 + i)
			     + s->s_type == SOCKTYPE_TCP ? r : (n + 8));
		/* FIXME: figure out if SI_DATA should be cleared */
		return r;
	}
}

arg_t net_write(struct socket * s, uint8_t flag)
{
	uint16_t i = s - sockets;
	uint16_t room;
	uint16_t n;
	uint8_t a = s->s_flag & SFLAG_ATMP ? SADDR_TMP : SADDR_SADDR_DST;

	i <<= 8;

	room = wiz_readw_n(Sn_TX_FSR + i);

	switch (s->s_socktype) {
	case SOCKTYPE_UDP:
		if (udata.u_count > 1472) {
			udata.u_error = EMSGSIZE;
			return -1;
		}
	case SOCKTYPE_IPRAW:
		if (udata.u_count > 1500) {
			udata.u_error = EMSGSIZE;
			return -1;
		}
		if (room < udata.u_count)
			return -2;
		wiz_writel(Sn_DIRPO + i, s->s_addr[a].addr);
		wiz_writel(Sn_DPORT0 + i, s->s_addr[a].port);
		/* Fall through */
	case SOCKTYPE_TCP:
		if (room == 0)
			return -2;
		n = min(room, udata.u_count);
		wiz_queue_u(i, n, udata.u_base);
		wiz_writew_n(Sn_TX_WR0 + i,
			     wiz_readw_n(Sn_TX_WR0 + i) + n);
		break;
	}
	udata.u_count = n;
	return n;
}

/* Everything below this line is still pure sketching of ideas as we don't
   really have a configuration interface designed yet ! */

struct netdevice net_dev = {
	6,			/* MAC size */
	"eth0",			/* Good a name as any */
	0,			/* No special flags */
};

/* Only some of these hit this code, most are handled by the core */
arg_t net_ioctl(uint8_t op, void *p)
{
	uint16_t n;

	switch (op) {
	case OP_SIFADDR:
		wiz_bwrite(SIPR0, p, 4);
		break;
	case OP_SIFMASK:
		wiz_bwrite(SUBR0, p, 4);
		break;
	case OP_SIFGW:
		wiz_bwrite(GAR0, p, 4);
		break;
	case OP_GIFHWADDR:
		wiz_bread(SHAR0, p, 6);
		break;
	case OP_SIFHWADDR:
		wiz_bwrite(SHAR0, p, 6);
		break;
	case OP_GPHY:
		return (wiz_bread(PSTATUS) & 0x20) ? LINK_UP : LINK_DOWN;
	default:
		return -EINVAL;
	}
	return 0;
}

void netdev_init(void)
{
	uint16_t i;
	for (i = 0; i < 8; i++) {
		wiz_writeb(Sn_RXMEM_SIZE + (i << 8), 0x2000);
		wiz_writeb(Sn_TXMEM_SIZE + (i << 8), 0x2000);
	}
}
