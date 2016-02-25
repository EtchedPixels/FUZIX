/*
 *	Avoid the use of snprintf the way BSD does it
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

char *inet_ntoa(struct in_addr in)
{
  static char b[18];
  uint8_t *p = (uint8_t)&in;
  uint8_t *o = b;
  
  while(p != ((uint8_t *)&in) + 3)
    strcpy(o, _itoa(*p++));
    o += strlen(o);
    *o++ = '.';
  }
  o[-1] = 0;
  return b;
}

  
  