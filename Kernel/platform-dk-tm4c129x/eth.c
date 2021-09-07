#include <kernel.h>
#include <kdata.h>
#include <netdev.h>
#include "inline-irq.h"

#define FLASH_USERREG0 0x400fe1e0
#define FLASH_USERREG1 0x400fe1e4

/* For now until we work out where this really belongs */
uint8_t sock_wake[NSOCKET];

static uint8_t mac_addr[6U] = { 0U, 0U, 0U, 0U, 0U, 0U };
static uint32_t ipa = 0U;
static uint32_t iga = 0U;
static uint32_t igm = 0U;
static uint16_t ifflags = IFF_BROADCAST | IFF_RUNNING | IFF_UP;

static void netdev_reload(void)
{
  // TODO: implement
}

int netproto_socket(void)
{
  // TODO: implement
  return 0;
}

int netproto_bind(struct socket *s)
{
  // TODO: implement
  return 0;
}

int netproto_autobind(struct socket *s)
{
  // TODO: implement
  return 0;
}

int netproto_find_local(struct ksockaddr *addr)
{
  struct socket *s = sockets;
  int i = 0U;

  while (i < NSOCKET) {
    if ((s->s_state < SS_BOUND) || (s->src_addr.sa.family != AF_INET)) {
      s++;
      i++;
      continue;
    }
    if (s->src_addr.sa.sin.sin_port == addr->sa.sin.sin_port) {
      if ((s->src_addr.sa.sin.sin_addr.s_addr ==
           addr->sa.sin.sin_addr.s_addr) ||
           (s->src_addr.sa.sin.sin_addr.s_addr == 0U))
        return i;
    }
    s++;
    i++;
  }
  return -1;
}

int netproto_listen(struct socket *s)
{
  // TODO: implement
  s->s_state = SS_LISTENING;
  return 0;
}

int netproto_begin_connect(struct socket *s)
{
  // TODO: implement
  return 0;
}

int netproto_accept_complete(struct socket *s)
{
  return 0;
}

int netproto_read(struct socket *s)
{
  // TODO: implement
  return 0;
}

arg_t netproto_write(struct socket *s, struct ksockaddr *addr)
{
  // TODO: implement
  return 0;
}

struct socket *netproto_sockpending(struct socket *s)
{
  int i;
  struct socket *n = sockets;
  uint8_t id = s->s_num;

  for (i = 0; i < NSOCKET; i++) {
    if ((n->s_state != SS_UNUSED) && (n->s_parent == id)) {
      n->s_parent = 0xffU;
      return n;
    }
    n++;
  }
  return NULL;
}

void netproto_setup(struct socket *s) { }

void netproto_free(struct socket *s)
{
  // TODO: implement
}

int netproto_close(struct socket *s)
{
  // TODO: implement
  return 0;
}

arg_t netproto_shutdown(struct socket *s, uint8_t how)
{
  // TODO: implement
  return 0;
}

arg_t netproto_ioctl(struct socket *s, int op, char *ifr_u /* in user space */)
{
  static struct ifreq ifr;

  if (uget(ifr_u, &ifr, sizeof(struct ifreq)))
    return -1;
  if (op != SIOCGIFNAME && memcmp(ifr.ifr_name, "eth0", 5U)) {
    udata.u_error = ENODEV;
    return -1;
  }
  switch (op) {
  /* Get side */
  case SIOCGIFNAME:
    if (ifr.ifr_ifindex) {
      udata.u_error = ENODEV;
      return -1;
    }
    memcpy(ifr.ifr_name, "eth0", 5U);
    goto copyback;
  case SIOCGIFINDEX:
    ifr.ifr_ifindex = 0;
    goto copyback;
  case SIOCGIFFLAGS:
    ifr.ifr_flags = ifflags;
    goto copyback;
  case SIOCGIFADDR:
    ifr.ifr_addr.sa.sin.sin_addr.s_addr = ipa;
    goto copy_addr;
  case SIOCGIFBRDADDR:
    /* Hardcoded in the engine */
    ifr.ifr_broadaddr.sa.sin.sin_addr.s_addr = (ipa & igm) | ~igm;
    goto copy_addr;
  case SIOCGIFGWADDR:
    ifr.ifr_gwaddr.sa.sin.sin_addr.s_addr = iga;
    goto copy_addr;
  case SIOCGIFNETMASK:
    ifr.ifr_netmask.sa.sin.sin_addr.s_addr = igm;
    goto copy_addr;
  case SIOCGIFHWADDR:
    memcpy(ifr.ifr_hwaddr.sa.hw.shw_addr, mac_addr, 6U);
    ifr.ifr_hwaddr.sa.hw.shw_family = HW_ETH;
    goto copyback;
  case SIOCGIFMTU:
    ifr.ifr_mtu = 1500;
    goto copyback;
    /* Set side */
  case SIOCSIFFLAGS:
    /* Doesn't really do anything ! */
    ifflags &= ~IFF_UP;
    ifflags |= ifr.ifr_flags & IFF_UP;
    return 0;
  case SIOCSIFADDR:
    ipa = ifr.ifr_addr.sa.sin.sin_addr.s_addr;
    break;
  case SIOCSIFGWADDR:
    iga = ifr.ifr_gwaddr.sa.sin.sin_addr.s_addr;
    break;
  case SIOCSIFNETMASK:
    igm = ifr.ifr_netmask.sa.sin.sin_addr.s_addr;
    break;
  case SIOCSIFHWADDR:
    memcpy(mac_addr, ifr.ifr_hwaddr.sa.hw.shw_addr, 6U);
    /* Not setting hwaddr really! */
    break;
  default:
    udata.u_error = EINVAL;
    return -1;
  }
  netdev_reload();
  return 0;
copy_addr:
  ifr.ifr_addr.sa.sin.sin_family = AF_INET;
copyback:
  return uput(&ifr, ifr_u, sizeof ifr);
}

void netdev_init(void)
{
  uint32_t user0;
  uint32_t user1;
  irqflags_t fl = __hard_di();

  user0 = inl(FLASH_USERREG0);
  user1 = inl(FLASH_USERREG1);
  mac_addr[0] = ((user0 >>  0U) & 0xffU);
  mac_addr[1] = ((user0 >>  8U) & 0xffU);
  mac_addr[2] = ((user0 >> 16U) & 0xffU);
  mac_addr[3] = ((user1 >>  0U) & 0xffU);
  mac_addr[4] = ((user1 >>  8U) & 0xffU);
  mac_addr[5] = ((user1 >> 16U) & 0xffU);
  __hard_irqrestore(fl);
}
