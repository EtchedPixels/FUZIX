#ifndef _DEV_NET_NET_NATIVE_H

struct sockdata {
	uint8_t err;
	uint8_t ret;
	uint8_t event;		/* Waiting events to go to user space */
#define NEVW_STATE	1
#define NEVW_READ	2
#define NEVW_WRITE	4
#define NEVW_MASK	7
#define NEVW_STATEW	128
	uint8_t newstate;	/* Requested new state */
	uint16_t rlen[NSOCKBUF];	/* TCP uses 0 as total space */
	uint8_t rbuf;
	uint8_t rnext;
	uint16_t tlen[NSOCKBUF];	/* Not used by TCP */
	uint16_t tbuf;		/* Next transmit buffer (pointer for tcp) */
	uint16_t tnext;		/* Buffers of room (bytes if TCP) */
};

struct sockmsg {
	struct socket s;
	struct sockdata sd;
};

#define NE_NEWSTATE	1
#define NE_EVENT	2
#define NE_SETADDR	3
#define NE_INIT		4
#define NE_ROOM		5
#define NE_DATA		6

/* These are by socket and each one is

   [RX.0][RX.1]..[RX.n][TX.0][TX.1]...[TX.n]
   
*/

#define NSOCKBUF	4	/* 4 buffers per socket */
#define TXBUFSIZE	1024
#define RXBUFSIZE	1024

#define SOCKBUFOFF	(RXBUFOFF + RXBUFSIZ)
#define RXBUFOFF	TXBUFSIZ

/* Total size is thus 8K * sockets - typically 64K for the file */



#endif