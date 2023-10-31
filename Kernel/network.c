#include <kernel.h>
#include <kdata.h>
#include <netdev.h>
#include <printf.h>

#ifdef CONFIG_NET

#define is_datagram(s)	((s)->s_class != SOCK_STREAM && (s)->s_class != SOCK_SEQPACKET)

struct socket sockets[NSOCKET];

/*
 *	Core logic for networking system calls. This is ultimately
 *	designed to live in a separate networking space when necessary
 */

int net_syscall(void)
{
	register struct socket *s = sockets + udata.u_net.sock;
	struct socket *n;
	int r;

	udata.u_error = 0;
	udata.u_retval = 0;

//	kprintf("cmd %d state %d\n", udata.u_net.args[0], s->s_state);
	switch (udata.u_net.args[0]) {
	case 0:		/* socket */
		return netproto_socket();
	case 1:		/* listen */
		if (s->s_state == SS_UNCONNECTED && netproto_autobind(s))
			return 0;
		if (is_datagram(s) || s->s_state != SS_BOUND) {
			udata.u_error = EINVAL;
			return 0;
		}
		return netproto_listen(s);
	case 2:		/* bind */
		if (s->s_state != SS_UNCONNECTED)
			break;
		if (netproto_find_local(&udata.u_net.addrbuf) != -1) {
			udata.u_error = EADDRINUSE;
			return 0;
		}
		netproto_bind(s);
		/* Will have set errno for any error cases or 0 if not */
		return 0;
	case 3:		/* connect */
		/* If we are connecting and it's not blocking we report EALREADY
		   if blocking we ask the caller to wait and re-call */
		if (s->s_state == SS_CONNECTING) {
			udata.u_error = EALREADY;
			return 1;
		}
		if (s->s_state == SS_UNCONNECTED) {
			if (netproto_autobind(s))
				return 0;
			s->s_state = SS_BOUND;
		}
		if (s->s_state == SS_BOUND) {
			netproto_begin_connect(s);
			return 1;
		}
		if (s->s_state == SS_CONNECTED || s->s_state == SS_CLOSEWAIT)
			return 0;
		/* The connection closed under us */
		udata.u_error = s->s_error;
		/* TODO: review if belongs in stack close path handler */
		if (udata.u_error == 0)
			udata.u_error = ECONNREFUSED;
		return 0;
	case 4:		/* accept */
		if (s->s_state != SS_LISTENING) {
			udata.u_error = EALREADY;
			return 0;
		}
		if ((n = netproto_sockpending(s)) == NULL)
			return 1;
		n->s_state = SS_CONNECTED;
		netproto_accept_complete(n);
		/* Socket back to an id */
		udata.u_net.sock = n - sockets;
		return 0;
	case 5:		/* getsockname */
		if (s->s_state == SS_UNCONNECTED)
			break;
		udata.u_net.addrlen = s->src_len;
		memcpy(&udata.u_net.addrbuf, &s->src_addr, s->src_len);
		return 0;
	case 6:		/* sendto */
		if (s->s_state == SS_UNCONNECTED && netproto_autobind(s))
			return 0;
		if (s->s_state < SS_BOUND)
			break;
		if (!is_datagram(s) || udata.u_net.args[5] == 0)
			return netproto_write(s, &s->dst_addr);
		r = netproto_write(s, &udata.u_net.addrbuf);
		udata.u_retval = udata.u_done;
		return r;
	case 7:		/* recvfrom */
		if (s->s_state == SS_UNCONNECTED && netproto_autobind(s))
			return 0;
		if (s->s_state < SS_BOUND)
			break;
		/* This will put the address into u_net.n_addr for us */
		r = netproto_read(s);
		udata.u_retval = udata.u_done;
		return r;
	case 8:		/* shutdown */
		if (udata.u_argn1 > 2)
			break;
		return netproto_shutdown(s, udata.u_net.args[2]);
	case 9:		/* getpeername */
		if (s->s_state != SS_CONNECTED) {
			udata.u_error = ENOTCONN;
			return 0;
		}
		udata.u_net.addrlen = s->dst_len;
		memcpy(&udata.u_net.addrbuf, &s->dst_addr, s->dst_len);
		return 0;
	default:;
	}
	udata.u_error = EINVAL;
	return 0;
}

int net_close(void)
{
	struct socket *s = sockets + udata.u_net.sock;
	netproto_close(s);
	return 0;
}

int net_write(void)
{
	int r;
	register struct socket *s = sockets + udata.u_net.sock;
	if (s->s_state != SS_CONNECTED && s->s_state != SS_CLOSING) {
		if (s->s_state >= SS_CLOSEWAIT) {
			udata.u_net.sig = SIGPIPE;
			udata.u_error = EPIPE;
		} else
			udata.u_error = EINVAL;
		return 0;
	}
	r = netproto_write(s, &s->src_addr);
	udata.u_retval = udata.u_done;
	return r;
}

int net_read(void)
{
	int r;
	register struct socket *s = sockets + udata.u_net.sock;
	if (s->s_error) {
		udata.u_error = s->s_error;
		s->s_error = 0;
		return 0;
	}
	/* Q: datagram read in SS_BOUND valid but stream not ? TODO */
	if (s->s_state < SS_CONNECTED) {
		udata.u_error = EINVAL;
		return 0;
	}
	r = netproto_read(s);
	udata.u_retval = udata.u_done;
	return r;
}

int net_ioctl(int req, char *data)	/* data is user */
{
	struct socket *s = sockets + udata.u_net.sock;
	/* Nothing but devices at this point */
	return netproto_ioctl(s, req, data);
}

void net_free(void)
{
	netproto_free(sockets + udata.u_net.sock);
}

/*
 *	Helpers
 */

void net_setup(struct socket *sp)
{
	register struct socket *s = sp;	/* TODO: clean up when compiler gets reg arg */
	s->s_state = SS_INIT;
	s->s_iflags = 0;
	s->s_parent = 0xFF;
	s->s_error = 0;
	s->src_len = 0;
	s->dst_len = 0;
	s->s_ino = NULL;
	netproto_setup(s);
}

void net_inode(void)
{
	struct socket *s = sockets + udata.u_net.sock;
	s->s_ino = udata.u_net.inode;
}

#endif
