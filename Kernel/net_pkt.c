#include "kernel.h"
#include "net.h"

/*
 *	We copy headers into fixed target locations so that
 *	processors with poor indexing generate references to
 *	link time resolved static memory addresses.
 */

int pkt_pull(void *ptr, uint16_t len)
{
  if (len > pkt_left)
    return -1;
  memcpy(ptr, pkt_buf, len);
  pkt_buf += len;
  pkt_left -= len;
  return 0;
}
