/* dwnet.h interface



 */
#ifndef DWNET_H
#define DWNET_H

struct link{
    uint8_t flags;          /* flags */
#define LINK_CLOSED 1
#define LINK_SHUTR  2       /* don't release our link/state until */
#define LINK_SHUTW  4       /* both are set */
#define LINK_OPEN   8	    /* TCP connected onward */
#define LINK_DEAD   16      /* No longer exists to the kernel (may be live
                               internally to uIP still */
#define LINK_UNBIND 32	    /* The kernel wants netd to confirm this link is
                               dead and will receive no more messages */
    uint8_t socketn;        /* Kernel's socket no */
    uint8_t lcn;            /* Kernel's idea of lcn ???*/
    struct uip_conn *conn;  /* uIP's idea of lcn */
    uint16_t tstart;     /* start of data in transmit ring */
    uint16_t tend;       /* end of data in transmit ring */
    uint16_t len;        /* length of data last transmitted */
    uint16_t rstart;     /* start of data in recv ring */
    uint16_t rend;       /* end of data in recv ring*/
    uint16_t port;       /* which port are we listening on? */
    /* for udp only */
    uint16_t tsize[NSOCKBUF]; /* number of bytes to xmit in buffer */
    uint16_t rsize[NSOCKBUF]; /* number of bytes to recv in buffers */
};


#if 0	// already in fuzix-conf.h
typedef uint8_t uip_tcp_appstate_t; /* index to link info */
typedef uint8_t uip_udp_appstate_t;
#endif
void netd_appcall( void );
void netd_udp_appcall( void );

extern uint8_t has_arp;

#endif
