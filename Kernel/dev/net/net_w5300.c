/*
 *	First draft of an implementation of the low level half of the
 *	network code for a WizNet 5300.
 */

/* TODO 
  - server socket fixes (we should allocate one channel to the listening
    socket and when it connects allocate another one. That means we need to
    use to break the 1:1 mapping between wiznet and OS socket numbering
  - handle address collision interrupt
  - rework disconnect state machine
  
  - handle FMTUR and MTU changing via Sn_MSSR
  - review MSR, TOS, keepalive settings
 */

#include <kernel.h>

#ifdef CONFIG_NET_WIZNET5300

#include <kdata.h>
#include <printf.h>
#include <netdev.h>
#include <net_w5300.h>
#include <platform_w5300.h>

static uint16_t irqmask;
static uint8_t w5300;

#define RX_MASK 	0x07FF
#define TX_MASK		0x07FF

#define MR0		0x0000
#define		MR0_DBW		0x80
#define		MR0_MPF		0x40
#define		MR0_WDF		0x38
#define		MR0_RDH		0x04
#define		MR0_LE		0x01	/* Little endian */
#define MR1		0x0001
#define		MR1_RESET	0x80
#define		MR1_PB		0x10
#define		MR1_PPPOE	0x08
/* No autoinc on the 5300 */
#define		MR1_INDIRECT	0x01

#define IDM_AR		0x0002
#define IDM_DR		0x0004

#define IR		0x0002
#define IR_IPCF			0x80
#define IR_DPUR			0x40
#define IR_PPPT			0x20
#define IR_FMTU			0x10
#define IR_SnINT(n)		(1 << n)

#define IMR		0x0004
#define IMR_MASK		0xF0FF
#define ICFGR		0x0006
#define SHAR		0x0008
#define GAR		0x0010
#define SUBR		0x0014
#define SIPR		0x0018
#define RTR		0x001C
#define RCR		0x001E
#define TMSR0		0x0020
#define TMSR1		0x0021
#define TMSR2		0x0022
#define TMSR3		0x0023
#define TMSR4		0x0024
#define TMSR5		0x0025
#define TMSR6		0x0026
#define TMSR7		0x0027

#define RMS0		0x0028
#define RMS1		0x0029
#define RMS2		0x002A
#define RMS3		0x002B
#define RMS4		0x002C
#define RMS5		0x002D
#define RMS6		0x002E
#define RMS7		0x002F

#define MTYPER		0x0030
#define PATR		0x0032
#define PPALGOR		0x0034
#define PTIMER		0x0036
#define PMAGIC		0x0038
#define PSTATER		0x003A
#define PSIDR		0x003C
#define PDHAR		0x0040
#define UIPR		0x0048
#define UPORTR		0x004C
#define FMTUR		0x004E
#define	Sn_RTCR(n)	((n * 2) + 0x50)
#define Pn_BRDYR(n)	((n * 4) + 0x60)
#define Pn_PEN			0x80
#define Pn_MT			0x40
#define Pn_PPL			0x20
#define Pn_SN(n)		((n) & 0x07)
#define Pn_BDPTHR(n)	((n * 4) + 0x62)
#define IDR		0x00FE
#define VERSIONR	0x00FE

/* 0x200 base 0x40 per socket */

#define Sn_MR		0x0200
#define	Sn_MR_ALIGN		0x0100
#define Sn_MR_MULTI		0x0080
#define Sn_MR_MF		0x0040
#define Sn_MR_IGPMv		0x0020
#define Sn_MR_ND		0x0010
#define Sn_MR_TYPE		0x000F
#define Sn_MR_CLOSE		0x0000
#define Sn_MR_TCP		0x0001
#define Sn_MR_UDP		0x0002
#define Sn_MR_IPRAW		0x0003
#define Sn_MR_MACRAW		0x0004
#define Sn_MR_PPPoE		0x0005

#define Sn_CR		0x0202
#define 	OPEN		0x01
#define		LISTEN		0x02
#define		CONNECT		0x04
#define		DISCON		0x08
#define		CLOSE		0x10
#define		SEND		0x20
#define		SEND_MAC	0x21
#define		SEND_KEEP	0x22
#define		RECV		0x40
#define		PCON		0x23
#define		PDISCON		0x24
#define		PCR		0x25
#define		PCN		0x26
#define		PCJ		0x27
#define	Sn_IMR		0x0204
#define Sn_IR		0x0206
#define		I_RECV_FAIL	0x80
#define		I_PFAIL		0x40
#define		I_PNEXT		0x20
#define		I_SEND_OK	0x10
#define		I_TIMEOUT	0x08
#define		I_RECV		0x04
#define		I_DISCON	0x02
#define		I_CON		0x01
#define Sn_SSR		0x0208
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
#define Sn_PORTR	0x020A
#define Sn_DHAR		0x020C
#define Sn_DPORTR	0x0212
#define Sn_DIPR		0x0214
#define Sn_MSSR		0x0218
#define Sn_KPALVTR	0x021A
#define Sn_PROTOR	0x021A
#define Sn_TOSR		0x021C
#define Sn_TTLR		0x021E
#define Sn_TX_WRSR	0x0220
#define Sn_TX_FSR	0x0224
#define Sn_RX_RSR	0x0228
#define Sn_FRAGR	0x022C
#define Sn_TX_FIFOR	0x022E
#define Sn_RX_FIFOR	0x0230

#define SOCKOFF(n)	(0x40 * (n))


#ifdef CONFIG_WIZNET_INDIRECT

void w5300_writew(uint16_t port, uint16_t val)
{
	idm_arh = port >> 8;
	idm_arl = port;
	idm_drh = val >> 8;
	idm_drl = val;
}

void w5300_writebe(uint16_t port, uint16_t val)
{
	idm_arh = port >> 8;
	idm_arl = port;
	idm_drh = val;
	idm_drl = val >> 8;
}

uint16_t w5300_readw(uint16_t port)
{
	idm_arh = port >> 8;
	idm_arl = port;
	return (idm_drh << 8) | idm_drl;
}

void w5300_setup(void)
{
	idm_mr0 = 0;
	idm_mr1 = MR1_RESET;
	idm_mr1 = MR1_INDIRECT;
}
	
#endif

	
/* These are somewhat cleaner on the 5300 as the interface is a FIFO interface
   not a memory writing one. It's not quite as nice as it could be as the
   interface is word wide */

static void w5300_queue(uint16_t i, uint16_t n, uint8_t * p)
{
	/* FIXME: double check byte sized I/O */
	uint16_t port = Sn_TX_FIFOR + SOCKOFF(i);
	for (i = 0; i < n; i += 2) {
		w5300_writew(port, *p << 8 | p[1]);
		p += 2;
	}
}

static void w5300_queue_u(uint16_t i, uint16_t n, uint8_t * p)
{
	uint16_t port = Sn_TX_FIFOR + SOCKOFF(i);
	uint16_t a;
	uint8_t b;
	/* Caller has validated */
	for (i = 0; i < n; i += 2) {
		a = _ugetc(p++);
		b = _ugetc(p++);
		w5300_writew(port, a << 8 | b);
	}
}

static void w5300_dequeue(uint16_t i, uint16_t n, uint8_t * p)
{
	/* FIXME: double check byte sized I/O */
	uint16_t port = Sn_RX_FIFOR + SOCKOFF(i);
	uint16_t d;
	
	for (i = 0; i < n; i++) {
		if (!(i & 1)) {
			d = w5300_readw(port);
			*p++ = d >> 8;
		} else
			*p++ = d;
	}
}

static void w5300_dequeue_u(uint16_t i, uint16_t n, uint8_t * p)
{
	/* FIXME: double check byte sized I/O */
	uint16_t port = Sn_RX_FIFOR + SOCKOFF(i);
	uint16_t d;
	
	for (i = 0; i < n; i++) {
		if (!(i & 1)) {
			d = w5300_readw(port);
			uputc(d >> 8, p++);
		} else
			uputc(d, p++);
	}
}

static void w5300_wakeall(struct socket *s)
{
	wakeup(s);
	wakeup(&s->s_iflag);
	wakeup(&s->s_data);
}

static void w5300_eof(struct socket *s)
{
	s->s_iflag |= SI_EOF;
	w5300_wakeall(s);
}

/* Mapping between Wiznet and host sockets. We can't have a 1:1 mapping due
   to the differing way the Wiznet processes accepting a connection */

static uint8_t sock2wiz_map[8] = { 0xFF, 0xFF, 0xFF, 0xFF };
static uint8_t wiz2sock_map[8] = { 0xFF, 0xFF, 0xFF, 0xFF };

/* State management for creation of a socket. If need be allocate the socket
   on the IP offload device. May block */
static int net_alloc(void)
{
	uint8_t *p = wiz2sock_map;
	uint8_t i;
	for (i = 0; i < 8; i++) {
		if (*p++ == 0xFF)
			return i;
		i++;
	}
	return -1;
}

/*
 *	Process interrupts from the WizNet device
 */
static void w5300_event_s(uint8_t i)
{
	int sn = wiz2sock_map[i];
	struct socket *s;
	uint16_t offset = SOCKOFF(i);
	uint16_t stat = w5300_readw(Sn_IR + offset);

	/* We got a pending event for a dead socket as we killed it. Shoot it
	   again to make sure it's dead */
	if (sn == 0xFF) {
		w5300_writew(Sn_CR + offset, CLOSE);
		irqmask &= ~(1 << i);
		w5300_writew(IMR, irqmask);
		return;
	}

	s = &sockets[sn];
	/* TODO: RECV_FAIL/PFAIL/PNEXT */

	if (stat & I_SEND_OK) {
		/* Transmit completed: window re-open. We can allow more
		   data to flow from the user */
		s->s_iflag &= ~SI_THROTTLE;
		wakeup(&s->s_data);
	}
	if (stat & I_TIMEOUT) {
		/* Timeout */
		s->s_error = ETIMEDOUT;
		w5300_writew(Sn_CR + offset, CLOSE);
		w5300_wakeall(s);
		w5300_eof(s);
		/* Fall through and let CLOSE state processing do the work */
	}
	if (stat & I_RECV) {
		/* Receive wake: Poke the user in case they are reading */
		s->s_iflag |= SI_DATA;
		wakeup(&s->s_iflag);
	}
	if (stat & I_DISCON) {
		/* Disconnect: Just kill our host socket. Not clear if this
		   is right or we need to drain data first */
		w5300_writew(Sn_CR + offset, CLOSE);
		w5300_eof(s);
		/* When we fall through we'll see CLOSE state and do the
		   actual shutting down */
	}
	if (stat & I_CON) {
		/* Connect: Move into connected state */
		if (s->s_state == SS_CONNECTING) {
			s->s_state = SS_CONNECTED;
			wakeup(s);
		}
	}
	
	/* REVIEW */
	/* Clear interrupt sources down */
	w5300_writew(Sn_IR + offset, stat >> 8);

	/* See what state we are n and decide what to do about it */
	switch (stat >> 8) {
	case SOCK_CLOSED:
		if (s->s_state != SS_CLOSED && s->s_state != SS_UNUSED) {
			if (s->s_state != SS_CLOSING && s->s_state != SS_DEAD) {
				s->s_error = ECONNRESET;	/* Sort of a guess */
				w5300_wakeall(s);
			} else
				wakeup(s);
			irqmask &= ~(1 << i);
			w5300_writew(IMR, irqmask);
			w5300_eof(s);
			/* Net layer wants us to burn the socket */
			if (s->s_state == SS_DEAD) {
				wiz2sock_map[i] = 0xFF;
				sock_closed(s);
			}
			else	/* so net_close() burns the socket */
				s->s_state = SS_CLOSED;
		}
		break;
	case SOCK_INIT:
		break;
	case SOCK_LISTEN:
		break;
	case SOCK_ESTABLISHED:
		if (s->s_state == SS_CONNECTING) {
			s->s_state == SS_CONNECTED;
			wakeup(s);
		} else if (s->s_state == SS_LISTENING) {
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
				w5300_writew(Sn_CR + offset, CLOSE);
				net_bind(s);
				net_listen(s);
				break;
			}
			/* Resources exist so do the juggling */
			aslot = ac - sockets;

			/* Map the existing socket to the new w5300 socket */
			sock2wiz_map[sn] = slot;
			wiz2sock_map[slot] = sn;
			/* Map the new socket to the existing w5300 socket */
			sock2wiz_map[aslot] = i;
			wiz2sock_map[i] = aslot;
			/* Now set the new socket back up as it should be */
			net_bind(ac);
			net_listen(ac);
			/* And kick the accepter */
			wakeup(s);
		}
		break;
	case SOCK_CLOSE_WAIT:
		if (s->s_state == SS_CONNECTED
		    || s->s_state == SS_CONNECTING)
			s->s_state = SS_CLOSEWAIT;
		w5300_eof(s);
		if (s->s_state == SS_ACCEPTWAIT) {
			/* HUM ??? */
		}
		break;
	case SOCK_UDP:
	case SOCK_IPRAW:
	case SOCK_MACRAW:
	/* We don't care about PPPOE */
		/* Socket has been created */
		s->s_state = SS_UNCONNECTED;
		wakeup(s);
		break;
	}
}

void w5300_event(void)
{
	uint16_t irq;
	uint8_t i = 0;
	struct socket *s = sockets;


	/* Polling cases */
	irq = w5300_readw(IR) & 0xFF;
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


int net_init(struct socket *s)
{
	int i = s - sockets;
	int n;

	if (w5300 == 0) {
		udata.u_error = ENETDOWN;
		return -1;
	}
	n = net_alloc();
	if (n < 0) {
		udata.u_error = ENOENT;//FIXME ENOBUFS
		return -1;
	}
	wiz2sock_map[n] = i;
	sock2wiz_map[i] = n;
	s->s_state = SS_UNCONNECTED;
	return 0;
}

/* Bind a socket to an address. May block */
int net_bind(struct socket *s)
{
	uint16_t i = sock2wiz_map[s - sockets];
	uint8_t r = SOCK_INIT;
	uint16_t off = SOCKOFF(i);

	/* FIXME: review for 5300 type codes etc */
	switch (s->s_type) {
	case SOCKTYPE_TCP:
		/* We keep ports net endian so don't byte swap */
		w5300_writew(Sn_MR + off, 0x01);	/* TCP, delayed ack */
		w5300_writebe(Sn_PORTR + off, s->s_addr[SADDR_SRC].port);
		break;
	case SOCKTYPE_UDP:
		w5300_writew(Sn_MR + off, 0x02);	/* UDP */
		w5300_writebe(Sn_PORTR + off, s->s_addr[SADDR_SRC].port);
		r = SOCK_UDP;
		break;
	case SOCKTYPE_RAW:
		/* Hackish */
		w5300_writebe(Sn_MR + off, 0x03);	/* RAW */
		w5300_writew(Sn_PORTR + off, s->s_addr[SADDR_SRC].port);
		r = SOCK_IPRAW;
	}
	/* Make an open request to open the socket */
	w5300_writew(Sn_CR + off, OPEN);

	/* If the reply is not immediately SOCK_INIT we failed */
	if (w5300_readw(Sn_SSR + off) != r) {
		udata.u_error = EADDRINUSE;	/* Something broke ? */
		return -1;
	}
	/* Interrupt on if available mark as bound */
	irqmask |= 1 << i;
	w5300_writew(IMR, irqmask);
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
	uint16_t i = sock2wiz_map[s - sockets];

	i = SOCKOFF(i);

	/* Issue a listen command. Check the state went to SOCK_LISTEN */
	w5300_writew(Sn_CR + i, LISTEN);
	if (w5300_readw(Sn_SSR + i) != SOCK_LISTEN) {
		udata.u_error = EIO;//FIXME EPROTO;	/* ??? */
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
		i = SOCKOFF(sock2wiz_map[s - sockets]);
		/* Already net endian */
		w5300_writebe(Sn_DIPR + i, s->s_addr[SADDR_DST].addr >> 16);
		w5300_writebe(Sn_DIPR + 2 + i, s->s_addr[SADDR_DST].addr);
		w5300_writebe(Sn_DPORTR + i, s->s_addr[SADDR_DST].port);
		w5300_writew(Sn_CR + i, CONNECT);
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
	uint16_t i = sock2wiz_map[s - sockets];
	uint16_t off = SOCKOFF(i);

	if (s->s_type == SOCKTYPE_TCP && s->s_state != SS_CLOSED) {
		w5300_writew(Sn_CR + off, DISCON);
		s->s_state = SS_CLOSING;
	} else {
		irqmask &= ~(1 << i);
		w5300_writew(IMR, irqmask);
		w5300_writew(Sn_CR + off, CLOSE);
		wiz2sock_map[i] = 0xFF;
		sock_closed(s);
	}
}

arg_t net_read(struct socket *s, uint8_t flag)
{
	uint16_t n = 0xFFFF;
	uint16_t r;
	uint16_t i = SOCKOFF(sock2wiz_map[s - sockets]);
	uint8_t st;

	/* FIXME: IRQ protection */
	/* Wait for data - push int core code ? */
	while (1) {
		/* See if we have lost the link */
		if (s->s_state < SS_CONNECTED) {
			udata.u_error = EINVAL;
			return -1;
		}
		/* Check for an EOF (covers post close cases too) */
		if (s->s_iflag & SI_EOF)
			return 0;
		/* Keep waiting until we get the right state */
		/* Bytes available */
		n = w5300_readw(Sn_RX_FIFOR + i);
		if (n) {
			s->s_iflag |= SI_DATA;
			break;
		}
		st = w5300_readw(Sn_SSR);
		if (st >= SOCK_CLOSING && st <= SOCK_UDP)
			return 0;
		/* Need IRQ protection to avoid sleep race */
		if (psleep_flags(&s->s_iflag, flag))
			return -1;
	}
	switch (s->s_type) {
	case SOCKTYPE_RAW:
	case SOCKTYPE_UDP:
		/* UDP comes with a header */
		w5300_dequeue(i, 4, (uint8_t *) & s->s_addr[SADDR_TMP].addr);
		if (s->s_type == SOCKTYPE_UDP)
			w5300_dequeue(i, 2,
				    (uint8_t *) & s->s_addr[SADDR_TMP].
				    port);
		w5300_dequeue(i, 2, (uint8_t *) & n);	/* Actual packet size */
		n = ntohs(n);	/* Big endian on device */
		/* Fall through */
	case SOCKTYPE_TCP:
		/* Bytes to consume */
		r = min(n, udata.u_count);
		/* Now dequeue some bytes into udata.u_base */
		w5300_dequeue_u(i, r, udata.u_base);
		/* For datagrams we always discard the entire frame */
		/* FIXME: FIFO clean if UDP etc ? */
		/* FIXME: figure out if SI_DATA should be cleared */
		/* Now tell the device we ate the data */
		w5300_writew(Sn_CR, RECV);
	}
	/* FIXME: 0 window workaround */
	if (s->s_type == SOCKTYPE_TCP && w5300_readw(Sn_RX_RSR + i) == 0) {
		w5300_writew(Sn_TX_WRSR + i, 1);
		w5300_writew(Sn_TX_FIFOR + i, 0);
		w5300_writew(Sn_CR, SEND);
		/* Will cause a send event and we'll clean up as if was real */
	}
	return r;
}

arg_t net_write(struct socket * s, uint8_t flag)
{
	uint16_t i = SOCKOFF(sock2wiz_map[s - sockets]);
	uint16_t room;
	uint16_t n = 0;
	uint8_t a = s->s_flag & SFLAG_ATMP ? SADDR_TMP : SADDR_DST;

	/* FIXME: blocking ?? */
	used(flag);

	/* Check FIFO space */
	room = w5300_readw(Sn_TX_FSR + i);

	switch (s->s_type) {
	case SOCKTYPE_UDP:
		if (udata.u_count > 1472) {
			udata.u_error = EMSGSIZE;
			return -1;
		}
	case SOCKTYPE_RAW:
		if (udata.u_count > 1500) {
			udata.u_error = EMSGSIZE;
			return -1;
		}
		if (room < udata.u_count)
			return -2;
		/* Already native endian */
		w5300_writebe(Sn_DIPR + i, s->s_addr[a].addr >> 16);
		w5300_writebe(Sn_DIPR + i, s->s_addr[a].addr);
		w5300_writebe(Sn_DPORTR + i, s->s_addr[a].port);
		/* Fall through */
	case SOCKTYPE_TCP:
		if (room == 0)
			return -2;
		n = min(room, udata.u_count);
		w5300_queue_u(i, n, udata.u_base);
		/* Write the data to the FIFO, then write the length
		   Pad odd bytes */
		/* Write length to WRSR */
		w5300_writew(Sn_TX_WRSR + i, 0);
		w5300_writew(Sn_TX_WRSR + i + 2, n);
		w5300_writew(Sn_CR, SEND);
		break;
	}
	return n;
}

arg_t net_shutdown(struct socket *s, uint8_t flag)
{
	int i = sock2wiz_map[s - sockets] << 8;
	s->s_iflag |= flag;
	if (s->s_iflag & SI_SHUTW)
		w5300_writew(Sn_CR + i, DISCON);
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
		w5300_bwrite(SIPR0, p, 4);
		break;
	case OP_SIFMASK:
		w5300_bwrite(SUBR0, p, 4);
		break;
	case OP_SIFGW:
		w5300_bwrite(GAR0, p, 4);
		break;
	case OP_GIFHWADDR:
		w5300_bread(SHAR0, p, 6);
		break;
	case OP_SIFHWADDR:
		w5300_bwrite(SHAR0, p, 6);
		break;
	case OP_GPHY:
		return (w5300_readb(PSTATUS) & 0x20) ? LINK_UP : LINK_DOWN;
	default:
		return -EINVAL;
	}
	return 0;
}
#endif
static uint32_t ipa = 0x00000000;	/* Tmp hack */
static uint16_t fakeaddr[3] = { 0xC0FF, 0xEEC0, 0xFFEE };
static uint32_t iga = 0x020000C0;
static uint32_t igm = 0x00FFFFFF;

void netdev_init(void)
{
	/* Sets up MR, mode etc */
	w5300_setup();
	/* FIXME: reset and presence check needs wiring in properly */
	if (w5300_readw(IDR) != 0x5300)
		return;

	kprintf("WizNET 5300 detected.\n");

	w5300_writew(IMR, 0);
//	w5300_writeb(RTR, );
//	w5300_writeb(RCR, );
	/* Set GAR, SHAR, SUBR, SIPR to defaults ? */
	w5300_writebe(SIPR, ipa >> 16);
	w5300_writebe(SIPR + 2, ipa);
	w5300_writebe(GAR, iga >> 16);
	w5300_writebe(GAR + 2, iga);
	w5300_writebe(SUBR, igm >> 16);
	w5300_writebe(SUBR + 2, igm);
	w5300_writew(SHAR, fakeaddr[0]);
	w5300_writew(SHAR + 2, fakeaddr[1]);
	w5300_writew(SHAR + 4, fakeaddr[2]);
	w5300 = 1;
}

#endif
