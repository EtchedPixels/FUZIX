/*
 *	net fields in struct net in udata for net kernel builds
 *
 *	The networking system calls are awkward as some have a lot of
 *	arguments and we do not have a good way to do such syscalls on
 *	some platforms so intead we have as single netcall, which is
 *	passed an argument structure. Slightly slower but not really a problem
 *	and read/write are normal syscall paths anyway.
 */
#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#ifdef CONFIG_NET


#define N_MAKE		0x80
#define N_SOCKFD	0x40
#define N_ADDR_IN	0x20
#define N_ADDR_OUT	0x10
#define N_DATAIO	0x08

#define NUM_NETCALL	10

static uint8_t ncall_tab[NUM_NETCALL] = {
	N_MAKE,			/* Socket */
	N_SOCKFD,		/* Listen fd, num */
	N_SOCKFD | N_ADDR_IN,	/* bind */
	N_SOCKFD | N_ADDR_IN,	/* connect */
	N_MAKE | N_SOCKFD | N_ADDR_OUT,	/* accept */
	N_SOCKFD | N_ADDR_OUT,	/* getsockname */
	N_SOCKFD | N_DATAIO | N_ADDR_IN,	/* sendto */
	N_SOCKFD | N_DATAIO | N_ADDR_OUT,	/* recvfrom */
	N_SOCKFD,		/* shutdown */
	N_SOCKFD | N_ADDR_OUT	/* getpeername */
};

#define IN2SOCK(ino)		((ino)->c_node.i_addr[0])

bool issocket(inoptr ino)
{
	if ((ino->c_node.i_mode & F_MASK) == F_SOCK)
		return 1;
	return 0;
}

static inoptr sock_get(int fd, uint8_t * flags)
{
	inoptr ino = getinode(fd);
	if (ino == NULL)
		return NULL;
	if (!issocket(ino)) {
		udata.u_error = EINVAL;
		return NULL;
	}
	*flags = of_tab[udata.u_files[fd]].o_access;
	udata.u_net.inode = ino;
	udata.u_net.sock = IN2SOCK(ino);
	return ino;
}

/* The network layer made us a socket. We just need to nail an inode to it */
static int make_socket(uint16_t sock)
{
	int8_t uindex;
	int8_t oftindex;
	inoptr ino;

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
	ino->c_readers = 1;
	ino->c_writers = 1;

	/* Do we need a reverse lookup ? */
	udata.u_net.sock = sock;
	udata.u_net.inode = ino;

	of_tab[oftindex].o_inode = ino;
	of_tab[oftindex].o_access = O_RDWR;
	udata.u_files[uindex] = oftindex;
	return uindex;

      noalloc:
	oft_deref(oftindex);	/* Will call i_deref! */
      nooft:
	udata.u_files[uindex] = NO_FILE;
      nosock:
	return -1;
}

#define argptr ((void *)udata.u_argn)

arg_t _netcall(void)
{
	uint8_t flags = 0;
	uint8_t cn;
	arg_t *ap = udata.u_net.args;
	uint8_t op;
	usize_t s;
	int n;
	inoptr ino = NULL;

	if (valaddr(argptr, sizeof(udata.u_net.args)) !=
	    sizeof(udata.u_net.args)) {
		udata.u_error = EFAULT;
		return -1;
	}
	uget(argptr, udata.u_net.args, sizeof(udata.u_net.args));

	cn = *ap++;
	op = ncall_tab[cn];

	if (cn >= NUM_NETCALL) {
		udata.u_error = ENOSYS;
		return -1;
	}

/*	kprintf("net_syscall %d\n", cn); */

	udata.u_done = 0;
	udata.u_error = 0;

	/* First argument is a file handle. Set up the wake pointer (inode) and
	   socket number, error if it's not a valid socket */
	if (op & N_SOCKFD) {
		/* This also sets up u_net.sock */
		ino = sock_get(*ap++, &flags);
		if (ino == NULL)
			return -1;
	}
	/* Next argument is data in or out (will need to tweak when we ever get
	   read-only memory spaces). It is an error if the buffer passed is all
	   unwriteable unless the length is 0. A length 0 socket I/O has meaning
	   in some protocols */
	if (op & N_DATAIO) {
		udata.u_base = (void *) *ap;
		udata.u_count = valaddr((void *) *ap, ap[1]);
		if (s == 0 && ap[1])
			return -1;
		ap += 2;
		udata.u_count = s;
	}
	/* Expect an address and length for an address to copy into the network
	   address buffer. It must be big enough to be meaningful but fit */
	if (op & N_ADDR_IN) {
		if (*ap) {
			udata.u_net.addrlen = ap[1];
			if (udata.u_net.addrlen < MIN_SOCKADDR ||
			    udata.u_net.addrlen > MAX_SOCKADDR) {
				udata.u_error = EINVAL;
				return -1;
			}
			/* Put the buffer into the kernel */
			if (uget((void *) *ap, &udata.u_net.addrbuf,
			     udata.u_net.addrlen)) {
				udata.u_error = EFAULT;
				return -1;
			}
		} else
			udata.u_net.addrlen = 0;
	}
	/* Run states of the network machine. It will return > 0 for a sleep and
	   0 when done. We handle the rules for I/O interruption ourselves */
/*	kprintf("Do net call %d\n", udata.u_net.args[0]); */
	while (net_syscall()) {
/*		kprintf("wait: %d %d %d\n",
			udata.u_retval, udata.u_error, udata.u_net.sig); */
		/* Caller wants us to wait but has provided an error code
		   if non blocking */
		if (udata.u_error && (flags & O_NDELAY))
			return -1;
/*		kprintf("sleep: %x\n", ino); */
		if (psleep_flags(ino, flags) == -1)
			return -1;
/*		kprintf("loop: %d %d %d\n",
			udata.u_retval, udata.u_error, udata.u_net.sig); */
	}
/*(	kprintf("done: %d %d %d\n",
		udata.u_retval, udata.u_error, udata.u_net.sig); */
	/* If it asked us to SIGPIPE do so */
	if (udata.u_net.sig) {
		ssig(udata.u_ptab, udata.u_net.sig);
		udata.u_net.sig = 0;
	}
	if (udata.u_error == 0) {
		/* The syscall makes a new socket. Allocate it an inode and
		   make the return our file handle */
		if (op & N_MAKE) {
			n = make_socket(udata.u_net.sock);
			if (n < 0) {
				/* net.sock is set ready */
				net_free();
				return -1;
			} else
				net_inode();
			udata.u_retval = n;
		}
		/* The syscall returns an address into the user provided buffer
		   and updates the length field. */
		if (op & N_ADDR_OUT) {
			if (*ap != NULL) {
				/* Buffer size limit */
				s = ugeti((void *) ap[1]);
				/* Replace it with the actual size */
				uputi(udata.u_net.addrlen, (void *) ap[1]);
				/* Copy the buffer, oe less if truncated by size */
				if (s > udata.u_net.addrlen)
					s = udata.u_net.addrlen;
				if (uput(&udata.u_net.addrbuf, (void *) *ap, s) != s) {
					udata.u_error = EFAULT;
					return -1;
				}
			}
		}
		return udata.u_retval;
	}
	return -1;
}

#undef argptr

/*
 *	Other paths that lead to a socket access
 */

/*
 *	The read() syscall path. The basic validation was already done
 *	by the caller, along with setting up u_done etc.
 */
int sock_read(inoptr ino, uint8_t flags)
{
	udata.u_net.sock = IN2SOCK(ino);
	while (net_read()) {
		if (udata.u_error && (flags & O_NDELAY))
			return -1;
		if (psleep_flags(ino, flags) == -1)
			return -1;
	}
	return udata.u_done;
}

/*
 *	The write() syscall path. The basic validation was already done
 *	by the caller, along with setting up u_done etc.
 */
int sock_write(inoptr ino, uint8_t flags)
{
	udata.u_net.sock = IN2SOCK(ino);
	while (net_write()) {
		if (udata.u_error && (flags & O_NDELAY))
			return -1;
		if (psleep_flags(ino, flags) == -1)
			return -1;
	}
	return udata.u_done;
}

/*
 *	Final close of the socket file. This may be close but could also
  *	occur in exit and execve so should not block.
 */
int sock_close(inoptr ino)
{
	udata.u_net.sock = IN2SOCK(ino);
	net_close();
	/* And needs the final closedown to be asynchronous for the net layer
	   so be careful when we free stuff up */
	return udata.u_retval;
}

arg_t sock_ioctl(inoptr ino, int req, char *data)
{
	udata.u_net.sock = IN2SOCK(ino);
	/* For now we only support the device ioctls */
	return net_ioctl(req, data);
}

void sock_init(void)
{
	netdev_init();
}

#endif
