/*
 *	WizNET stack driver
 *
 *	TODO
 *	- Test and debug accept and incoming socket paths
 *	- Pull socket_wake out somewhere sensible
 *	- Look at filtering limitations
 *	- Setsockopt in the core and into drivers for MTU etc
 *	- Test ifconfig interface
 */

#include <kernel.h>

#if defined(CONFIG_NET_WIZNET) && !defined(CONFIG_NET_W5300)

#include <kdata.h>
#include <printf.h>
#include <netdev.h>
#include <net_w5x00.h>


/* For now until we work out where this really belongs */
uint8_t sock_wake[NSOCKET];

static uint_fast8_t irqmask;

static uint_fast8_t wiznet_present;

#ifdef CONFIG_NET_W5100

#define HWNAME	"5100"

#define WIZ_SOCKET	4

/* Offset conversions: in the W5100 into a linear 16bit space */
#define SOCK2BANK_C(x)	(0x0000 + ((x) << 8))
#define SOCK2BANK_W(x)	(0x4000 + ((x) << 11))
#define SOCK2BANK_R(x)	(0x6000 + ((x) << 11))
#define BANK_CTRL	0

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

#define W5100_TCP	0x21
#define W5100_UDP	0x02
#define W5100_RAW	0x03

/* On the 5100 part the interrupt register low bits are what we need */
#define SIR		IR
#define SIMR		IMR
#define SIR_MASK	0x0F

#elif defined(CONFIG_NET_W5200)

/* The 5200 is quite similar to the 5100 except that everything has moved
   around the memory map */

#define HWNAME	"5200"

#define WIZ_SOCKET	8

/* Offset conversions: in the W5200 into a linear 16bit space */
#define SOCK2BANK_C(x)	(0x0000 + ((x) << 8))
#define SOCK2BANK_W(x)	(0x8000 + ((x) << 11))
#define SOCK2BANK_R(x)	(0xC000 + ((x) << 11))
#define BANK_CTRL	0

static uint8_t sock_free = 0xFF;
static uint8_t wiz2sock_map[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

#define RX_MASK 	0x07FF
#define TX_MASK		0x07FF

#define MR		0x0000
#define		MR_RESET	0x80
#define		MR_WOL		0x20
#define		MR_PB		0x10
#define		MR_PPPOE	0x08

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
#define PPPALGO		0x001E
#define VERSIONR	0x001F
#define 	VERSION_ID	0x03
#define PTIMER		0x0028
#define PMAGIC		0x0029
#define UIPR0		0x002A
#define UPORT0		0x002E
#define INTLEVEL0	0x0030
#define INTLEVEL1	0x0031
#define IR2		0x0034
#define PSTATUS		0x0035
#define IMR2		0x0036

#define Sn_MR		0x4000
#define Sn_CR		0x4001
#define 	OPEN		0x01
#define		LISTEN		0x02
#define		CONNECT		0x04
#define		DISCON		0x08
#define		CLOSE		0x10
#define		SEND		0x20
#define		SEND_MAC	0x21
#define		SEND_KEEP	0x22
#define		RECV		0x40
#define	Sn_IR		0x4002
#define		I_SEND_OK	0x10
#define		I_TIMEOUT	0x08
#define		I_RECV		0x04
#define		I_DISCON	0x02
#define		I_CON		0x01
#define Sn_SR		0x4003
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
#define Sn_PORT0	0x4004
#define Sn_DHAR0	0x4006
#define Sn_DIPR0	0x400C
#define Sn_DPORT0	0x4010
#define Sn_MSSR0	0x4012
#define Sn_PROTO	0x4014
#define Sn_TOS		0x4015
#define Sn_TTL		0x4016
#define Sn_RXMEM_SIZE	0x401E
#define Sn_TXMEM_SIZE	0x401F
#define Sn_TX_FSR	0x4020
#define Sn_TX_RD0	0x4022
#define Sn_TX_WR0	0x4024
#define Sn_RX_RSR	0x4026
#define Sn_RX_RD0	0x4028
#define Sn_RX_WR0	0x402A
#define Sn_IMR		0x402C

#define W5100_TCP	0x21
#define W5100_UDP	0x02
#define W5100_RAW	0x03

/* Same function different name */
#define SIR		IR2
#define SIMR		IMR2
#define SIR_MASK	0xFF

#elif defined(CONFIG_NET_W5500)

#define HWNAME	"5500"

#define WIZ_SOCKET	8

/* The 5500 has bank information in the SPI control byte so there multiple
   real spaces */
#define _SLOT(x)	((x) << 2)
#define SOCK2BANK_C(x)	((_SLOT(x) | 1) << 3)
#define SOCK2BANK_W(x)	((_SLOT(x) | 2) << 3)
#define SOCK2BANK_R(x)	((_SLOT(x) | 3) << 3)
#define BANK_CTRL	0

static uint8_t sock_free = 0xFF;
static uint8_t wiz2sock_map[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

#define RX_MASK 	0x07FF
#define TX_MASK		0x07FF

#define MR		0x0000
#define		MR_RESET	0x80
#define		MR_WOL		0x20
#define		MR_PB		0x10
#define		MR_PPPOE	0x08
#define		MR_FARP		0x01

/* System control space 0 */
#define GAR0		0x0001
#define SUBR0		0x0005
#define SHAR0		0x0009
#define SIPR0		0x000F
#define INTLEVEL0	0x0013
#define INTLEVEL1	0x0014
/* These are different to the 5100 and only deal with special events */
#define IR		0x0015
#define IMR		0x0016
/* And these deal with the 8 sockets */
#define SIR		0x0017
#define 	SIR_MASK	0xFF
#define SIMR		0x0018
#define RTR0		0x0019
#define RTR1		0x001A
#define RCR		0x001B
#define PTIMER		0x001C
#define PMAGIC		0x001D
#define PHAR0		0x001E
#define PSID0		0x0024
#define PMRU0		0x0026
#define UIPR0		0x002A
#define UPORT0		0x002C
#define PHYCFGR		0x002E
#define 	PHY_RESET	0x80
#define		PHY_OPMD	0x40
#define		PHY_OPMDC	0x38
#define		PHY_DPX		0x04
#define		PHY_SPEED	0x02
#define		PHY_LINK	0x01
#define VERSIONR	0x0039
#define 	VERSION_ID	0x04

/* Socket registers live in a different space */
#define Sn_MR		0x0000
#define Sn_CR		0x0001
#define 	OPEN		0x01
#define		LISTEN		0x02
#define		CONNECT		0x04
#define		DISCON		0x08
#define		CLOSE		0x10
#define		SEND		0x20
#define		SEND_MAC	0x21
#define		SEND_KEEP	0x22
#define		RECV		0x40
#define	Sn_IR		0x0002
#define		I_SEND_OK	0x10
#define		I_TIMEOUT	0x08
#define		I_RECV		0x04
#define		I_DISCON	0x02
#define		I_CON		0x01
#define Sn_SR		0x0003
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
#define Sn_PORT0	0x0004
#define Sn_DHAR0	0x0006
#define Sn_DIPR0	0x000C
#define Sn_DPORT0	0x0010
#define Sn_MSSR0	0x0012
//#define Sn_PROTO	0x0014
#define Sn_TOS		0x0015
#define Sn_TTL		0x0016
#define Sn_RXBUF_SIZE	0x001E
#define Sn_TXBUF_SIZE	0x001F
#define Sn_TX_FSR	0x0020
#define Sn_TX_RD0	0x0022
#define Sn_TX_WR0	0x0024
#define Sn_RX_RSR	0x0026
#define Sn_RX_RD0	0x0028
#define Sn_RX_WR0	0x002A
#define Sn_IMR		0x002C
#define Sn_FRAG0	0x002D
#define Sn_KPLAVTR	0x002F

#define W5100_TCP	0x01
#define W5100_UDP	0x02
#define W5100_RAW	0x03

#else
#error "Unknown interface type"
#endif



/* FIXME: look for ways to fold these four together */
static void w5x00_queue(uint16_t i, uint16_t n, void *p)
{
	uint16_t dm = w5x00_readsw(i, Sn_TX_WR0) & TX_MASK;
	i = SOCK2BANK_W(i);
	if (dm + n > TX_MASK) {
		uint16_t us = TX_MASK + 1 - dm;
		w5x00_bwrite(i, dm, p, us);
		w5x00_bwrite(i, 0, (uint8_t *)p + us, n - us);
	} else
		w5x00_bwrite(i, dm, p, n);
}

static void w5x00_queue_u(uint16_t i, uint16_t n, void *p)
{
	uint16_t dm = w5x00_readsw(i, Sn_TX_WR0) & TX_MASK;
	i = SOCK2BANK_W(i);

	if (dm + n > TX_MASK) {
		uint16_t us = TX_MASK + 1 - dm;
		w5x00_bwriteu(i, dm, p, us);
		w5x00_bwriteu(i, 0, (uint8_t *)p + us, n - us);
	} else
		w5x00_bwriteu(i, dm, p, n);
}

static void w5x00_read(uint16_t i, uint16_t off, uint16_t n, void *p)
{
	i = SOCK2BANK_R(i);

	if (off + n > RX_MASK) {
		uint16_t us = RX_MASK + 1 - off;
		w5x00_bread(i, off, p, us);
		w5x00_bread(i, 0, (uint8_t *)p + us, n - us);
	} else
		w5x00_bread(i, off, p, n);
}

static void w5x00_read_u(uint16_t i, uint16_t off, uint16_t n, void *p)
{
	i = SOCK2BANK_R(i);

	if (off + n > RX_MASK) {
		uint16_t us = RX_MASK + 1 - off;
		w5x00_breadu(i, off, p, us);
		w5x00_breadu(i, 0, (uint8_t *)p + us, n - us);
	} else
		w5x00_breadu(i, off, p, n);
}

static void w5x00_cmd(uint8_t s, uint8_t v)
{
	w5x00_writesb(s, Sn_CR, v);
	while(w5x00_readsb(s, Sn_CR));
}

/* Must only be called by the IRQ path */
static void w5x00_eof(struct socket *s)
{
	s->s_iflags |= SI_EOF;
	s->s_wake = 1;
}

/* State management for creation of a socket. */
static int net_alloc(void)
{
	register uint_fast8_t i = 0;
	register uint_fast8_t j = 1;
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
			s = sockets + n;
			/* Some of this belong sin core code ? */
			s->s_num = n;
			s->s_parent = 0xFF;
			s->proto.slot = i;
			wiz2sock_map[i] = n;
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
	w5x00_writecb(IMR, irqmask);
	w5x00_cmd(i, CLOSE);
	s->s_state = SS_UNUSED;
	netproto_free(s);
}

/* Bind a socket to an address. */
/* We arrange the internal types we use to match the chip
   - 21 TCP 02 UDP 03 RAW

   The higher level stuff is done further done in netproto_bind, this
   function merely handles the poking of the device */
static int do_netproto_bind(struct socket *s)
{
	uint16_t i = s->proto.slot;
	uint8_t r = SOCK_INIT;

	w5x00_writesb(i, Sn_MR, s->s_type);
	/* Make an open request to open the socket */
	w5x00_cmd(i, OPEN);
	switch (s->s_type) {
	case W5100_UDP:
		r = SOCK_UDP;
	case W5100_TCP:
		/* We keep ports net endian so don't byte swap */
		w5x00_bwrite(SOCK2BANK_C(i), Sn_PORT0, &s->src_addr.sa.sin.sin_port, 2);
		break;
#ifdef CONFIG_NET_W5100		
	case W5100_RAW:
		w5x00_writesb(i, Sn_PROTO, s->s_protocol);
		r = SOCK_IPRAW;
#endif		
	}
	/* If the reply is not immediately SOCK_INIT we failed */
	if (w5x00_readsb(i, Sn_SR) != r) {
		udata.u_error = EADDRINUSE;	/* Something broke ? */
		return -1;
	}
	/* Interrupt on if available mark as bound */
	irqmask |= 1 << i;
	w5x00_writecb(SIMR, irqmask);
	s->s_state = SS_BOUND;
	s->src_len = sizeof(struct sockaddr_in);
	return 0;
}

/*
 *	Process interrupts from the WizNet device
 */
static void w5x00_event_s(uint8_t i)
{
	uint_fast8_t sn = wiz2sock_map[i];
	register struct socket *s;
	register uint16_t stat = w5x00_readsw(i, Sn_IR);	/* BE read of reg pair */

	/* We got a pending event for a dead socket as we killed it. Shoot it
	   again to make sure it's dead */
	if (sn == 0xFF) {
		w5x00_cmd(i, CLOSE);
		irqmask &= ~(1 << i);
		w5x00_writecb(SIMR, irqmask);
		return;
	}
//	kprintf("sock %d slot %d event %x\n",
//		sn, i, stat);
	s = sockets + sn;

	if (stat & 0x1000) {
		/* Transmit completed: window re-open. We can allow more
		   data to flow from the user */
		s->s_iflags &= ~SI_THROTTLE;
		s->s_wake = 1;
	}
	if (stat & 0x800) {
		/* Timeout */
		s->s_error = ETIMEDOUT;
		w5x00_cmd(i, CLOSE);
		s->s_wake = 1;
		w5x00_eof(s);
		/* Fall through and let CLOSE state processing do the work */
	}
	if (stat & 0x400) {
		/* Receive wake: Poke the user in case they are reading */
		s->s_iflags |= SI_DATA;
		s->s_wake = 1;
	}
	if (stat & 0x200) {
		w5x00_eof(s);
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
	w5x00_writesb(i, Sn_IR, stat >> 8);

	switch ((uint8_t)stat) {
	case 0:		/* SOCK_CLOSED */
		if (s->s_state != SS_CLOSED && s->s_state != SS_UNUSED) {
			if (s->s_state != SS_CLOSING)
				s->s_error = ECONNRESET;	/* Sort of a guess */
			s->s_wake = 1;
			w5x00_eof(s);
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
				w5x00_cmd(i, CLOSE);
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
		w5x00_eof(s);
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

void w5x00_event(void)
{
	register uint_fast8_t irq;
	uint_fast8_t i = 0;
	register struct socket *s = sockets;

	/* Polling cases */
	irq = w5x00_readcb(SIR) & SIR_MASK;
	if (irq == 0)
		return;

	while (irq) {
		if (irq & 1)
			w5x00_event_s(i);
		irq >>= 1;
		i++;
		s++;
	}
}

void w5x00_poll(void)
{
	if (irqmask)
		w5x00_event();
}

struct socktype {
	uint8_t family;
	uint8_t type;
	uint8_t protocol;
	uint8_t info;
};

static struct socktype socktype[4] = {
	{ AF_INET, SOCK_STREAM, IPPROTO_TCP, W5100_TCP },
	{ AF_INET, SOCK_DGRAM, IPPROTO_UDP, W5100_UDP },
#ifdef CONFIG_NET_W5100	
	{ AF_INET, SOCK_RAW, 0, W5100_RAW },
#endif	
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
	w5x00_cmd(s->proto.slot, LISTEN);
	if (w5x00_readsb(s->proto.slot, Sn_SR) != SOCK_LISTEN) {
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
	if (s->s_type == W5100_TCP) {
		uint16_t i = s->proto.slot;
		/* Already net endian */
		w5x00_bwrite(SOCK2BANK_C(i), Sn_DIPR0, &udata.u_net.addrbuf.sa.sin.sin_addr, 4);
		w5x00_bwrite(SOCK2BANK_C(i), Sn_DPORT0, &udata.u_net.addrbuf.sa.sin.sin_port, 2);
		w5x00_cmd(i, CONNECT);
		s->s_state = SS_CONNECTING;
		s->dst_len = sizeof(struct sockaddr_in);
		return 1;
	} else {
		/* UDP/RAW - note have to do our own filtering for 'connect' */
		memcpy(&s->src_addr, &udata.u_net.addrbuf, sizeof(struct ksockaddr));
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
	if (s->s_type == W5100_TCP && s->s_state >= SS_CONNECTING && s->s_state <= SS_CONNECTED) {
		w5x00_cmd(s->proto.slot, DISCON);
		s->s_state = SS_CLOSING;
	} else
		netproto_cleanup(s);
	return 0;
}

/* We must avoid racing an interrupt handler when maninpulating the flags */
static void set_iflags(struct socket *s, uint8_t flags)
{
	irqflags_t irq = di();
	s->s_iflags |= flags;
	irqrestore(irq);
}

int netproto_read(struct socket *s)
{
	uint16_t i = s->proto.slot;
	uint16_t n;
	register uint16_t r;
	uint_fast8_t filtered;
	uint16_t off;

	if (udata.u_count == 0)
		return 0;

	/* Bytes available */
	/* FIXME: set SI_DATA on CONNECTED state or UDP connect etc and
	   check higher up without this check here */
	do {
		filtered = 0;
		n = w5x00_readsw(i, Sn_RX_RSR);
		if (n == 0) {
			if (s->s_iflags & SI_EOF)
				return 0;
			return 1;
		}
		set_iflags(s, SI_DATA);

		memcpy(&udata.u_net.addrbuf, &s->dst_addr, sizeof(struct ksockaddr));

		off = w5x00_readsw(i, Sn_RX_RD0) & RX_MASK;

		switch (s->s_type) {
		case W5100_RAW:
		case W5100_UDP:
		/* UDP comes with a header */
			w5x00_read(i, off, 4, &udata.u_net.addrbuf.sa.sin.sin_addr);
			off +=4;
			if (s->s_type == W5100_UDP) {
				w5x00_read(i, off, 2, &udata.u_net.addrbuf.sa.sin.sin_port);
				off +=2;
				if (s->src_addr.sa.sin.sin_addr.s_addr && s->src_addr.sa.sin.sin_addr.s_addr !=
					udata.u_net.addrbuf.sa.sin.sin_addr.s_addr)
					filtered = 1;
			}
			w5x00_read(i, off, 2, (uint8_t *) &n);	/* Actual packet size */
			off +=2;
			n = ntohs(n);	/* Big endian on device */
			/* Fall through */
		case W5100_TCP:
			/* Bytes to consume */
			r = min(n, udata.u_count);
			if (!filtered) {
				if (r == 0) {
					/* Check for an EOF (covers post close cases too) */
					if (s->s_iflags & SI_EOF)
						return 0;
					/* Wait */
					return 1;
				}
				/* Now read some bytes into udata.u_base */
				w5x00_read_u(i, off, r, udata.u_base);
				udata.u_done += r;
				udata.u_base += r;
			}
			/* For datagrams we always discard the entire frame */
			if (s->s_type == W5100_UDP)
				r = n + 8;
			w5x00_writesw(i, Sn_RX_RD0, w5x00_readsw(i, Sn_RX_RD0) + r);
			/* FIXME: figure out if SI_DATA should be cleared */
			/* Now tell the device we ate the data */
			w5x00_cmd(i, RECV);
		}
	} while(filtered);
	return 0;
}


arg_t netproto_write(struct socket *s, struct ksockaddr *ka)
{
	register uint16_t i = s->proto.slot;
	uint16_t room;
	uint16_t n = 0;

	room = w5x00_readsw(i, Sn_TX_FSR);

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
		w5x00_bwrite(SOCK2BANK_C(i), Sn_DIPR0, &ka->sa.sin.sin_addr, 4);
		w5x00_bwrite(SOCK2BANK_C(i), Sn_DPORT0, &ka->sa.sin.sin_port, 2);
		/* Fall through */
	case W5100_TCP:
		/* No room blocks, a partial write returns */
		if (room == 0) {
			set_iflags(s, SI_THROTTLE);
			return 1;
		}
		n = min(room, udata.u_count);
		w5x00_queue_u(i, n, udata.u_base);
		w5x00_writesw(i, Sn_TX_WR0, 
			     w5x00_readsw(i, Sn_TX_WR0) + n);
		w5x00_cmd(i, SEND);
		break;
	}
	udata.u_done += n;
	udata.u_base += n;
	return 0;
}

arg_t netproto_shutdown(struct socket *s, uint8_t flag)
{
	set_iflags(s, flag);
	if (s->s_iflags & SI_SHUTW) {
		/* FIXME: handle SI_SHUTW in write path */
		w5x00_cmd(s->proto.slot, DISCON);
	}
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
	w5x00_bwrite(BANK_CTRL, SIPR0, &ipa, 4);
	w5x00_bwrite(BANK_CTRL, GAR0, &iga, 4);
	w5x00_bwrite(BANK_CTRL, SUBR0, &igm, 4);
	w5x00_bwrite(BANK_CTRL, SHAR0, fakeaddr, 6);
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
		if (w5x00_readcb(PSTATUS) & 0x20)
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
	case SIOCGIFGWADDR:
		ifr.ifr_gwaddr.sa.sin.sin_addr.s_addr = iga;
		goto copy_addr;
	case SIOCGIFNETMASK:
		ifr.ifr_netmask.sa.sin.sin_addr.s_addr = igm;
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
	case SIOCSIFGWADDR:
		iga = ifr.ifr_gwaddr.sa.sin.sin_addr.s_addr;
		break;
	case SIOCSIFNETMASK:
		igm = ifr.ifr_netmask.sa.sin.sin_addr.s_addr;
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

	w5x00_setup();	

#ifdef VERSIONR
	if (w5x00_readcb(VERSIONR) == VERSION_ID) {
		kputs("Wiznet "HWNAME" detected.\n");
		wiznet_present = 1;
	}
#else
	w5x00_writecb(GAR0, 0x55);
	if (w5x00_readcb(GAR0) == 0x55)
		wiznet_present = 1;
#endif

	w5x00_writecb(IMR, 0);
	/* Set GAR, SHAR, SUBR, SIPR to defaults ? */
	netdev_reload();
#ifdef CONFIG_NET_W5100
	w5x00_writecb(RMSR, 0x55);	/* 2k a socket */
	w5x00_writecb(TMSR, 0x55);	/* 2k a socket */
#endif
}

/* We probably want a generic ipv4 helper layer for some of this */
int netproto_find_local(struct ksockaddr *ka)
{
	register struct socket *s = sockets;
	uint_fast8_t n = 0;
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
	register uint16_t port = ntohs(autoport);
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
