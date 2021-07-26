/*
 *	Avoid the use of snprintf the way BSD does it
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

char *_inet_ntoa(uint32_t in)
{
  static char b[18];
  uint8_t *p = (uint8_t *)&in;
  char *o = b;
  
  while(p != ((uint8_t *)&in) + 4) {
    strcpy(o, _itoa(*p++));
    o += strlen(o);
    *o++ = '.';
  }
  o[-1] = 0;
  return b;
}

  
  