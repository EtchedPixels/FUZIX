#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* This is only compiled and generated for little endian platforms */

uint16_t htons(uint16_t x)
{
  return (x << 8) | (x >> 8);
}
