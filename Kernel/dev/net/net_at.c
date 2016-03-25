#include <kernel.h>
#include <kdata.h>
#include <netdev.h>
#include <net_at.h>
#include <printf.h>

/*
 *	Implement a one socket TCP model for interfaces that provide only AT
 *	command emulation of ATD addr:port, ATH and carrier line.
 */

static uint8_t at_state;

static void netat_write(const char *p, uint16_t len)
{
  while(len--)
    netat_outbyte(*p++);
}

/* Borrowed from libc */
static const char *_uitoa(uint16_t i)
{
  static char buf[8];
  char *p = buf + sizeof(buf);
  int c;

  *--p = '\0';
  do {
    c = i % 10;
    i /= 10;
    *--p = '0' + c;
  } while(i);
  return p;
}

static void netat_write_u16ch(uint16_t v, char c)
{
  const char *p = _uitoa(v);
  while(*p)
    netat_outbyte(*p++);
  netat_outbyte(c);
}

static void netat_write_u8ch(uint8_t v, char c)
{
  netat_write_u16ch(v, c);
}

static void wakeup_all(void)
{
  wakeup(&sockets[0]);
  wakeup(&sockets[0].s_data);
  wakeup(&sockets[0].s_iflag);
}

void netat_hangup(void)
{
  if (sockets[0].s_state != SS_UNUSED) {
    sockets[0].s_state = SS_CLOSED;
    sockets[0].s_error = EPIPE;
    wakeup_all();
  }
}

static void netat_do_hangup(void)
{
  /* Need to delay and space this but this will do now for human
     pretending to be modem testing */
  netat_write("+++ATH\n", 7);
}
/* We read off the ready event until we get data */

void netat_event(void)
{
  uint8_t ch;
  if (at_state == 4) {
    sockets[0].s_iflag |= SI_DATA;
    wakeup(&sockets[0].s_iflag);
    netat_nowake();
    return;
  }
  ch = netat_byte();
  switch(at_state) {
    case 0:	/* Discard */
      return;
    case 1:
      if (ch == 'C')
        at_state = 2;
      else {
        netat_hangup();
        at_state = 0;
      }
      break;
    case 2:
      if (ch == 'O')
        at_state = 3;
      else {
        netat_hangup();
        at_state = 0;
      }
      break;
    case 3:
      if (ch != '\n')
        break;
      at_state = 4;	/* Don't process */
      sockets[0].s_state = SS_CONNECTED;
      wakeup(&sockets[0]);
      break;
  }
}

int net_init(struct socket *s)
{
  if (s != &sockets[0]) {
    udata.u_error = ENOMEM;
    return -1;
  }
  if (s->s_type != SOCKTYPE_TCP) {
    udata.u_error = EPFNOSUPPORT;
    return -1;
  }
  s->s_state = SS_UNCONNECTED;
  return 0;
}

int net_bind(struct socket *s)
{
  s->s_state = SS_BOUND;
  return 0;
}

int net_listen(struct socket *s)
{
  used(s);
  udata.u_error = EOPNOTSUPP;
  return -1;
}
  
int net_connect(struct socket *s)
{
  uint32_t n = s->s_addr[SADDR_DST].addr;
  uint16_t p = s->s_addr[SADDR_DST].port;

  if (IN_LOOPBACK(n) || IN_LOOPBACK(s->s_addr[SADDR_SRC].addr)) {
    udata.u_error = ECONNRESET;
    s->s_state = SS_CLOSED;
    return -1;
  }

  netat_wake();

  n = ntohl(n);

  /* Pity AT command sets won't talk addresses and ports as a hex block ! */
  netat_flush();
  netat_write("ATD ", 4);
  netat_write_u8ch(n >> 24, '.');
  netat_write_u8ch(n >> 16, '.');
  netat_write_u8ch(n >> 8, '.');
  netat_write_u8ch(n, ' ');
  netat_write_u16ch(p, '\n');
  /* Data ready on the channel will do the rest */
  s->s_state = SS_CONNECTING;
  at_state = 1;
  return 0;
}

void net_close(struct socket *s)
{
  if (at_state == 4) {
    netat_do_hangup();		/* Either +++ ATH with spacing, or carrier drop */
    at_state = 0;
  }
  s->s_state = SS_CLOSED;
}

arg_t net_read(struct socket *s, uint8_t flag)
{
  uint16_t n = 0;
  
  while(1) {
    if (s->s_state < SS_CONNECTED) {
      udata.u_error = EINVAL;
      return -1;
    }
    
    while(netat_ready() && n < udata.u_count) {
      if (s->s_iflag & SI_SHUTR)
        break;
      uputc(netat_byte(), udata.u_base++);
      n++;
    }
    if (n || (s->s_iflag & SI_SHUTR))
      return n;
    s->s_iflag &= ~SI_DATA;
    netat_wake();
    /* Could do with using timeouts here to be clever for non O_NDELAY so
       we aggregate data. For now assume a fifo */
    if (psleep_flags(&s->s_iflag, flag))
      return -1;
  }
}

arg_t net_write(struct socket *s, uint8_t flag)
{
  uint16_t n = 0;

  used(s);
  used(flag);

  while(n < udata.u_count) {
    if (sockets[0].s_state == SS_CLOSED || (sockets[0].s_iflag & SI_SHUTW)) {
      udata.u_error = EPIPE;
      ssig(udata.u_ptab, SIGPIPE);
      return -1;
    }
    /* FIXME - screen +++ handling ! */
    netat_outbyte(ugetc(udata.u_base++));
    n++;
  }
  return udata.u_count;
}

arg_t net_shutdown(struct socket *s, uint8_t flag)
{
  s->s_iflag |= flag;
  netat_wake();
  return 0;
}

struct netdevice net_dev = {
  0,
  "at0",
  IFF_POINTOPOINT
};

arg_t net_ioctl(uint8_t op, void *p)
{
  used(op);
  used(p);
  return -EINVAL;
}

void netdev_init(void)
{
}
