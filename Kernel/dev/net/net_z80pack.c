/*
 *	Network glue for Z80 pack emulator networking extension
 *
 *	Designed as a model for emulated and offload implementations
 */

#include "kernel.h"
#include "kdata.h"
#include "netdev.h"
#include "printf.h"
#include "net_z80pack.h"

__sfr __at 50 netstat;
__sfr __at 51 netdata;
__sfr __at 52 netctrl;

/* Wake all blockers on a socket - used for error cases */
static void wakeup_all(struct socket *s)
{
	wakeup(s);
	wakeup(&s->s_data);
	wakeup(&s->s_iflag);
}

static uint8_t errormap[] = {
        0,
        EPIPE, ETIMEDOUT, EADDRINUSE, EADDRNOTAVAIL,
        EHOSTUNREACH, ENOTCONN, ESHUTDOWN, EINPROGRESS,
        ECONNREFUSED, EPERM
};
        
static uint8_t status(struct socket *s)
{
	uint8_t st;
	netstat = s->s_num;
	st = netstat;
	if (st & 0x43) {	/* Read, write or EOF */
		if (st & 0x02) {
			if (s->s_state == SS_CONNECTING) {
				s->s_state = SS_CONNECTED;
			        wakeup(s);
                        }
			if (s->s_iflag & SI_THROTTLE) {
				s->s_iflag &= SI_THROTTLE;
				wakeup(&s->s_data);
			}
		}
		if ((st & 0x40) && (s->s_state == SS_CONNECTED)) {
			s->s_state = SS_CLOSEWAIT;
                        wakeup(s);
                }
		/* EOF also wakes a read */
		if ((st & 0x01) && !(s->s_iflag & SI_DATA)) {
			s->s_iflag |= SI_DATA;
			wakeup(&s->s_iflag);
		}
	}
	if (st & 0x80) {
		s->s_error = netctrl;
                if (s->s_error <= sizeof(errormap))
        		s->s_error = errormap[s->s_error];
                else
                        s->s_error = EINVAL;
		if (s->s_state >= SS_CONNECTING) {
		        switch(s->s_error) {
		        case EINPROGRESS:
		        case ENOTCONN:
		                s->s_error = 0;
		                break;
                        default:
                		s->s_state = SS_CLOSED;
	                	wakeup_all(s);
                        }
		}
	}
	return st;
}

static void err_xlate(struct socket *s)
{
        udata.u_error = s->s_error;
        s->s_error = 0;
}

void netz_poll(void)
{
	struct socket *s = sockets;
	while(s < sockets + NSOCKET) {
		if (s->s_state > SS_BOUND && s->s_state != SS_CLOSED)
			status(s);
		s++;
	}
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
	/* For the moment */
	if (s->s_type != SOCKTYPE_TCP) {
		/* FIXME: EPROTONOSUPPORT ? */
		udata.u_error = EAFNOSUPPORT;
		return -1;
	}
	/* For now keep the LCN matching the socket # */
	if (s->s_num < 16) {
		/* Select our new socket and turn off NDELAY bit flag */
		netstat = s->s_num | 0x20;
		/* Check allowed selection */
		if ((netstat & 0x80) == 0 || netctrl == 0x06) {
			s->s_state = SS_UNCONNECTED;
			return 0;
		}
		status(s);
		err_xlate(s);
		return -1;
	}
	udata.u_error = EBUSY;
	return -1;
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
	/* We don't currently support setting the local address so just
	   carry on */
	s->s_state = SS_BOUND;
	return 0;
}

/*
 *	A listen has been issued by the user. Inform the underlying TCP
 *	stack that it should accept connections on this socket. A stack that
 *	lacks incoming connection support can error instead
 */
int net_listen(struct socket *s)
{
        used(s);
	/* Not yet supported */
	udata.u_error = EOPNOTSUPP;
	return -1;
}

/*
 *	A connect has been issued by the user. This message tells the
 *	stack to begin connecting. It should put the socket state into
 *	SS_CONNECTING before returning, or it can error.
 */
int net_connect(struct socket *s)
{
	/* This is in host big endian order already */
	uint8_t *p = (uint8_t*)&s->s_addr[SADDR_DST].addr;
	uint8_t err;
	irqflags_t irq = di();

	/* Select our socket and reset the address pointer */
	netstat = s->s_num | 0x10;
	/* Write the address */
	netctrl = *p++;
	netctrl = *p++;
	netctrl = *p++;
	netctrl = *p;
	p = (uint8_t*)&s->s_addr[SADDR_DST].port;
	/* Write the port */
	netctrl = *p++;
	netctrl = *p;
	/* The next status query will trigger the socket set up */
	err = status(s);
	irqrestore(irq);
	if (err & 0x80) {
		/* Error pending */
		err_xlate(s);
		return -1;
	}
	/* It worked. We may have received data or eof events but that
	   is fine, we'll get them again when we ask with data */
	s->s_state = SS_CONNECTING;
	kputs("Connecting\n");
	return 0;
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
	netstat = s->s_num | 0x40;	/* Close */
	s->s_state = SS_CLOSED;
	netctrl;
}

/*
 *	Read or recvfrom a socket. We don't yet handle message addresses
 *	sensibly and that needs fixing
 */
arg_t net_read(struct socket *s, uint8_t flag)
{
	struct sockdata *sd = s->s_priv;
	irqflags_t irq;
	uint8_t st;
	volatile uint8_t data;


	while (1)  {
		irq = di();

		netstat = s->s_num;
		st = status(s);
		/* Error */
		if (st & 0xC0)
			break;
		if (s->s_state < SS_CONNECTED) {
			irqrestore(irq);
			udata.u_error = EINVAL;
			return -1;
		}
		/* We hit the EOF */
		if (s->s_state > SS_CLOSEWAIT)
			break;
		/* Check our status */
		/* Data ready */
		if (st & 0x01) {
			data = netdata;
			/* Could be an error */
			if (data == 0) {
			        st = status(s);
			        if (st & 0xC0)
        				break;
                        }
			uputc(data, udata.u_base++);
			udata.u_done++;
			if (udata.u_done < udata.u_count)
			        continue;
		}
		if (udata.u_done)
		        break;
		s->s_iflag &= ~SI_DATA;
		if (psleep_flags_io(&s->s_iflag, flag)) {
        		irqrestore(irq);
			return -1;
                }
	}
	if (udata.u_done)
		return udata.u_done;
	if (st & 0x80) {
		err_xlate(s);
		return -1;
	}
	return 0;
}

/*
 *	Write or sendto a socket. We don't yet handle message addresses
 *	sensible and that needs fixing
 */
arg_t net_write(struct socket * s, uint8_t flag)
{
	uint8_t p = 0;
	struct sockdata *sd = s->s_priv;
	irqflags_t irq;
	uint8_t st;


	irq = di();
	while (1) {

		netstat = s->s_num;
		st = status(s);

		if (s->s_state == SS_CLOSED || (s->s_iflag & SI_SHUTW)) {
		        udata.u_error = EPIPE;
			break;
                }

		/* Error or EOF */
		if (st & 0xC0)
			break;

		/* Good status after a write means byte ok */
		udata.u_done += p;
		if (udata.u_done == udata.u_count) {
		        irqrestore(irq);
			return udata.u_done;
                }
		/* Can we send more bytes ? */
		p = 0;

		if (st & 2) {
			/* Count bytes sent. The byte we just loaded isn't
			   sent until we check the status of it and it is
			   clean */
			p = 1;
			netdata = ugetc(udata.u_base++);
			continue;
		}
		s->s_iflag |= SI_THROTTLE;
		if (psleep_flags_io(&s->s_iflag, flag)) {
                        irqrestore(irq);
			return n;
                }
                di();
	}
	/* It broke mummy ! */
        irqrestore(irq);
	if (udata.u_done) {
	        s->s_error = udata.u_error;
	        udata.u_error = 0;
		return udata.u_done;
        }
	err_xlate(s);
	if (udata.u_error == EPIPE)
		ssig(udata.u_ptab, SIGPIPE);
	return -1;
}

arg_t net_shutdown(struct socket *s, uint8_t flag)
{
        s->s_iflag |= flag;
        wakeup_all(s);
        return 0;
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
