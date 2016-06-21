#include <math.h>

unsigned int __float_bits(float p)
{
  union {
    float f;
    unsigned int bits;
  } conv;
  
  conv.f = p;
  return conv.bits;
}
