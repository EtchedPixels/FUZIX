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
  - rework disconnect state machine
 */

#include <kernel.h>

#ifdef CONFIG_NET_WIZNET

#include <kdata.h>
#include <printf.h>
#include <netdev.h>
#include <net_w5100.h>

static uint8_t irqmask;
static uint8_t sock_free = 0x0F;
static uint8_t wiz2sock_map[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

#define RX_MASK 	0x07FF
#define TX_MASK		0x07FF

#define MR		0x0000
#define		MR_RESET	0x80
#define		MR_PB		0x10
#define		MR_PPPOE	0x08
#define		MR_AUTOINC	0x02
#define		MR_INDIRECT	0x01

#define GAR0		0x0001
#define SUBR0		0x0005
#define SHAR0		0x0009
#define SIPR0		0x000F
#define IR		0x0015
#define IMR		0x0016
#define RTR0		0x0017
#define RCR		0x0019
#define RMSR		0x001A
#define TMSR		0x001B
#define PATR0		0x001C
#define PTIMER		0x0028
#define PMAGIC		0x0029
#define UIPR0		0x002A
#define UPORT0		0x002E

#define Sn_MR		0x0400
#define Sn_CR		0x0401
#define 	OPEN		0x01
#define		LISTEN		0x02
#define		CONNECT		0x04
#define		DISCON		0x08
#define		CLOSE		0x10
#define		SEND		0x20
#define		SEND_MAC	0x21
#define		SEND_KEEP	0x22
#define		RECV		0x40
#define	Sn_IR		0x0402
#define		I_SEND_OK	0x10
#define		I_TIMEOUT	0x08
#define		I_RECV		0x04
#define		I_DISCON	0x02
#define		I_CON		0x01
#define Sn_SR		0x0403
#define		SOCK_CLOSED		0x00
#define		SOCK_ARP		0x01
#define		SOCK_INIT		0x13
#define		SOCK_LISTEN		0x14
#define		SOCK_SYNSENT		0x15
#define		SOCK_SYNRECV		0x16
#define		SOCK_ESTABLISHED	0x17
#define		SOCK_FIN_WAIT		0x18
#define		SOCK_CLOSING		0x1A
#define		SOCK_TIME_WAIT		0x1B
#define		SOCK_CLOSE_WAIT		0x1C
#define		SOCK_LAST_ACK		0x1D
#define		SOCK_UDP		0x22
#define		SOCK_IPRAW		0x32
#define		SOCK_MACROW		0x42
#define		SOC_PPPOE		0x5F
#define Sn_PORT0	0x0404
#define Sn_DHAR0	0x0406
#define Sn_DIPR0	0x040C
#define Sn_DPORT0	0x0410
#define Sn_MSSR0	0x0412
#define Sn_PROTO	0x0414
#define Sn_TOS		0x0415
#define Sn_TTL		0x0416
#define Sn_TX_FSR	0x0420
#define Sn_TX_RD0	0x0422
#define Sn_TX_WR0	0x0424
#define Sn_RX_RSR	0x0426
#define Sn_RX_RD0	0x0428



/* FIXME: look for ways to fold these four together */
static void w5100_queue(uint16_t i, uint16_t n, void *p)
{
	uint16_t dm = w5100_readw(Sn_TX_WR0 + i) & TX_MASK;
	uint16_t tx_base = 0x4000 + (i << 3);	/* i is already << 8 */

	if (dm + n > TX_MASK) {
		uint16_t us = TX_MASK + 1 - dm;
		w5100_bwrite(dm + tx_base, p, us);
		w5100_bwrite(tx_base, p + us, n - us);
	} else
		w5100_bwrite(dm + tx_base, p, n);
}

static void w5100_queue_u(uint16_t i, uint16_t n, void *p)
{
	uint16_t dm = w5100_readw(Sn_TX_WR0 + i) & TX_MASK;
	uint16_t tx_base = 0x4000 + (i << 3);	/* i is already << 8 */

	if (dm + n > TX_MASK) {
		uint16_t us = TX_MASK + 1 - dm;
		w5100_bwriteu(dm + tx_base, p, us);
		w5100_bwriteu(tx_base, p + us, n - us);
	} else
		w5100_bwriteu(dm + tx_base, p, n);
}

static void w5100_dequeue(uint16_t i, uint16_t n, void *p)
{
	uint16_t dm = w5100_readw(Sn_RX_RD0 + i) & RX_MASK;
	uint16_t rx_base = 0x6000 + (i << 3);	/* i is already << 8 */

	if (dm + n > RX_MASK) {
		uint16_t us = RX_MASK + 1 - dm;
		w5100_bread(dm + rx_base, p, us);
		w5100_bread(rx_base, p + us, n - us);
	} else
		w5100_bread(dm + rx_base, p, n);
}

static void w5100_dequeue_u(uint16_t i, uint16_t n, void *p)
{
	uint16_t dm = w5100_readw(Sn_RX_RD0 + i) & RX_MASK;
	uint16_t rx_base = 0x6000 + (i << 3);	/* i is already << 8 */

	if (dm + n > RX_MASK) {
		uint16_t us = RX_MASK + 1 - dm;
		w5100_breadu(dm + rx_base, p, us);
		w5100_breadu(rx_base, p + us, n - us);
	} else
		w5100_breadu(dm + rx_base, p, n);
}

static void w5100_eof(struct socket *s)
{
	s->s_iflags |= SI_EOF;
	s->wake = 1;
}

/* State management for creation of a socket. */
static int net_alloc(void)
{
	uint8_t i = 0;
	uint8_t j = 1;
	while(i < NSOCKET) {
		if (sock_free & j) {
			sock_free &= ~j;
			return i;
		}
		j <<= 1;
	}
	return -1;
}

void netproto_free(struct socket *s)
{
	sock_free |= (1 << s->proto.slot);
}

/* Until we do incoming socket support */
struct socket *netproto_sockpending(struct socket *s)
{
	return NULL;
}

/*
 *	Process interrupts from the WizNet device
 */
static void w5100_event_s(uint8_t i)
{
	int sn = wiz2sock_map[i];
	struct socket *s;
	uint16_t offset = i << 8;
	uint16_t stat = w5100_readw(Sn_IR + offset);	/* BE read of reg pair */

	/* We got a pending event for a dead socket as we killed it. Shoot it
	   again to make sure it's dead */
	if (sn == 0xFF) {
		w5100_writeb(Sn_CR + offset, CLOSE);
		irqmask &= ~(1 << i);
		w5100_writeb(IMR, irqmask);
		return;
	}

	s = sockets + sn;

/*	kprintf("event %x on socket %d\n", stat, i); */
	if (stat & 0x1000) {
		/* Transmit completed: window re-open. We can allow more
		   data to flow from the user */
		s->s_iflags &= ~SI_THROTTLE;
		s->wake = 1;
	}
	if (stat & 0x800) {
		/* Timeout */
		s->s_error = ETIMEDOUT;
		w5100_writeb(Sn_CR + offset, CLOSE);
		s->wake = 1;
		w5100_eof(s);
		/* Fall through and let CLOSE state processing do the work */
	}
	if (stat & 0x400) {
		/* Receive wake: Poke the user in case they are reading */
		s->s_iflags |= SI_DATA;
		s->wake = 1;
	}
	if (stat & 0x200) {
		/* Disconnect: Just kill our host socket. Not clear if this
		   is right or we need to drain data first */
		w5100_writeb(Sn_CR + offset, CLOSE);
		w5100_eof(s);
		/* When we fall through we'll see CLOSE state and do the
		   actual shutting down */
	}
	if (stat & 0x100) {
		/* Connect: Move into connected state */
		if (s->s_state == SS_CONNECTING) {
			s->s_state = SS_CONNECTED;
			s->wake = 1;
		}
	}
	/* Clear interrupt sources down */
	w5100_writeb(Sn_IR + offset, stat >> 8);

	switch ((uint8_t)stat) {
	case 0:		/* SOCK_CLOSED */
		if (s->s_state != SS_CLOSED && s->s_state != SS_UNUSED) {
			if (s->s_state != SS_CLOSING && s->s_state != SS_DEAD)
				s->s_error = ECONNRESET;	/* Sort of a guess */
			s->wake = 1;
			w5100_eof(s);
			/* Net layer wants us to burn the socket */
			if (s->s_state == SS_DEAD) {
				/* FIXME: some of this should be in network.c */
				wiz2sock_map[i] = 0xFF;
				irqmask &= ~(1 << i);
				w5100_writeb(IMR, irqmask);
				netproto_free(s);
				s->s_state = SS_UNUSED;
			}
			else	/* so net_close() burns the socket */
				s->s_state = SS_CLOSED;
		}
		break;
	case 0x13:		/* SOCK_INIT */
		break;
	case 0x14:		/* SOCK_LISTEN */
		break;
	case 0x17:		/* SOCK_ESTABLISHED */
		if (s->s_state == SS_CONNECTING) {
			s->s_state == SS_CONNECTED;
				s->wake = 1;
		} else if (s->s_state == SS_LISTENING) {
#if 0
			/*
			 * The WizNET believes you allocte a LISTEN socket and
			 * it turns into a connection and you then if need be
			 * allocate another LISTEN socket.
			 *
			 * The socket API believes you set a listening socket
			 * up and it stays listening creating new connected
			 * sockets.
			 *
			 * We do some gymnastics to convince both sides that
			 * what they saw happened.
			 */
			int slot;
			struct socket *ac;
			int aslot;
			/* Find a new wiznet slot and socket */
			slot = net_alloc();
			if (slot == -1 || (ac = sock_alloc_accept(s)) == NULL) {
				/* No socket free, go back to listen */
				w5100_writeb(Sn_CR + offset, CLOSE);
				net_bind(s);
				net_listen(s);
				break;
			}
			/* Resources exist so do the juggling */
			aslot = ac - sockets;

			/* Map the existing socket to the new w5100 socket */
			sock2wiz_map[sn] = slot;
			wiz2sock_map[slot] = sn;
			/* Map the new socket to the existing w5100 socket */
			sock2wiz_map[aslot] = i;
			wiz2sock_map[i] = aslot;
			/* Now set the new socket back up as it should be */
			net_bind(ac);
			net_listen(ac);
			/* And kick the accepter */
			s->wake = 1;
#endif
		}
		break;
	case 0x1C:		/* SOCK_CLOSE_WAIT */
		if (s->s_state == SS_CONNECTED
		    || s->s_state == SS_CONNECTING)
			s->s_state = SS_CLOSEWAIT;
		w5100_eof(s);
		if (s->s_state == SS_ACCEPTWAIT) {
			/* HUM ??? */
		}
		break;
	case 0x22:		/* SOCK_UDP */
	case 0x32:		/* SOCK_IPRAW */
	case 0x42:		/* SOCK_MACRAW */
		/* Socket has been created */
		s->s_state = SS_UNCONNECTED;
		s->wake = 1;
		break;
	}
	/* Until we work out how to make this a nice callback */
	if (s->wake) {
		wakeup(s->s_ino);
	}
}

void w5100_event(void)
{
	uint8_t irq;
	uint8_t i = 0;
	struct socket *s = sockets;

	/* Polling cases */
	irq = w5100_readb(IR) & 0x0F;
	if (irq == 0)
		return;

	while (irq) {
		if (irq & 1)
			w5100_event_s(i);
		irq >>= 1;
		i++;
		s++;
	}
}

void w5100_poll(void)
{
	if (irqmask)
		w5100_event();
}

#define W5100_TCP	0x21
#define W5100_UDP	0x02
#define W5100_RAW	0x03

struct socktype {
	uint8_t family;
	uint8_t type;
	uint8_t protocol;
	uint8_t info;
};

static struct socktype socktype[4] = {
	{ AF_INET, SOCK_STREAM, IPPROTO_TCP, W5100_TCP },
	{ AF_INET, SOCK_DGRAM, IPPROTO_UDP, W5100_UDP },
	{ AF_INET, SOCK_RAW, 0, W5100_RAW },
	{ 0, }
};

/* Set up protocol private fields in new socket */
void netproto_setup(struct socket *s)
{
}

/* Helper belongs in network.c ? */
int netproto_socket(void)
{
	struct socktype *st = socktype;
	int n = net_alloc();
	uint8_t famok = 0;
	struct socket *s = sockets + n;
	if (n < 0) {
		udata.u_error = ENOBUFS;
		return 0;
	}
	/* Now check if it's one we can do */
	while(st->family) {
		if (st->family == udata.u_net.args[1]) {
			famok = 1;
			if (st->type == udata.u_net.args[2] &&
				(st->protocol == 0 || udata.u_net.args[3] == 0 || udata.u_net.args[3] == st->protocol)) {
				net_setup(s);
				s->s_state = SS_UNCONNECTED;
				s->s_type = st->info;
				/* For now */
				s->proto.slot = n;
				udata.u_net.sock = n;
				/* Set up socket state etc */
				return 0;
			}
		}
		st++;
	}
	if (famok)
		udata.u_error = EPROTONOSUPPORT;
	else
		udata.u_error = EAFNOSUPPORT;
	return 0;
}

/* Bind a socket to an address. May block */
/* We arrange the internal types we use to match the chip
   - 21 TCP 02 UDP 03 RAW

   The higher level stuff is done further done in netproto_bind, this
   function merely handles the poking of the device */
static int do_netproto_bind(struct socket *s)
{
	uint16_t i = s->proto.slot;
	uint8_t r = SOCK_INIT;
	uint16_t off = i << 8;

	w5100_writeb(Sn_MR + off, s->s_type);
	switch (s->s_type) {
	case W5100_UDP:
		r = SOCK_UDP;
	case W5100_TCP:
		/* We keep ports net endian so don't byte swap */

		w5100_bwrite(Sn_PORT0 + off, &s->src_addr.sa.sin.sin_port, 2);
		break;
	case W5100_RAW:
		/* FIXME: check endianness here */
		w5100_writeb(Sn_MR + off, s->src_addr.sa.sin.sin_port);
		r = SOCK_IPRAW;
	}
	/* Make an open request to open the socket */
	w5100_writeb(Sn_CR + off, OPEN);

	/* If the reply is not immediately SOCK_INIT we failed */
	if (w5100_readb(Sn_SR + off) != r) {
		udata.u_error = EADDRINUSE;	/* Something broke ? */
		return -1;
	}
	/* Interrupt on if available mark as bound */
	irqmask |= 1 << i;
	w5100_writeb(IMR, irqmask);
	/* FOR NOW TODO */
	wiz2sock_map[i] = i;
	return 0;
}

/* TODO: the wiznet is very different in API here - when a connection
   completes incoming it takes over that socket and we need to create a new
   socket for the next one. That will need some kind of split socket/wiznet
   indexing or a way to update the host side mapping (which may be cleaner) */
int netproto_listen(struct socket *s)
{
	uint16_t i = s->proto.slot;

	i <<= 8;

	/* Issue a listen command. Check the state went to SOCK_LISTEN */
	w5100_writeb(Sn_CR + i, LISTEN);
	if (w5100_readb(Sn_SR + i) != SOCK_LISTEN) {
		udata.u_error = EIO;//FIXME EPROTO;	/* ??? */
		return 0;
	}
	s->s_state = SS_LISTENING;
	return 0;
}

int netproto_accept_complete(struct socket *s)
{
	return 0;
}

/* Start connecting to a remote host. We can't implement the UDP case correctly
   in just hardware. FIXME - add filters */
int netproto_begin_connect(struct socket *s)
{
	if (s->s_type == W5100_TCP) {
		uint16_t i;
		i = s->proto.slot << 8;
		/* Already net endian */
		w5100_bwrite(Sn_DIPR0 + i, &udata.u_net.addrbuf.sa.sin.sin_addr, 4);
		w5100_bwrite(Sn_DPORT0 + i, &udata.u_net.addrbuf.sa.sin.sin_port, 2);
		w5100_writeb(Sn_CR + i, CONNECT);
		s->s_state = SS_CONNECTING;
		return 1;
	} else {
		/* UDP/RAW - note have to do our own filtering for 'connect' */
		s->s_state = SS_CONNECTED;
	}
	return 0;
}

/* Close down a socket - preferably politely */
int netproto_close(struct socket *s)
{
	uint16_t i = s->proto.slot;
	uint16_t off = i << 8;

	if (s->s_type == W5100_TCP && s->s_state != SS_CLOSED) {
		w5100_writeb(Sn_CR + off, DISCON);
		s->s_state = SS_CLOSING;
	} else {
		irqmask &= ~(1 << i);
		w5100_writeb(IMR, irqmask);
		w5100_writeb(Sn_CR + off, CLOSE);
		/* FIXME: remove duplication with interrupt close path */
		sock_free |= (1 << i);
		netproto_free(s);
		wiz2sock_map[i] = 0xFF;
		s->s_state = SS_UNUSED;
	}
	return 0;
}

int netproto_read(struct socket *s)
{
	uint16_t n;
	uint16_t r;
	uint16_t i = s->proto.slot;
	uint8_t st;

	i <<= 8;

	if (udata.u_count == 0)
		return 0;

	/* Bytes available */
	/* FIXME: set SI_DATA on CONNECTED state or UDP connect etc and
	   check higher up without this check here */
	n = w5100_readw(Sn_RX_RSR + i);
	if (n == 0) {
		if (s->s_iflags & SI_EOF)
			return 0;
		return 1;
	}
	s->s_iflags |= SI_DATA;

	memcpy(&udata.u_net.addrbuf, &s->dst_addr, sizeof(struct ksockaddr));

	switch (s->s_type) {
	case W5100_RAW:
	case W5100_UDP:
		/* TODO filter */
		/* UDP comes with a header */
		w5100_dequeue(i, 4, &udata.u_net.addrbuf.sa.sin.sin_addr);
		if (s->s_type == W5100_UDP)
			w5100_dequeue(i, 2, &udata.u_net.addrbuf.sa.sin.sin_addr);
		w5100_dequeue(i, 2, (uint8_t *) & n);	/* Actual packet size */
		n = ntohs(n);	/* Big endian on device */
		/* Fall through */
	case W5100_TCP:
		/* Bytes to consume */
		r = min(n, udata.u_count);
		if (r == 0) {
			/* Check for an EOF (covers post close cases too) */
			if (s->s_iflags & SI_EOF)
				return 0;
			/* Wait */
			return 1;
		}
		/* Now dequeue some bytes into udata.u_base */
		w5100_dequeue_u(i, r, udata.u_base);
		udata.u_done += r;
		udata.u_base += r;
		/* For datagrams we always discard the entire frame */
		if (s->s_type == W5100_UDP)
			r = n + 8;
		w5100_writew(Sn_RX_RD0 + i, w5100_readw(Sn_RX_RD0 + i) + r);
		/* FIXME: figure out if SI_DATA should be cleared */
		/* Now tell the device we ate the data */
		w5100_writeb(Sn_CR + i, RECV);
	}
	return 0;
}

arg_t netproto_write(struct socket * s, struct ksockaddr *ka)
{
	uint16_t i = s->proto.slot;
	uint16_t room;
	uint16_t n = 0;

	i <<= 8;

	room = w5100_readw(Sn_TX_FSR + i);

	switch (s->s_type) {
	case W5100_UDP:
		if (udata.u_count > 1472) {
			udata.u_error = EMSGSIZE;
			return 0;
		}
	case W5100_RAW:
		if (udata.u_count > 1500) {
			udata.u_error = EMSGSIZE;
			return 0;
		}
		if (room < udata.u_count)
			return 0;
		/* Already native endian */
		w5100_bwrite(Sn_DIPR0 + i, &ka->sa.sin.sin_addr, 4);
		w5100_bwrite(Sn_DPORT0 + i, &ka->sa.sin.sin_addr, 4);
		/* Fall through */
	case W5100_TCP:
		/* No room blocks, a partial write returns */
		if (room == 0) {
			s->s_iflags |= SI_THROTTLE;
			return 1;
		}
		n = min(room, udata.u_count);
		w5100_queue_u(i, n, udata.u_base);
		w5100_writew(Sn_TX_WR0 + i,
			     w5100_readw(Sn_TX_WR0 + i) + n);
		w5100_writeb(Sn_CR + i, SEND);
		break;
	}
	udata.u_done += n;
	udata.u_base += n;
	return 0;
}

arg_t netproto_shutdown(struct socket *s, uint8_t flag)
{
	int i = s->proto.slot << 8;
	s->s_iflags |= flag;
	if (s->s_iflags & SI_SHUTW)
		w5100_writeb(Sn_CR + i, DISCON);
	/* Really we need to look for SHUTR and received data and CLOSE if
	   so - FIXME */
	return 0;
}

/* Everything below this line is still pure sketching of ideas as we don't
   really have a configuration interface designed yet ! */

struct netdevice net_dev = {
	6,			/* MAC size */
	"eth0",			/* Good a name as any */
	0,			/* No special flags */
};
#if 0
/* Only some of these hit this code, most are handled by the core */
arg_t net_ioctl(uint8_t op, void *p)
{
	uint16_t n;

	switch (op) {
	case OP_SIFADDR:
		w5100_bwrite(SIPR0, p, 4);
		break;
	case OP_SIFMASK:
		w5100_bwrite(SUBR0, p, 4);
		break;
	case OP_SIFGW:
		w5100_bwrite(GAR0, p, 4);
		break;
	case OP_GIFHWADDR:
		w5100_bread(SHAR0, p, 6);
		break;
	case OP_SIFHWADDR:
		w5100_bwrite(SHAR0, p, 6);
		break;
	case OP_GPHY:
		return (w5100_readb(PSTATUS) & 0x20) ? LINK_UP : LINK_DOWN;
	default:
		return -EINVAL;
	}
	return 0;
}
#endif
static uint32_t ipa = 0x00000000;	/* Tmp hack */
static uint8_t fakeaddr[6] = { 0xC0, 0xFF, 0xEE, 0xC0, 0xFF, 0xEE };
static uint32_t iga = 0x020000C0;
static uint32_t igm = 0x00FFFFFF;
static uint16_t autoport = ntohs(5000);

void netdev_init(void)
{
	uint16_t i;

	w5100_setup();	
	w5100_writeb(IMR, 0);
//	w5100_writeb(RTR, );
//	w5100_writeb(RCR, );
	/* Set GAR, SHAR, SUBR, SIPR to defaults ? */
	w5100_bwrite(SIPR0, &ipa, 4);
	w5100_bwrite(GAR0, &iga, 4);
	w5100_bwrite(SUBR0, &igm, 4);
	w5100_bwrite(SHAR0, fakeaddr, 6);
	w5100_writeb(RMSR, 0x55);	/* 2k a socket */
	w5100_writeb(TMSR, 0x55);	/* 2k a socket */
	for (i = 0; i < 4 * 256; i += 256) {
		/* Do we need to set anything here */
	}
}

/* We probably want a generic ipv4 helper layer for some of this */
int netproto_find_local(struct ksockaddr *ka)
{
	struct socket *s = sockets;
	uint8_t n = 0;
	while(n < NSOCKET) {
		if (s->s_state < SS_BOUND || s->src_addr.sa.family != AF_INET) {
			s++;
			n++;
			continue;
		}
		if (s->src_addr.sa.sin.sin_port == ka->sa.sin.sin_port) {
			if (s->src_addr.sa.sin.sin_addr.s_addr ==
				ka->sa.sin.sin_addr.s_addr ||
			    s->src_addr.sa.sin.sin_addr.s_addr == 0)
			        return n;
		}
		s++;
		n++;
	}
	return -1;
}


static void inc_autoport(void)
{
	uint16_t port = ntohs(autoport);
	port++;
	if (port == 32767)
		port = 5000;
	autoport = ntohs(port);
}

int netproto_autobind(struct socket *s)
{
	s->src_addr.sa.family = AF_INET;
	s->src_addr.sa.sin.sin_addr.s_addr = 0;
	/* Raw socket autobind is a no-op for port */
	if (s->s_type != W5100_RAW) {
		do {
			s->src_addr.sa.sin.sin_port = autoport;
			inc_autoport();
		} while (netproto_find_local(&s->src_addr) != -1);
	}
	do_netproto_bind(s);
	return 0;
}

int netproto_bind(struct socket *s)
{
	if (udata.u_net.addrbuf.sa.family != AF_INET) {
		udata.u_error = EPROTONOSUPPORT;
		return 0;
	}
	if (udata.u_net.addrbuf.sa.sin.sin_addr.s_addr != ipa) {
		udata.u_error = EADDRNOTAVAIL;
		return 0;
	}
	if (ntohs(udata.u_net.addrbuf.sa.sin.sin_port) < 1024 && udata.u_euid != 0) {
		udata.u_error = EACCES;
		return 0;
	}
	memcpy(&s->src_addr, &udata.u_net.addrbuf, sizeof(struct ksockaddr));
	do_netproto_bind(s);
	return 0;
}

#endif
