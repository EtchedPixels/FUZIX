#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* This is only compiled and generated for little endian platforms */

uint32_t htonl(uint32_t v)
{
  uint8_t *p = (uint8_t *)&v;
  return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}
