#include "kernel.h"
#include "net.h"

void ip_mirror(void)
{
  memcpy(&oiph, &iph, sizeof(iph));
  if (pkt_type != PKT_HOST)
    oiph.saddr = ip_addr;
  else
    oiph.saddr = iph.daddr;
  oiph.daddr = iph.saddr;
  oiph.check = 0;
  oiph.ihl = 5;
  oiph.frag_off = 0;
}

void ip_prepare(struct socket *s, uint16_t len)
{
 oiph.saddr = s->saddr;
 oiph.daddr = s->daddr;
 oiph.tot_len = htons(len);
 oiph.protocol = s->proto; 
 oiph.ihl = 5;
 oiph.check = 0;
 oiph.frag_off = 0;
 oiph.tos = 0;
}

int ip_output(void *pbuf, uint16_t plen, void *dbuf, uint16_t dlen)
{
  oiph.id = htons_inc(ip_id);
  oiph.ttl = IP_TTL;
  oiph.check = 0;
  oiph.version = 4;
  /* Options not supported */
  oiph.ihl = 5;
  oiph.check = ip_checksum(&oiph, sizeof(oiph));

  output_begin();	/* Set up output buffer (space left for header) */
  output_add(&oiph, 4 * oiph.ihl);
  output_add(pbuf, plen);
  ouput_add(dbuf, dlen);
  if (LOOPBACK(oiph.daddr) || oiph.daddr == ip_addr)
    return loopback_queue();
  else
    return mac_queue();
  /* We do blocking writes, when this code returns the buffer is on the
     wire and we don't have to fret about re-use. Does mean slip has to be
     careful to buffer the receive side while queueing output */
}

static int ip_outions(void)
{
  return 0;
}

void ip_rcv(void)
{
  uint16_t len = pkt_left;
  SNMP(ip_in_receives);
  if (!pkt_pull(&iph, sizeof(struct iphdr)))
    return;
  plen = ntohs(iph.tot_len);
  /* FIXME: for speed fold ihl/version and be smarter */
  if (iph.ihl < 5 || iph.version != 4 || len < plen) {
    SNMP(ip_in_hdr_errors);
    return;
  }
  plen -= sizeof(struct iphdr));
  if (pkt_len > plen)
    pkt_len = plen;

  /* FIXME: checksum */
  if (iph.ihl != 5)
    if (ip_options())
      return;
  /* No frags for now (memory limits on 8bit) */
  if (iph.frag_off)	/* FIXME: check MF and FO */
    return;
  if (iph.daddr == 0xFFFFFFFF)
    pkt_type = PKT_BROADCAST;
  else if (MULTICAST(iph.daddr))
    pkt_type = PKT_MULTICAST;
  else if (iph.daddr == ip_addr || LOOPBACK(iph.daddr))
    pkt_type = PKT_HOST;
  else
    /* No forwarding so we don't have to worry about martians either */
    return;

  /* FIXME: raw sockets ?? */
  if (iph.protocol == IPPROTO_TCP)
    tcp_rcv();
  else if (iph.protocol == IPPROTO_UDP)
    udp_rcv();
  else if (iph.protocol == IPPROTO_ICMP)
    icmp_rcv();
  else
    icmp_send_unreach(ICMP_DEST_UNREACH, ICMP_PROT_UNREACH);
}
