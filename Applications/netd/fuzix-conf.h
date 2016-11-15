#include <stdint.h>
#include <sys/netdev.h>
#include <sys/net_native.h>

#define UIP_CONF_STATISTICS 0
#define CCIF

typedef uint8_t uip_tcp_appstate_t;
typedef uint8_t uip_udp_appstate_t;
typedef uint8_t uip_stats_t;

#define UIP_APPCALL netd_appcall
#define UIP_UDP_APPCALL netd_udp_appcall

#define UIP_CONF_LLH_LEN 14

#define UIP_CONF_BYTE_ORDER      UIP_BIG_ENDIAN

#define UIP_CONF_ACTIVE_OPEN 1

#define UIP_CONF_RECEIVE_WINDOW 1500 

#define MIN(n, m)   (((n) < (m)) ? (n) : (m))
