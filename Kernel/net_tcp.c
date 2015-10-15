
static void tcp_fix_endian(void)
{
  ntohl_insitu(&tcph.seq);
  ntohl_insitu(&tcph.ack_seq);
}

static void tcp_tx_reset(void)
{
  tcp_fix_endian();	/* Reverse the fixes of endianness */
  ip_mirror();		/* Going back where it came from */
  /* Now concoct a tcp reset frame and send it */
  /* FIXME */
}

static int tcp_sequence(struct sock *sk)
{
  /* We share next_seq with tcp_queue */
  next_seq = pkt_left - 4 *tcph.doff;
  if (tcph.fin)
    next_seq++;
  next_seq += tcph.seq;
  /* We don't do out of order - if it doesn't overlap or directly follow
     then we kick it out */
  if (after(tcph.seq, s->acked_seq) ||	/* Off the start */
      !after(next_seq, s->acked_seq)) {	/* Already consumed */
    if (tcph.rst)
      return 0;
    if (s->pstate == TCP_SYN_SENT || s->pstate == TCP_SYN_RECV) {
      tcp_tx_reset();
      return 1;
    }
    /* Tell the other end we are out of sync */
    tcp_send_ack(s);
  }
  /* Ok data is relevant */
  return 1;
}

/* We don't accept data + SYN together. This code assumes that as it doesn't
   factor tcph.syn into the maths */

static void tcp_queue(struct sock *s)
{
  uint8_t fin = tcph.fin;
  /* We know this is 16bit safe */
  uint16_t usable = next_seq - sk->acked_seq;
  uint8_t *dptr;
  
  if (fin)	/* FIN isn't a data byte */
    usable--;
  /* First byte of data to consume */
  dptr = pkt_next + pkt_left - usable;
  /* Might be nicer to allow a FIN anyway if all data fitted ? */
  if (usable > s->window)
    usable = s->window;
    fin = 0;
  }
  if (usable) {
    if (s->shutdown & RCV_SHUTDOWN) {
      tcp_tx_reset();
      return;
    }
    /* Queue the data */
    dgram_init();
    dgram.sport = tcph.src;
    dgram.dport = tcph.dst;
    queue_stream(s->inode, dptr, usable);
    /* Our ack position moves on */
    s->acked_seq += usable;
  }
  if (fin) {	/* We have accepted the FIN */
      s->acked_seq++;
    tcp_fin_recv(s);
  }
  /* Queue an ack. More likely we'll actually do the ack when the user
     consumes the data */
  tcp_queue_ack(s);
}

void tcp_rcv(void)
{
  struct socket *s;
  uint16_t len = pkt_left;
  void *thp = pkt_next;
  SNMP(tcp_in_segs);
  if (pkt_type != PKT_HOST)
    return;
  if (!pkt_pull(&tcph, sizeof(tcph)))
    return;
  tcp_fix_endian();	/* Cheaper to do it once */
  if (ip_compute_csum(thp, pkt_left) || tcph.doff < sizeof(tcph) / 4)
    return;
  if (tcph.doff != sizeof(tcph) / 4)
    if (!tcp_options())
      return;
  s = tcp_find();

  if (s == NULL) {
    goto reset;
  }
  
  if (s->pstate != TCP_ESTABLISHED) {
    /* New connection check */
    if (s->pstate == TCP_LISTEN) {
      if (thp.ack || thp.ack || !thp.syn)
        goto reset;
      tcp_create(s);
      return;
    }
    /* Duplicate SYN */
    if (s->pstate == TCP_SYN_RECV && tcph.syn && tcph->seq + 1 == sk->ack_seq)
      return;
    if (s->pstate == TCP_SYN_SENT) {
      if (tcph.ack) {
        if (!tcp_valid_ack(s))
          goto reset;
        if (tcph.rst) {
          tcp_rx_reset(s);	/* Reset the socket */
          return;
        }
        if (!tcph.syn)
          return;
        s->acked_seq = tcph.seq + 1;
        s->fin_seq = tcp.seq;
        s->pstate = TCP_ESTABLISHED;
        s->copied_seq = s->acked_seq;
        tcp_established(s);
        if (sk->max_window == 0)
          s->max_window = 32;
          s->mss = min(s->max_window, s->mtu);
        }
      } else {
        /* Not an ACK */
        if (tcph.syn && !tcph.rst) {
          if (iph.saddr == iph.daddr && tcph.source == tcph.dest)
            goto reset;
          s->pstate = TCP_SYN_RECV;
          /* Check: SYN|ACK reply needed ? */
        }
        return;
      }
      goto rfc_step6;
    }
  }
  /* Established states. Everything must be in window to be valid */
  if (!tcp_sequence(s))
    return;
  if (tcph.rst) {
    tcp_rx_reset(s);
    return;
  }
  if (tcph.syn && !syn_ok)
    goto reset;
  if (tcph.ack && !tcp_ack(s))
    if (s->pstate == TCP_SYN_RECV)
      goto reset;
rfc_step6:
  /* Urgent data - skipping for now */
  s->bytes_rcv += pkt_left;
  /* Empty probe */
  if (pkt_left == 0 && !tcph.fin) }
    if (!tcph.ack)
      tcp_send_ack(s);
    return;
  }
  /* Queue data */
  tcp_queue(s);
  return;

reset:
  tcp_tx_reset();
}
