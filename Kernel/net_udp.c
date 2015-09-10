#include <kernel.h>
#include <net.h>

static struct socket *udp_find(void)
{
  return sock_find(udph.src, udph.dst);
}

void udp_rcv(void)
{
  struct socket *s;
  SNMP(ip_in_delivers);
  if (!pkt_pull(&udph, sizeof(struct udphdr)) ||
     udph.csum && ip_compute_csum_data(&udph, sizeof(udph), pkt_next, pkt_left))) {
    SNMP(udp_in_errors);
    return;
  }

  s = udp_find();
  if (s == NULL)
    icmp_send_unreach(ICMP_DEST_UNREACH, ICMP_PORT_UNREACH);
  else {
    dgram_init();/* msg.len = pkt_left, msg.ptr = pkt_next, sets src/dst ip */
    dgram.sport = udph.src;
    dgram.dport = udph.dst;
    queue_dgram(s->inode);
  }
}

int udp_send(struct socket *s, void *buf, uint16_t len)
{
  udph.src = s->sport;
  udph.dst = s->dport;
  udph.len = htons(len);
  udph.check = 0;
  len += sizeof(struct udphdr);
  ip_prepare(s, IPPROTO_UDP, len);
  udph.check = ip_compute_csum_data(&udph, sizeof(udph), buf, len);
  len += sizeof(struct iphdr);
  if (len > MAX_IP)
   return -EMSGSIZE;
  return ip_output(&udph, sizeof(udph), buf, len);
}
