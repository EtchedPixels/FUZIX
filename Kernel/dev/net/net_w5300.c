/*
 *	WizNET 5300 Driver
 *
 *	The W5300 is a strange offshoot of the line with 16bit registers and
 *	a somewhat wacky 16bit FIFO arrangement. We split the driver from the
 *	main WizNet driver to keep the main one clean.
 *
 *	TODO:
 *	- Why does ifconfig sometimes hang
 *	- Possible scribble
 *	- Can we do anything in software for the bad reset case
 *	- Test write properly
 *	- Test UDP
 *	- Test IPRAW socket support
 */

#include <kernel.h>

#ifdef CONFIG_NET_W5300

#include <kdata.h>
#include <printf.h>
#include <netdev.h>
#include <net_w5300.h>

#define NSOCKET		8
#define WIZ_SOCKET	8

#define W5300_TCP	0x21
#define W5300_UDP	0x02
#define W5300_RAW	0x03

#define MR		0x000
#define		MR_DBW		0x8000
#define		MR_MPF		0x4000
#define		MR_WDF		0x3800
#define		MR_RDH		0x0400
#define		MR_FS		0x0100
#define		MR_RESET	0x0080
#define		MR_MT		0x0020
#define		MR_PB		0x0010
#define		MR_PPPOE	0x0008
#define		MR_DBS		0x0004
#define		MR_INDIRECT	0x0001
#define IR		0x002
#define IMR		0x004
#define SHAR		0x008
#define SHAR2		0x00A
#define SHAR3		0x00C
#define GAR		0x010
#define GAR2		0x012
#define SUBR		0x014
#define	SUBR2		0x016
#define SIPR		0x018
#define SIPR2		0x01A
#define RTR		0x01C
#define RCR		0x01E
#define TMSRx		(0x020 + (x))
#define RMSRx		(0x028 + (x))
#define MTYPER		0x030
#define PATR		0x032
#define PTIMER		0x036
#define PMAGICR		0x038
#define PSIDR		0x03C
#define PDHAR		0x040
#define PDHAR2		0x042
#define PDHAR4		0x044
#define UIPR		0x048
#define UIPR2		0x04A
#define UPORTR		0x04C
#define FMTUR		0x04E
#define Px_BRDYR	(0x060 + 2 * (x))
#define Px_BDPOTHR	(0x062 + 2 * (x))
#define IDR		0x0FE
#define 	VERSION_ID	0x5300

#define Sn_MR		0x200
#define Sn_CR		0x202
#define 	OPEN		0x01
#define		LISTEN		0x02
#define		CONNECT		0x04
#define		DISCON		0x08
#define		CLOSE		0x10
#define		SEND		0x20
#define		SEND_MAC	0x21
#define		SEND_KEEP	0x22
#define		RECV		0x40
#define Sn_IMR		0x204
#define Sn_IR		0x206
#define		I_SEND_OK	0x10
#define		I_TIMEOUT	0x08
#define		I_RECV		0x04
#define		I_DISCON	0x02
#define		I_CON		0x01
#define Sn_SSR		0x208
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
#define		SOCK_MACRAW		0x42
#define		SOC_PPPOE		0x5F
#define Sn_PORTR	0x20A
#define SN_DHAR		0x20C
#define SN_DHAR2	0x20E
#define SN_DHAR4	0x210
#define Sn_DPORTR	0x212
#define Sn_DIPR		0x214
#define Sn_DIPR2	0x216
#define Sn_MSSR		0x218
#define Sn_PORTOR	0x21A
#define Sn_TOSR		0x21C
#define Sn_TTLR		0x21E
#define Sn_TX_WRSR	0x220
#define Sn_TX_WRSR2	0x222
#define Sn_TX_FSR2	0x226
#define Sn_RX_RSR	0x228
#define Sn_RX_RSR2	0x22A
#define Sn_RX_FRAGR	0x22C
#define Sn_TX_FIFOR	0x22E
#define Sn_RX_FIFOR	0x230


/*
 *	Low level helpers
 */


void w5300_writes(uint_fast8_t s, uint16_t off, uint16_t val)
{
	w5300_write(off + 0x40 * s, val);
}

void w5300_writesn(uint_fast8_t s, uint16_t off, uint16_t val)
{
	w5300_writen(off + 0x40 * s, val);
}

uint16_t w5300_reads(uint_fast8_t s, uint16_t off)
{
	return w5300_read(off + 0x40 * s);
}

uint16_t w5300_reads_atomic(uint_fast8_t s, uint16_t off)
{
	register uint16_t r,n;
	off += 0x40 * s;

	do {
		r = w5300_read(off);
		n = w5300_read(off);
	} while(n != r);
	return r;
}

uint16_t w5300_readsn(uint_fast8_t s, uint16_t off)
{
	return w5300_readn(off + 0x40 * s);
}

/* This is native endian already */
void w5300_writeip(uint16_t off, uint32_t val)
{
	w5300_writen(off + 2, val >> 16);
	w5300_writen(off, val);
}

void w5300_writesip(uint_fast8_t s, uint16_t off, uint32_t val)
{
	off += 0x40 * s;
	w5300_writeip(off, val);
}

/* For now until we work out where this really belongs */
uint8_t sock_wake[NSOCKET];

static uint8_t irqmask;

static uint8_t wiznet_present;

static uint8_t sock_free = 0xFF;
static uint8_t wiz2sock_map[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };


static void w5300_cmd(uint8_t s, uint8_t v)
{
	uint16_t n = 0xFFFF;
	w5300_writes(s, Sn_CR, v);
	while(n-- && (w5300_reads(s, Sn_CR) & 0xFF));
	if (n == 0)
		kputs("w5300: not responding - check your reset signal is clean.\n");
}

/* Must not be called except from interrupt state as we don't protect
   the flag updates */
static void w5300_eof(struct socket *s)
{
	s->s_iflags |= SI_EOF;
	s->s_wake = 1;
}

/* State management for creation of a socket. */
static int net_alloc(void)
{
	register uint_fast8_t i = 0;
	uint_fast8_t j = 1;
	while(i < NSOCKET) {
		if (sock_free & j) {
			sock_free &= ~j;
			return i;
		}
		j <<= 1;
		i++;
	}
	return -1;
}

static void fifo_init(uint_fast8_t s);

/*
 *	Allocate a socket and bind it to a physical socket. Update the
 *	map tables in the process.
 */
static struct socket *netproto_create(void)
{
	int n;
	register int i;
	register struct socket *s;
	for (i = 0; i < WIZ_SOCKET; i++) {
		if (wiz2sock_map[i] == 0xFF) {
			n = net_alloc();
			if (n == -1)
				return NULL;
			s = sockets + n;
			/* Some of this belongs in core code ? */
			s->s_num = n;
			s->s_parent = 0xFF;
			s->proto.slot = i;
			wiz2sock_map[i] = n;
			fifo_init(i);
			return s;
		}
	}
	return NULL;
}

/*
 *	Mark a socket as free and release the associated wiznet channel
 */
void netproto_free(struct socket *s)
{
	wiz2sock_map[s->proto.slot] = 0xFF;
	sock_free |= (1 << s->s_num);
}

/* Until we do incoming socket support */
struct socket *netproto_sockpending(struct socket *s)
{
	register struct socket *n = sockets;
	register uint_fast8_t id = s->s_num;
	int i;
	for (i = 0; i < NSOCKET; i++) {
		if (n->s_state != SS_UNUSED && n->s_parent == id) {
			n->s_parent = 0xFF;
			return n;
		}
		n++;
	}
	return NULL;
}

/*
 *	Used when a socket is finally cleaned up and nobody either side
 *	wants it.
 */
static void netproto_cleanup(struct socket *s)
{
	uint16_t i = s->proto.slot;

	irqmask &= ~(1 << i);
	w5300_write(IMR, irqmask);
	w5300_cmd(i, CLOSE);
	s->s_state = SS_UNUSED;
	netproto_free(s);
}

/* Bind a socket to an address.*/
/* We arrange the internal types we use to match the chip
   - 21 TCP 02 UDP 03 RAW

   The higher level stuff is done in netproto_bind, this
   function merely handles the poking of the device */
static int do_netproto_bind(struct socket *s)
{
	uint16_t i = s->proto.slot;
	uint16_t r = SOCK_INIT;
	uint16_t n;

	w5300_writes(i, Sn_MR, s->s_type);
	/* Make an open request to open the socket */
	switch (s->s_type) {
	case W5300_UDP:
		r = SOCK_UDP;
	case W5300_TCP:
		/* We keep ports net endian so don't byte swap */
		w5300_writes(i, Sn_PORTR, s->src_addr.sa.sin.sin_port);
		break;
	case W5300_RAW:
		r = SOCK_IPRAW;
		n = w5300_reads(i, Sn_PORTOR);
		n &= 0xFF00;
		n |= s->s_protocol;
		w5300_writes(i, Sn_PORTOR, n);
		break;
	}
	w5300_cmd(i, OPEN);
	/* If the reply is not immediately SOCK_INIT we failed */
	n = w5300_reads(i, Sn_SSR);
	if ((n & 0xFF) != r)
		kputs("w5300: bind error.\n");
	/* Interrupt on if available mark as bound */
	irqmask |= 1 << i;
	w5300_write(IMR, irqmask);
	s->s_state = SS_BOUND;
	s->src_len = sizeof(struct sockaddr_in);
	return 0;
}

/*
 *	Process interrupts from the WizNet device
 */
static void w5300_event_s(uint8_t i)
{
	uint_fast8_t sn = wiz2sock_map[i];
	register struct socket *s;
	register uint16_t stat = w5300_reads(i, Sn_IR);

	/* Fold status and event flags together */
	stat <<= 8;
	stat |= w5300_reads(i, Sn_SSR);

//	kprintf("sock %d slot %d event %x\n",
//		sn, i, stat);

	/* We got a pending event for a dead socket as we killed it. Shoot it
	   again to make sure it's dead */
	if (sn == 0xFF) {
		w5300_cmd(i, CLOSE);
		irqmask &= ~(1 << i);
		w5300_write(IMR, irqmask);
		return;
	}
	s = sockets + sn;

	/* A send completed. We are permitted to queue more bytes. Differs
	   from the behaviour on the other chips a bit */
	if (stat & 0x1000) {
		/* Transmit completed: window re-open. We can allow more
		   data to flow from the user */
		s->s_iflags &= ~(SI_THROTTLE|SI_WAIT);
		s->s_wake = 1;
	}
	if (stat & 0x800) {
		/* Timeout */
		s->s_error = ETIMEDOUT;
		w5300_cmd(i, CLOSE);
		s->s_wake = 1;
		w5300_eof(s);
		/* Fall through and let CLOSE state processing do the work */
	}
	if (stat & 0x400) {
		/* Receive wake: Poke the user in case they are reading */
		s->s_iflags |= SI_DATA;
		s->s_wake = 1;
	}
	if (stat & 0x200) {
		/* Signal an EOF */
		w5300_eof(s);
		/* When we fall through we'll see CLOSE state and do the
		   actual shutting down if appropriate */
	}
	if (stat & 0x100) {
		/* Connect: Move into connected state */
		if (s->s_state == SS_CONNECTING) {
			s->s_state = SS_CONNECTED;
			s->s_wake = 1;
		}
	}
	/* Clear interrupt sources down */
	w5300_writes(i, Sn_IR, stat >> 8);

	switch ((uint8_t)stat) {
	case 0:		/* SOCK_CLOSED */
		if (s->s_state != SS_CLOSED && s->s_state != SS_UNUSED) {
			if (s->s_state != SS_CLOSING)
				s->s_error = ECONNRESET;	/* Sort of a guess */
			s->s_wake = 1;
			w5300_eof(s);
			/* Net layer wants us to burn the socket */
			if (s->s_state == SS_CLOSING)
				/* FIXME: some of this should be in network.c */
				netproto_cleanup(s);
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
			s->s_state = SS_CONNECTED;
			s->s_wake = 1;
		} else if (s->s_state == SS_LISTENING) {
			struct socket *ns;
			int s1;
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
			ns = netproto_create();
			if (ns == NULL) {
				/* No socket to re-use for the listen, cycle
				   this one */
				w5300_cmd(i, CLOSE);
				do_netproto_bind(s);
				break;
			}
			/* Swap the bindings over so that ns becomes the data
			   recipient and s the new listener */
			s1 = s->proto.slot;
			s->proto.slot = ns->proto.slot;
			ns->proto.slot = s1;
			/* From this point on 's' is the listening socket but
			   attached to the new instance, and ns is the 'new'
			   socket attached to the old one */
			/* Fix up the new socket data */
			memcpy(&ns->src_addr, &s->src_addr, sizeof(struct ksockaddr));
			memcpy(&ns->dst_addr, &s->dst_addr, sizeof(struct ksockaddr));
			ns->s_type = s->s_type;
			ns->src_len = s->src_len;
			ns->dst_len = s->dst_len;
			ns->s_ino = NULL;
			ns->s_iflags = 0;
			ns->s_error = 0;
			ns->s_state = SS_CONNECTED;
			/* So accept can find it */
			ns->s_parent = sn;
			/* Now fix up the new physical channel and put it into
			   listen */
			do_netproto_bind(s);
			netproto_listen(s);
			s->s_wake = 1;
		}
		break;
	case 0x1C:		/* SOCK_CLOSE_WAIT */
		if (s->s_state == SS_CONNECTED
		    || s->s_state == SS_CONNECTING)
			s->s_state = SS_CLOSEWAIT;
		w5300_eof(s);
		if (s->s_state == SS_ACCEPTWAIT) {
			/* HUM ??? */
		}
		break;
	case 0x22:		/* SOCK_UDP */
	case 0x32:		/* SOCK_IPRAW */
	case 0x42:		/* SOCK_MACRAW */
		/* Socket has been created */
		s->s_state = SS_UNCONNECTED;
		s->s_wake = 1;
		break;
	}
	/* Set the socket wake up flag and then wake it. We will need
	   sock_wake to be common for splitting it properly, and the wakeup
	   to be a callback */
	if (s->s_wake) {
		sock_wake[sn] = 1;
		wakeup(sock_wake + sn);
//		kprintf("wake %d\n", sn);
	}
}

void w5300_event(void)
{
	register uint_fast8_t irq;
	uint_fast8_t i = 0;
	register struct socket *s = sockets;

	/* For now ignore the upper bits */
	irq = w5300_read(IR);
	if (irq == 0)
		return;

	while (irq) {
		if (irq & 1)
			w5300_event_s(i);
		irq >>= 1;
		i++;
		s++;
	}
}

void w5300_poll(void)
{
	if (irqmask)
		w5300_event();
}

struct socktype {
	uint8_t family;
	uint8_t type;
	uint8_t protocol;
	uint8_t info;
};

static struct socktype socktype[4] = {
	{ AF_INET, SOCK_STREAM, IPPROTO_TCP, W5300_TCP },
	{ AF_INET, SOCK_DGRAM, IPPROTO_UDP, W5300_UDP },
	{ AF_INET, SOCK_DGRAM, IPPROTO_UDP, W5300_RAW },
	{ 0, }
};

/* Set up protocol private fields in new socket */
void netproto_setup(struct socket *s)
{
}

/* Helper belongs in network.c ? */
int netproto_socket(void)
{
	register struct socktype *st = socktype;
	register struct socket *s;
	uint_fast8_t famok = 0;

	if (!wiznet_present) {
		udata.u_error = ENETDOWN;
		return 0;
	}
	s = netproto_create();
	if (s == NULL) {
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
				s->s_class = udata.u_net.args[2];
				s->s_state = SS_UNCONNECTED;
				s->s_type = st->info;
				s->s_protocol = udata.u_net.args[3];
				udata.u_net.sock = s->s_num;
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


/* TODO: the wiznet is very different in API here - when a connection
   completes incoming it takes over that socket and we need to create a new
   socket for the next one. That will need some kind of split socket/wiznet
   indexing or a way to update the host side mapping (which may be cleaner) */
int netproto_listen(struct socket *s)
{
	/* Issue a listen command. Check the state went to SOCK_LISTEN */
	w5300_cmd(s->proto.slot, LISTEN);
	if (w5300_reads(s->proto.slot, Sn_SSR) != SOCK_LISTEN) {
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
   in just hardware. */
int netproto_begin_connect(struct socket *s)
{
	if (s->s_type == W5300_TCP) {
		uint16_t i = s->proto.slot;
		/* Already net endian */
		w5300_writesip(i, Sn_DIPR, udata.u_net.addrbuf.sa.sin.sin_addr.s_addr);
		w5300_writesn(i, Sn_DPORTR, udata.u_net.addrbuf.sa.sin.sin_port);
		w5300_cmd(i, CONNECT);
		s->s_state = SS_CONNECTING;
		s->dst_len = sizeof(struct sockaddr_in);
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
	if (s->s_state == SS_LISTENING) {
		struct socket *n = sockets;
		uint8_t p = s->s_num;
		uint8_t j;
		/* Close any accept queue sockets. This will only recurse
		   one deep */
		for (j = 0; j < NSOCKET; j++) {
			if (n->s_parent == p)
				netproto_close(n);
			n++;
		}
	}
	if (s->s_type == W5300_TCP && s->s_state >= SS_CONNECTING && s->s_state <= SS_CONNECTED) {
		w5300_cmd(s->proto.slot, DISCON);
		s->s_state = SS_CLOSING;
	} else
		netproto_cleanup(s);
	return 0;
}

/*
 *	FIFO management. The FIFO is a pain in the backside in several ways
 *	and we need to keep track of the state.
 *
 *	We use this in two ways - for UDP and RAW we read entire frames each
 *	time and it's not too hard - just discard the leftovers. For TCP we
 *	get given stuff packetised on arbitrary inconvenient boundaries and
 *	have to track everything.
 */


struct w5300_fifo {
	uint16_t left;
	uint8_t val;
	uint8_t valid;
};

struct w5300_fifo rx_state[NSOCKET];


/* This assumes we put the FIFO in native endian order */
static uint8_t *fifo_readu_bytes(uint16_t addr, uint_fast8_t s, uint8_t *uptr, uint16_t len)
{
	register struct w5300_fifo *f = rx_state + s;

	if (len == 0)
		return uptr;

	/* There is half a left over FIFO word pending - use it */
	if (f->valid) {
		 _uputc(f->val, uptr++);
		 len--;
		f->valid = 0;
	}
	/* Copy whole words */
	while(len > 1) {
		_uputw(w5300_read(addr), uptr);
		len -= 2;
		uptr += 2;
	}
	/* Pull a last word from the FIFO and stash half of it for next time */
	if (len) {
		uint16_t v = w5300_read(addr);
		/* If there is 1 byte left in the real packet and we are
		 * reading it then we should not save the next byte as it's padding to
		 * align the header correctly
		 */
		if (f->left > 1) {
			f->val  = v >> 8;
			f->valid = 1;
		}
		uputc(v, uptr++);
	}
	return uptr;
}

static uint16_t fifo_readu(uint_fast8_t s, uint8_t *uptr, uint16_t len, uint_fast8_t flush)
{
	register struct w5300_fifo *f = rx_state + s;
	uint16_t addr = Sn_RX_FIFOR + 0x40 * s;
	uint16_t n;

	/* See how much remains, in this frame if some left, if not then
	   the FIFO top is a word giving the next packet size */

	if (f->left == 0) {
		f->left = w5300_readn(addr);
//	   	kprintf("fifo: new packet size %d\n", f->left);
	}
	n = f->left;

//	kprintf("fifo_readu: s %d up %p l %d f %d\n", s, uptr, len, flush);

	/*
	 *	Pull the bytes up to the next header if they will fit, if
	 *	not do a partial pull
	 */
	if (n > len)
		n = len;
	f->left -= n;
	uptr = fifo_readu_bytes(addr, s, uptr, n);
	len -= n;

	/*
	 *	If flush is set the discard any remaining bits for this packet
	 */

	if (flush) {
		n = f->left;
		f->left = 0;
		/* Discard any odd byte we had to take out the FIFO */
		if (f->valid) {
			f->valid = 0;
			n--;
		}
		/* Flush words */
		while(n > 1) {
			w5300_read(addr);
			n -= 2;
		}
		/* And flush any trailing byte */
		if (n)
			w5300_read(addr);
	}
	/* We finished mid packet. The remainder is in f->left so we will
	 pick it up next time. Do not send a RECV as we don't want to
	 flush the frame yet */
	if (f->left)
		return 0;
	/* This frame is complete */
	w5300_cmd(s, RECV);
	/* Nothing is left  */
	return len;
}

static void fifo_init(uint_fast8_t s)
{
	struct w5300_fifo *f = rx_state + s;
	f->valid = 0;
	f->left = 0;
}
	
/*
 *	Read. This is a bit nasty because we cannot read a partial non word
 *	sized chunk from the FIFO and we must also deal with embedded length
 *	fields with each chunk. If we were copying to internal buffers and
 *	then user space it would be lovely, but we are not so it's painful.
 */
int netproto_read(struct socket *s)
{
	uint16_t i = s->proto.slot;
	register uint16_t n;
	register uint16_t r;
	uint8_t filtered;
	uint8_t flush = 0;

	if (udata.u_count == 0)
		return 0;

	/* Bytes available */
	/* FIXME: set SI_DATA on CONNECTED state or UDP connect etc and
	   check higher up without this check here */
	do {
		filtered = 0;
		/* Bytes pending */
		n = rx_state[i].left;
		if (n == 0)
			n = w5300_reads_atomic(i, Sn_RX_RSR2);
//		kprintf("read pending %d\n", n);
		if (n == 0) {
			if (s->s_iflags & SI_EOF)
				return 0;
			return 1;
		}
		memcpy(&udata.u_net.addrbuf, &s->dst_addr, sizeof(struct ksockaddr));

		switch (s->s_type) {
		case W5300_RAW:
		case W5300_UDP:
			/* UDP comes with a header. The FIFO is byteswapping on a little endia box
			   but swapping bytes only not word order. The target address is big
			   endian. Are we confused yet ? */
			udata.u_net.addrbuf.sa.sin.sin_addr.s_addr = w5300_readsn(i, Sn_RX_FIFOR);
			udata.u_net.addrbuf.sa.sin.sin_addr.s_addr |= ((uint32_t)w5300_readsn(i, Sn_RX_FIFOR)) << 16;
			udata.u_net.addrbuf.sa.sin.sin_port = w5300_readsn(i, Sn_RX_FIFOR);

			if (s->src_addr.sa.sin.sin_addr.s_addr && s->src_addr.sa.sin.sin_addr.s_addr !=
				udata.u_net.addrbuf.sa.sin.sin_addr.s_addr) {
				filtered = 1;
			}
			flush = 1;
		case W5300_TCP:
			/* Bytes to consume */
			r = min(n, udata.u_count);
			/* returns the space left, so this turns r into the
			   amount actually ready */
			if (filtered)
				fifo_readu(i, udata.u_base, 0, 1);
			else {
				r -= fifo_readu(i, udata.u_base, r, flush);
				if (r == 0) {
				/* Check for an EOF (covers post close cases too) */
					if (s->s_iflags & SI_EOF)
						return 0;
					/* Wait */
					return 1;
				}
				udata.u_done += r;
				udata.u_base += r;
				return 0;
			}
		}
	} while(filtered);
	return 0;
}

/* We must avoid racing an interrupt handler when maninpulating the flags */
static void set_iflags(struct socket *s, uint8_t flags)
{
	irqflags_t irq = di();
	s->s_iflags |= flags;
	irqrestore(irq);
}

arg_t netproto_write(struct socket *s, struct ksockaddr *ka)
{
	register uint16_t i = s->proto.slot;
	register uint16_t room;
	uint16_t addr = Sn_TX_FIFOR + 0x40 * i;
	uint16_t n = 0;
	uint16_t len;

	/* Is a SEND in progress ? */
	if (s->s_iflags & SI_WAIT)
		return 1;

	/* FIFO is available, but what room is left ? */
	room = w5300_reads_atomic(i, Sn_TX_FSR2);

	switch (s->s_type) {
	case W5300_UDP:
		if (udata.u_count > 1472) {
			udata.u_error = EMSGSIZE;
			return 0;
		}
	case W5300_RAW:
		if (udata.u_count > 1500) {
			udata.u_error = EMSGSIZE;
			return 0;
		}
		if (room < udata.u_count)
			return 0;
		/* Already native endian */
		w5300_writesip(i, Sn_DIPR, ka->sa.sin.sin_addr.s_addr);
		w5300_writesn(i, Sn_DPORTR, ka->sa.sin.sin_port);
		/* Fall through */
	case W5300_TCP:
		/* No room blocks, a partial write returns */
		if (room == 0) {
			set_iflags(s, SI_THROTTLE);
			return 1;
		}
		set_iflags(s, SI_WAIT);

		/* Transmit is via a FIFO rather than a ring buffer on the
		   other WizNET devices */
		len = min(room, udata.u_count);
		n = len; 
		while(n > 1) { 
		        w5300_write(addr, _ugetw(udata.u_base));
		        udata.u_base += 2;
		        n -= 2;
		}
		if (n)
		        w5300_write(addr, _ugetc(udata.u_base++));
		w5300_writes(i, Sn_TX_WRSR2 , len);
		w5300_cmd(i, SEND);
		break;
	}
	udata.u_done += len;
	return 0;
}

arg_t netproto_shutdown(struct socket *s, uint8_t flag)
{
	set_iflags(s, flag);
	if (s->s_iflags & SI_SHUTW)
		w5300_cmd(s->proto.slot, DISCON);
	/* Really we need to look for SHUTR and received data and CLOSE if
	   so - FIXME */
	return 0;
}

static uint32_t ipa;
static uint8_t fakeaddr[6] = { 0xC0, 0xFF, 0xEE, 0xC0, 0xFF, 0xEE };
static uint32_t iga;
static uint32_t igm;
static uint16_t autoport = ntohs(5000);
static uint16_t mtu;
static uint16_t ifflags = IFF_BROADCAST|IFF_RUNNING|IFF_UP;

static void netdev_reload(void)
{
	register uint16_t *p = (uint16_t *)fakeaddr;
	w5300_writeip(SIPR, ipa);
	w5300_writeip(GAR, iga);
	w5300_writeip(SUBR, igm);
	w5300_writen(SHAR, *p++);
	w5300_writen(SHAR + 2, *p++);
	w5300_writen(SHAR + 4, *p);
}

arg_t netproto_ioctl(struct socket *s, int op, char *ifr_u /* in user space */)
{
	static struct ifreq ifr;
	if (uget(ifr_u, &ifr, sizeof(struct ifreq)))
		return -1;
	if (op != SIOCGIFNAME && strcmp(ifr.ifr_name, "eth0")) {
		udata.u_error = ENODEV;
		return -1;
	}
	switch(op) {
	/* Get side */
	case SIOCGIFNAME:
		if (ifr.ifr_ifindex) {
			udata.u_error = ENODEV;
			return -1;
		}
		memcpy(ifr.ifr_name, "eth0", 6);
		goto copyback;
	case SIOCGIFINDEX:
		ifr.ifr_ifindex = 0;
		goto copyback;
	case SIOCGIFFLAGS:
		ifr.ifr_flags = ifflags;
#ifdef PSTATUS
		if (w5300_readcb(PSTATUS) & 0x20)
			ifr.ifr_flags |= IFF_LINKUP;
#endif
		goto copyback;
	case SIOCGIFADDR:
		ifr.ifr_addr.sa.sin.sin_addr.s_addr = ipa;
		goto copy_addr;
	case SIOCGIFBRDADDR:
		/* Hardcoded in the engine */
		ifr.ifr_broadaddr.sa.sin.sin_addr.s_addr = (ipa & igm) | ~igm;
		goto copy_addr;
	case SIOCGIFNETMASK:
		ifr.ifr_netmask.sa.sin.sin_addr.s_addr = igm;
		goto copy_addr;
	case SIOCGIFGWADDR:
		ifr.ifr_gwaddr.sa.sin.sin_addr.s_addr = iga;
		goto copy_addr;
	case SIOCGIFHWADDR:
		memcpy(ifr.ifr_hwaddr.sa.hw.shw_addr, fakeaddr, 6);
		ifr.ifr_hwaddr.sa.hw.shw_family = HW_ETH;
		goto copyback;
	case SIOCGIFMTU:
		ifr.ifr_mtu = 1500;
		goto copyback;
	/* Set side */
	case SIOCSIFFLAGS:
		/* Doesn't really do anything ! */
		ifflags &= ~IFF_UP;
		ifflags |= ifr.ifr_flags & IFF_UP;
		return 0;
	case SIOCSIFADDR:
		ipa = ifr.ifr_addr.sa.sin.sin_addr.s_addr;
		break;
	case SIOCSIFNETMASK:
		igm = ifr.ifr_netmask.sa.sin.sin_addr.s_addr;
		break;
	case SIOCSIFGWADDR:
		iga = ifr.ifr_gwaddr.sa.sin.sin_addr.s_addr;
		break;
	case SIOCSIFHWADDR:
		memcpy(fakeaddr, ifr.ifr_hwaddr.sa.hw.shw_addr, 6);
		break;
	default:
		udata.u_error = EINVAL;
		return -1;
	}
	netdev_reload();
	return 0;
copy_addr:
	ifr.ifr_addr.sa.sin.sin_family = AF_INET;
copyback:
	return uput(&ifr,ifr_u, sizeof(ifr));
}

void netdev_init(void)
{
	ipa = ntohl(0xC0A80001);
	iga = ntohl(0xC0A800FE);
	igm = ntohl(0xFFFFFF00);

	w5300_setup();	

	if (w5300_read(IDR) == VERSION_ID) {
		kputs("Wiznet 5300 detected.\n");
		wiznet_present = 1;
	}
	// hack for some emu debug work
	wiznet_present = 1;

	w5300_write(IMR, 0);
	/* Set GAR, SHAR, SUBR, SIPR to defaults ? */
	netdev_reload();
}

/* We probably want a generic ipv4 helper layer for some of this */
int netproto_find_local(struct ksockaddr *ka)
{
	register struct socket *s = sockets;
	register uint_fast8_t n = 0;
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
	if (s->s_type != W5300_RAW) {
		do {
			s->src_addr.sa.sin.sin_port = autoport;
			inc_autoport();
		} while (netproto_find_local(&s->src_addr) != -1);
	}
	return do_netproto_bind(s);
}

int netproto_bind(struct socket *s)
{
	if (udata.u_net.addrbuf.sa.family != AF_INET) {
		udata.u_error = EPROTONOSUPPORT;
		return 0;
	}
	if (udata.u_net.addrbuf.sa.sin.sin_addr.s_addr &&
			udata.u_net.addrbuf.sa.sin.sin_addr.s_addr != ipa) {
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
