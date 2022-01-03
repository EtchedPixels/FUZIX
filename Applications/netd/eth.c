#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "fuzix-conf.h"
#include "netd.h"
#include "uip.h"
#include "uip_arp.h"

static int fd = -1;

int device_send(char *sbuf, int len)
{
  return write(fd, sbuf, len);
}

int device_read(char *buf, int len)
{
  return read(fd, buf, len);
}

int device_init(void)
{
  uip_eth_addr ethaddr;
  uip_ipaddr_t ipaddr;
  uint32_t ipv4addr;
  uint8_t *addr = ((uint8_t *)(&ipv4addr));

  fd = open("/dev/eth", O_RDWR);
  if (fd < 0) {
    perror("/dev/eth");
    return -1;
  }
  if (ioctl(fd, 0, ethaddr.addr)) {
    perror(">MAC");
    return -1;
  }
  uip_setethaddr(ethaddr);
  if (ioctl(knet, NET_MAC, ethaddr.addr)) {
    perror("MAC>");
    return -1;
  }
  if (ioctl(knet, NET_MTU, &mtu)) {
    perror("MTU>");
    return -1;
  }
  uip_gethostaddr(&ipaddr);
  ipv4addr = 0U;
  addr[0] = uip_ipaddr1(&ipaddr);
  addr[1] = uip_ipaddr2(&ipaddr);
  addr[2] = uip_ipaddr3(&ipaddr);
  addr[3] = uip_ipaddr4(&ipaddr);
  if (ioctl(knet, NET_IPADDR, &ipv4addr)) {
    perror("IPADDR>");
    return -1;
  }
  uip_getnetmask(&ipaddr);
  ipv4addr = 0U;
  addr[0] = uip_ipaddr1(&ipaddr);
  addr[1] = uip_ipaddr2(&ipaddr);
  addr[2] = uip_ipaddr3(&ipaddr);
  addr[3] = uip_ipaddr4(&ipaddr);
  if (ioctl(knet, NET_MASK, &ipv4addr)) {
    perror("NETMASK>");
    return -1;
  }
  uip_getdraddr(&ipaddr);
  ipv4addr = 0U;
  addr[0] = uip_ipaddr1(&ipaddr);
  addr[1] = uip_ipaddr2(&ipaddr);
  addr[2] = uip_ipaddr3(&ipaddr);
  addr[3] = uip_ipaddr4(&ipaddr);
  if (ioctl(knet, NET_GATEWAY, &ipv4addr)) {
    perror("GATEWAY>");
    return -1;
  }
  return 0;
}

uint8_t has_arp = 1U;
uint16_t mtu = 1500U;
