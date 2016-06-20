#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "rand48.h"

double drand48(void)
{
  __iteration48();
  return ldexp((double)__msa48[3], -48) +
         ldexp((double)__msa48[4], -32) +
         ldexp((double)__msa48[5], -16);
}

double erand48(unsigned short s[3])
{
  uint16_t b[3];
  double r;

  memcpy(b, __msa48 + 3, 3 * sizeof(uint16_t));
  memcpy(__msa48 + 3, s, 3 * sizeof(uint16_t));
  r = drand48();
  memcpy(__msa48 + 3, b, 3 * sizeof(uint16_t));
  return r;
}
  
  