#include <kernel.h>
#include <net.h>

static void icmp_echo(void)
{
  ip_mirror(&iph);	/* Flip the source and destination into oiph*/
  icmph.type = ICMP_ECHOREPLY;
  icmph.code = 0;
  icmph.checksum = 0;
  icmph.checksum = ip_compute_csum_data(&icmph, sizeof(icmph), pkt_next, pkt_left);
  ip_output(&icmph, sizeof(icmph), pkt_next, pkt_left) ;
  SNMP(icmp_out_echoreply);
}

static void icmp_unreach(void)
{
  /* Get the header of the packet that caused the problem */
  if (!pkt_pull(&iph2, sizeof(struct iphdr)))
    return;
  if (iph2.protocol == IPROTO_TCP)
    tcp_unreach();
}

/*
 *	We handle the various unreachables and echo (ping) but as per
 *	usual practice we don't touch broadcasts and we don't listen
 *	to redirects and the like.
 */
void icmp_rcv(void)
{
  uint16_t mask;
  SNMP(icmp_in);
  if (pkt_type != PKT_HOST)
    return;
  if (pkt_left < sizeof(struct icmphdr) || ip_compute_csum(pkt_next, pkt_left)) {
    SNMP(icmp_in_error);
    return;
  }
  pkt_pull(&icmph, sizeof(struct icmphdr));
  if (icmph.type > ICMP_ADDRESREPLY)
    return;
  SNMP(icmp_in_type[icmph.type]);
  mask = 1 << icmph.type;
  if (mask & ICMP_UNREACH_MASK)
    icmp_unreach();
  else if (icmph.type == ICMP_ECHO)
    icmp_echo();
}

/* Used by upper layers to send ICMP messages responding to frames. iph holds
   the original ip header, pkt_ip points to the ip header in the data block */

void icmp_send_unreach(uint8_t type, uint8_t code)
{
  len = iph.len << 2 + 8;
  ip_mirror(&iph);
  icmph.type = type;
  icmph.code = code;
  icmph.checksum = 0;
  /* FIXME: better to send more for modern stacks */
  icmph.checksum = ip_compute_csum_data(&icmph, sizeof(icmph), pkt_ip, len);
  ip_output(&icmph, sizeof(icmph), pkt_ip, len);
  SNMP(icmp_out_unreach);
}

