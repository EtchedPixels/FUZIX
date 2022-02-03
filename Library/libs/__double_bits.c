#include <math.h>

uint64_t __double_bits(double p)
{
  union {
    double d;
    unsigned long bits;
  } conv;
  
  conv.d = p;
  return conv.bits;
}
