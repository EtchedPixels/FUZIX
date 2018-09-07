/**
 * PLATOTerm64 - A PLATO Terminal for the Commodore 64
 * Based on Steve Peltz's PAD
 * 
 * Author: Thomas Cherryhomes <thom.cherryhomes at gmail dot com>
 *
 * io.c - Input/output functions (serial/ethernet)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "io.h"
#include "protocol.h"

static uint8_t ch=0;
int io_fd = -1;
int io_eof;


/**
 * io_init() - Open the device
 */
void io_init(void)
{
  int s = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in sin;
  if (s == -1) {
    perror("socket");
    return;
  }
  sin.sin_family = AF_INET;
  sin.sin_port = htons(8005);
  /* FIXME: don't hard code! */
  sin.sin_addr.s_addr = htonl(0x60E2F5B3);
  if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    perror("connect");
    return;
  }
  fcntl(s, F_SETFL, FNDELAY);
  io_fd = s;
}

/**
 * io_recv() - Receive and interpret data.
 */
void io_main(void)
{
  char buf[576];
  int l;

  /* The OS does our buffering (badly or not varies) */
  while((l = read(io_fd,  buf, 256)) > 0)
    ShowPLATO(buf, l);
  if (l <= 0) {
    if (l == -1 && errno != EAGAIN) {
      io_eof = 1;
      perror("read");
    }
  }
  io_process_queue();
}

/**
 * io_done() - Called to close I/O
 */
void io_done(void)
{
  close(io_fd);
  io_fd = -1;
}
