#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "netdb.h"

static int sock;
static struct ifreq ifr;
static struct sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr;
static struct sockaddr_hw *shw = (struct sockaddr_hw *)&ifr.ifr_hwaddr;

static void do_ip_set(char *p, int ioc, char *op)
{
	if (p == NULL) {
		fprintf(stderr, "ifconfig: missing argument.\n");
		exit(1);
	}
	if (!inet_aton(p, &sin->sin_addr)) {
		fprintf(stderr, "ifconfig: unknown command '%s'.\n", p);
		exit(1);
	}
	sin->sin_family = AF_INET;
	if (ioctl(sock, ioc, &ifr)) {
		fprintf(stderr, "ifconfig: unable to set ");
		perror(op);
		exit(1);
	}
}

static void do_ip_addr_set(char *p)
{
	do_ip_set(p, SIOCSIFADDR, "address");
}

static void do_ip_broad_set(char *p)
{
	do_ip_set(p, SIOCSIFBRDADDR, "broadcast");
}

static void do_ip_mask_set(char *p)
{
	do_ip_set(p, SIOCSIFNETMASK, "netmask");
}

static void do_ip_dst_set(char *p)
{
	do_ip_set(p, SIOCSIFDSTADDR, "destination");
}

static void do_ip_gw_set(char *p)
{
	do_ip_set(p, SIOCSIFGWADDR, "gateway");
}

static void do_flags(unsigned int mask, unsigned int val)
{
	if (ioctl(sock, SIOCGIFFLAGS, &ifr) == -1) {
		perror("SIOCGIFFLAGS");
		exit(1);
	}
	ifr.ifr_flags &= ~mask;
	ifr.ifr_flags |= val;
	if (ioctl(sock, SIOCSIFFLAGS, &ifr) == -1) {
		perror("SIOCSIFFLAGS");
		exit(1);
	}
}

static int do_keyword(char **arg)
{
	if (strcmp(*arg, "ip") == 0) {
		do_ip_addr_set(arg[1]);
		return 2;
	}
	if (strcmp(*arg, "netmask") == 0) {
		do_ip_mask_set(arg[1]);
		return 2;
	}
	if (strcmp(*arg, "dst") == 0) {
		do_ip_dst_set(arg[1]);
		return 2;
	}
	if (strcmp(*arg, "gw") == 0) {
		do_ip_gw_set(arg[1]);
		return 2;
	}
	if (strcmp(*arg, "broadcast") == 0) {
		do_ip_broad_set(arg[1]);
		return 2;
	}
	if (strcmp(*arg, "up") == 0) {	
		do_flags(IFF_UP, IFF_UP);
		return 1;
	}
	if (strcmp(*arg, "down") == 0) {	
		do_flags(IFF_UP, 0);
		return 1;
	}
	return 0;
}

static const char *fnames[] = {
	"up ",
	"broadcast ",
	"loopback ",
	"pointopoint ",
	"running ",
	"arp ",
	"promisc ",
	"multicast ",
	"link",
	NULL
};

static void ip_print(const char *name, int ioc, int skipok)
{
	if (ioctl(sock, ioc, &ifr) == -1) {
		if (skipok == 0)
			fprintf(stderr, "ifconfig: unable to get %s.\n", name);
		return;
	}
	/* This is a union and all addresses overlap */
	printf("%s %s  ",
		name, inet_ntoa(sin->sin_addr));
}

static const char *hwname[] = {
	NULL,
	"Ethernet",
	"Wireless",
	"SLIP",
	"CSLIP"
};
	
static void display_hwaddr(void)
{
	uint8_t *p = shw->shw_addr;
	unsigned int i;
	switch(shw->shw_family) {
	case HW_NONE: /* Virtual interface */
		return;
	case HW_SLIP: /* Serial with no mac address */
	case HW_CSLIP:
		printf("SLIP");
		break;
	case HW_ETH:
	case HW_WLAN:
		printf("ether: ");
		for (i = 0; i < 5; i++)
			printf("%02X:", *p++);
		printf("%02X  ", *p);
		break;
	default:
		printf("Unknown hardware type %d\n", shw->shw_family);
		return;
	}
	printf("(%s)\n", hwname[shw->shw_family]);
}
		
		
static void dump_interface(void)
{
	const char **p;
	short flags;
	printf("%-8s: ", ifr.ifr_name);
	if (ioctl(sock, SIOCGIFFLAGS, &ifr) == -1) {
		perror("SIOCGIFFLAGS");
		exit(1);
	}
	p = fnames;
	flags = ifr.ifr_flags;
	while(*p) {
		if (flags & 1)
			fputs(*p, stdout);
		flags >>= 1;
		p++;
	}
	flags = ifr.ifr_flags;

	if (ioctl(sock, SIOCGIFMTU, &ifr) == 0)
		printf(" mtu %d\n", ifr.ifr_mtu);
	if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0)
		display_hwaddr();
	ip_print("inet", SIOCGIFADDR, 0);
	if (flags & IFF_POINTOPOINT)
		ip_print("destination", SIOCGIFDSTADDR, 0);
	else
		ip_print("netmask", SIOCGIFNETMASK, 0);
	ip_print("broadcast", SIOCGIFBRDADDR, 1);
	ip_print("gateway", SIOCGIFGWADDR, 1);
	printf("\n");
	/* And we don't keep statistics */
}

static void list_interfaces(void)
{
	unsigned int i;
	for (i = 0; i < 32; i++) {
		ifr.ifr_ifindex = i;
		if (ioctl(sock, SIOCGIFNAME, &ifr) == 0) {
			dump_interface();
			printf("\n");
		} else if (errno != ENODEV) {
			perror("SIOCGIFNAME");
			exit(1);
		}
	}
}

int main(int argc, char *argv[])
{
	unsigned int n = 2;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1) {
		fprintf(stderr, "ifconfig: networking not enabled.\n");
		return 1;
	}
	if (argc == 1) {
		list_interfaces();
		return 0;
	}
	strncpy(ifr.ifr_name, argv[1], sizeof(ifr.ifr_name));
	if (ioctl(sock, SIOCGIFADDR, &ifr) == -1) {
		perror(ifr.ifr_name);
		return 1;
	}
	if (argc == 2) {
		dump_interface();
		return 1;
	}
	/* Configuration by keyword, implied source address */
	while(n < argc) {
		unsigned int s = do_keyword(argv + n);
		if (s == 0)
			do_ip_addr_set(argv[n++]);
		else
			n += s;
	}
}
		