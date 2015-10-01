#ifndef __FUZIX__NET_DOT_H
#define __FUZIX__NET_DOT_H

/* This code is based upon Linux 1.2.13 TCP/IP 
    Ross Biro,
    Fred N. van Kempen
    Mark Evans
    Corey Minyard
    Florian La Roche
    Charles Hedrick
    Linus Torvalds
    Alan Cox
    Matthew Dillon
    Arnt Gulbrandsen
*/

#define MAX_IP	1500			/* Largest frame we support */

/*
 *	Networking Functionality
 */

struct ethhdr {
  uint8_t src[6];
  uint8_t dst[6];
  uint16_t proto;
};

struct arphdr
{
  uint16_t hrd;		/* format of hardware address	*/
  uint16_t pro;		/* format of protocol address	*/
  uint8_t hln;		/* length of hardware address	*/
  uint8_t pln;		/* length of protocol address	*/
  uint16_t op;		/* ARP opcode (command)		*/

  uint8_t src_hw[6];	/* sender hardware address	*/
  uint32_t sip;		/* sender IP address		*/
  uint8_tt tha[6];	/* target hardware address	*/
  uint32_t tip;		/* target IP address		*/

};

/* ARP protocol HARDWARE identifiers. */
#define ARPHRD_NETROM	0		/* from KA9Q: NET/ROM pseudo	*/
#define ARPHRD_ETHER 	1		/* Ethernet 10Mbps		*/
#define	ARPHRD_EETHER	2		/* Experimental Ethernet	*/
#define	ARPHRD_AX25	3		/* AX.25 Level 2		*/
#define	ARPHRD_PRONET	4		/* PROnet token ring		*/
#define	ARPHRD_CHAOS	5		/* Chaosnet			*/
#define	ARPHRD_IEEE802	6		/* IEEE 802.2 Ethernet- huh?	*/
#define	ARPHRD_ARCNET	7		/* ARCnet			*/
#define	ARPHRD_APPLETLK	8		/* APPLEtalk			*/
/* Dummy types for non ARP hardware */
#define ARPHRD_SLIP	256
#define ARPHRD_CSLIP	257
#define ARPHRD_SLIP6	258
#define ARPHRD_CSLIP6	259
#define ARPHRD_RSRVD	260		/* Notional KISS type 		*/
#define ARPHRD_ADAPT	264
#define ARPHRD_PPP	512
#define ARPHRD_TUNNEL	768		/* IPIP tunnel			*/

/* ARP protocol opcodes. */
#define	ARPOP_REQUEST	1		/* ARP request			*/
#define	ARPOP_REPLY	2		/* ARP reply			*/
#define	ARPOP_RREQUEST	3		/* RARP request			*/
#define	ARPOP_RREPLY	4		/* RARP reply			*/

struct iphdr {
#if defined(LITTLE_ENDIAN_BITFIELD)
	uint8_t	ihl:4,
		version:4;
#elif defined (BIG_ENDIAN_BITFIELD)
	uint8_t	version:4,
  		ihl:4;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
	uint8_t	tos;
	uint16_t tot_len;
	uint16_t id;
	uint16_t frag_off;
	uint8_t	ttl;
	uint8_t	protocol;
	uint16_t check;
	uint32_t saddr;
	uint32_t daddr;
	/*The options start here. */
};

#define IPOPT_END	0
#define IPOPT_NOOP	1
#define IPOPT_SEC	130
#define IPOPT_LSRR	131
#define IPOPT_SSRR	137
#define IPOPT_RR	7
#define IPOPT_SID	136
#define IPOPT_TIMESTAMP	68

#define ICMP_ECHOREPLY		0	/* Echo Reply			*/
#define ICMP_DEST_UNREACH	3	/* Destination Unreachable	*/
#define ICMP_SOURCE_QUENCH	4	/* Source Quench		*/
#define ICMP_REDIRECT		5	/* Redirect (change route)	*/
#define ICMP_ECHO		8	/* Echo Request			*/
#define ICMP_TIME_EXCEEDED	11	/* Time Exceeded		*/
#define ICMP_PARAMETERPROB	12	/* Parameter Problem		*/
#define ICMP_TIMESTAMP		13	/* Timestamp Request		*/
#define ICMP_TIMESTAMPREPLY	14	/* Timestamp Reply		*/
#define ICMP_INFO_REQUEST	15	/* Information Request		*/
#define ICMP_INFO_REPLY		16	/* Information Reply		*/
#define ICMP_ADDRESS		17	/* Address Mask Request		*/
#define ICMP_ADDRESSREPLY	18	/* Address Mask Reply		*/


/* Codes for UNREACH. */
#define ICMP_NET_UNREACH	0	/* Network Unreachable		*/
#define ICMP_HOST_UNREACH	1	/* Host Unreachable		*/
#define ICMP_PROT_UNREACH	2	/* Protocol Unreachable		*/
#define ICMP_PORT_UNREACH	3	/* Port Unreachable		*/
#define ICMP_FRAG_NEEDED	4	/* Fragmentation Needed/DF set	*/
#define ICMP_SR_FAILED		5	/* Source Route failed		*/
#define ICMP_NET_UNKNOWN	6
#define ICMP_HOST_UNKNOWN	7
#define ICMP_HOST_ISOLATED	8
#define ICMP_NET_ANO		9
#define ICMP_HOST_ANO		10
#define ICMP_NET_UNR_TOS	11
#define ICMP_HOST_UNR_TOS	12

/* Codes for REDIRECT. */
#define ICMP_REDIR_NET		0	/* Redirect Net			*/
#define ICMP_REDIR_HOST		1	/* Redirect Host		*/
#define ICMP_REDIR_NETTOS	2	/* Redirect Net for TOS		*/
#define ICMP_REDIR_HOSTTOS	3	/* Redirect Host for TOS	*/

/* Codes for TIME_EXCEEDED. */
#define ICMP_EXC_TTL		0	/* TTL count exceeded		*/
#define ICMP_EXC_FRAGTIME	1	/* Fragment Reass time exceeded	*/


struct icmphdr {
  uint8_t type;
  uint8_t code;
  uint16_t checksum;
  union {
	struct {
		uint16_t id;
		uint16_t sequence;
	} echo;
	uint32_t gateway;
  } un;
};

struct tcphdr {
	uint16_t source;
	uint16_t dest;
	uint32_t seq;
	uint32_t ack_seq;
#if defined(LITTLE_ENDIAN_BITFIELD)
	uint16_t res1:4,
		doff:4,
		fin:1,
		syn:1,
		rst:1,
		psh:1,
		ack:1,
		urg:1,
		res2:2;
#elif defined(BIG_ENDIAN_BITFIELD)
	uint16_t doff:4,
		res1:4,
		res2:2,
		urg:1,
		ack:1,
		psh:1,
		rst:1,
		syn:1,
		fin:1;
#else
#error	"Adjust your <asm/byteorder.h> defines"
#endif	
	uint16_t window;
	uint16_t check;
	uint16_t urg_ptr;
};

struct udphdr {
  uint16_t source;
  uint16_t dest;
  uint16_t len;
  uint16_t check;
};

/* Standard well-defined IP protocols.  */
enum {
  IPPROTO_IP = 0,		/* Dummy protocol for TCP		*/
  IPPROTO_ICMP = 1,		/* Internet Control Message Protocol	*/
  IPPROTO_IGMP = 2,		/* Internet Gateway Management Protocol */
  IPPROTO_IPIP = 4,		/* IPIP tunnels (older KA9Q tunnels use 94) */
  IPPROTO_TCP = 6,		/* Transmission Control Protocol	*/
  IPPROTO_EGP = 8,		/* Exterior Gateway Protocol		*/
  IPPROTO_PUP = 12,		/* PUP protocol				*/
  IPPROTO_UDP = 17,		/* User Datagram Protocol		*/
  IPPROTO_IDP = 22,		/* XNS IDP protocol			*/

  IPPROTO_RAW = 255,		/* Raw IP packets			*/
  IPPROTO_MAX
};


/* Internet address. */
struct in_addr {
  uint32_t s_addr;
};

struct sockaddr_in {
  int16_t sin_family;
  uint16_t sin_port;
  struct in_addr sin_addr;
  uint8_t sin_zero[8];
};

extern int mac_queue(void);
extern int mac_queue_raw(void);
extern int loopback_queue(void);

extern struct socket *sock_find(uint16_t src, uint16_t dst);

extern void output_begin(void);
extern void output_add(void *ptr, uint16_t len);

extern int pkt_pull(void *ptr, uint16_t len);
extern uint8_t *pkt_next;
extern uint16_t pkt_len;

extern void arp_rcv(void);
extern void arp_put(struct arp *a);
extern struct arp *arp_get(struct arp *a);

struct arp {
  uint32_t addr;
  uint8_t users;
  uint8_t flags;
#define ARP_VALID	1
  uint8_t hw[6];
};

#define ARPSIZE 16

extern struct arp arptab[ARPSIZE];

extern void ip_mirror(void);
extern void ip_prepare(struct socket *s, uint16_t len);
extern int ip_output(void *pbuf, uint16_t plen, void *dbuf, uint16_t dlen);
extern void ip_rcv(void);

extern uint16_t ip_compute_csum(void *ptr, uint16_t size);
extern uint16_t ip_compute_csum_data(void *ptr, uint16_t size, void *dptr, uint16_t dsize);

extern struct iphdr iph;
extern struct iphdr iph2;
extern struct iphdr oiph;

extern void icmp_rcv(void);
extern void icmp_send_unreach(uint8_t type, uint8_t code);

extern int udp_send(struct socket *s, void *buf, uint16_t len);
extern void udp_rcv(void);

extern void tcp_rcv(void);
extern int tcp_send(struct socket *s, void *buf, uint16_t len);

#ifdef LITTLE_ENDIAN
#define LOOPBACK(x)		(((uint8_t)x)==0x7F)
#define MULTICAST(x)		((((uint8_t)x) & 0xF0) == 0xE0)
#else
#define LOOPBACK(x)		(((uint8_t)((x) >> 24)) == 0x7F)
#define MULTICAST(x)		(((uint8_t)((x) >> 28)) == 0xE)
#endif

#define SNMP(x)

#define PKT_HOST	0
#define PKT_BROADCAST	1
#define PKT_MULTICAST	2
#define PKT_OTHER	3

#endif
