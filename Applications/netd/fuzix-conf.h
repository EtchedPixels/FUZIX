#include <stdint.h>
#include <sys/netdev.h>
#include <sys/net_native.h>

#define UIP_CONF_STATISTICS 0
#define CCIF

typedef uint8_t uip_tcp_appstate_t;
typedef uint8_t uip_udp_appstate_t;
typedef uint8_t uip_raw_appstate_t;
typedef uint8_t uip_stats_t;

#define UIP_APPCALL netd_appcall
#define UIP_UDP_APPCALL netd_udp_appcall
#define UIP_RAW_APPCALL netd_raw_appcall

#define UIP_CONF_LLH_LEN 14


#if defined(NETD_LITTLE_ENDIAN)
#define UIP_CONF_BYTE_ORDER	UIP_LITTLE_ENDIAN
#elif defined(NETD_BIG_ENDIAN)
#define UIP_CONF_BYTE_ORDER     UIP_BIG_ENDIAN
#else
#error "Must -D the correct endianness"
#endif

#define UIP_CONF_ACTIVE_OPEN 1

#define UIP_CONF_RECEIVE_WINDOW 1500

#define UIP_CONF_TCP_MSS	(mtu - 40)
#define UIP_RAW 1
#define UIP_RAW_CONNS 8

#define MIN(n, m)   (((n) < (m)) ? (n) : (m))

/* gcc can do struct assignment, the others cannot */

#ifndef __GNUC__
#define uip_ipaddr_copy(dest,src)	memcpy((dest),(src), sizeof(uip_ipaddr_t))
#define uip_ip4addr_copy(dest,src)	memcpy((dest),(src), sizeof(uip_ip4addr_t))
#define uip_ip6addr_copy(dest,src)	memcpy((dest),(src), sizeof(uip_ip6addr_t))
#endif

extern void netd_appcall(void);
extern void netd_udp_appcall(void);
extern void netd_raw_appcall(void);

extern uint16_t mtu;
