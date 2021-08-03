#ifndef _NET_IF_H

#include <sys/ioctl.h>

#define IFNAMSIZ	8
/*
 * Interfaces for network configuration
 */

struct sockaddr_hw {
  uint16_t shw_family;
  uint8_t shw_addr[14];
};

struct ifreq {
	char ifr_name[IFNAMSIZ];
	union {
		struct sockaddr ifr_addr;
		struct sockaddr ifr_dstaddr;
		struct sockaddr ifr_broadaddr;
		struct sockaddr ifr_netmask;
		struct sockaddr ifr_gwaddr;
		struct sockaddr ifr_hwaddr;
		short ifr_flags;
		int ifr_ifindex;
		int ifr_mtu;
	} ifr_ifru;
};

#define ifr_addr	ifr_ifru.ifr_addr
#define ifr_dstaddr	ifr_ifru.ifr_dstaddr
#define ifr_broadaddr	ifr_ifru.ifr_broadaddr
#define ifr_netmask	ifr_ifru.ifr_netmask
#define ifr_hwaddr	ifr_ifru.ifr_hwaddr
#define ifr_gwaddr	ifr_ifru.ifr_gwaddr
#define ifr_flags	ifr_ifru.ifr_flags
#define ifr_ifindex	ifr_ifru.ifr_ifindex
#define ifr_mtu		ifr_ifru.ifr_mtu

#define IFF_UP		0x0001
#define IFF_BROADCAST	0x0002
#define IFF_LOOPBACK	0x0004
#define IFF_POINTOPOINT	0x0008
#define IFF_RUNNING	0x0010
#define IFF_NOARP	0x0020
#define IFF_PROMISC	0x0040
#define IFF_MULTICAST	0x0080
#define IFF_LINKUP	0x0100

#define SIOCGIFNAME	0x0400
#define SIOCGIFINDEX	0x0401
#define SIOCGIFFLAGS	0x0402
#define SIOCSIFFLAGS	(0x0403|__IOCTL_SUPER)
#define SIOCGIFADDR	0x0404
#define SIOCSIFADDR	(0x0405|__IOCTL_SUPER)
#define SIOCGIFDSTADDR	0x0406
#define SIOCSIFDSTADDR	(0x0407|__IOCTL_SUPER)
#define SIOCGIFBRDADDR	0x0408
#define SIOCSIFBRDADDR	(0x0409|__IOCTL_SUPER)
#define SIOCGIFNETMASK	0x040A
#define SIOCSIFNETMASK	(0x040B|__IOCTL_SUPER)
#define SIOCGIFHWADDR	0x040C
#define SIOCSIFHWADDR	(0x040D|__IOCTL_SUPER)
#define SIOCGIFMTU	0x040E
#define SIOCSIFMTU	(0x040F|__IOCTL_SUPER)
#define SIOCGIFGWADDR	0x0410
#define SIOCSIFGWADDR	(0x0411|__IOCTL_SUPER)

#define HW_NONE		0	/* e.g. loopback */
#define HW_ETH		1
#define HW_WLAN		2
#define HW_SLIP		3
#define HW_CSLIP	4

#endif
