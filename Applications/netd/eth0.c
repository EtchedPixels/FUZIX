#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

static int fd;

int device_send(char *sbuf, int len)
{
  // TODO: implement
  return -1;
}

int device_read(char *buf, int len)
{
  // TODO: implement
  return -1;
}

int device_init(void)
{
  fd = open("/dev/eth0", O_RDWR);
  if (fd < 0) {
    perror("/dev/eth0");
    return -1;
  }
  // TODO: implement
  return 0;
}

uint8_t has_arp = 1U;
uint16_t mtu = 1500U;
