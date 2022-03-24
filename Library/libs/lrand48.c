#include <stdlib.h>
#include <string.h>
#include "rand48.h"

static uint16_t msa48_def[4] = { 0xe66d, 0xdeec, 0x0005, 0x330E };
uint16_t __msa48[7] = {0xe66d, 0xdeec, 0x0005, 0x330E, 0xABCD, 0x1234, 0x000B};
static uint32_t sum;
static uint16_t tmp[2];
static uint16_t msa48_save[3];

static void lmuladd(uint8_t a, uint8_t b)
{
  sum += (unsigned long)__msa48[a] * (unsigned long)__msa48[b];
}

static uint16_t roll(void)
{
  uint16_t v = sum;
  sum >>= 16;
  return v;
}

void __iteration48(void)
{
  sum = 0;
  lmuladd(0, 3);
  sum += (uint32_t)__msa48[6];
  
  tmp[0] = roll();
  
  lmuladd(0, 4);
  lmuladd(1, 3);
  
  tmp[1] = roll();
  
  lmuladd(0, 5);
  lmuladd(1, 4);
  lmuladd(2, 3);
  
  __msa48[3] = tmp[0];
  __msa48[4] = tmp[1];
  __msa48[5] = roll();
}

void srand48(long seed)
{
  __msa48[4] = (uint16_t) seed;
  __msa48[5] = (uint16_t) (seed >> 16);

  memcpy(__msa48, msa48_def, 4 * sizeof(uint16_t));
}

void lcong48(unsigned short p[7])
{
  memcpy(__msa48, p, 7 * sizeof(uint16_t));
}

long lrand48(void)
{
  __iteration48();
  return (long)((__msa48[5] >> 1) + (((uint32_t)__msa48[4]) << 15));
}

long nrand48(unsigned short s[3])
{
  uint16_t b[3];
  long r;

  memcpy(b, __msa48 + 3, 3 * sizeof(uint16_t));
  memcpy(__msa48 + 3, s, 3 * sizeof(uint16_t));
  r = lrand48();
  memcpy(__msa48 + 3, b, 3 * sizeof(uint16_t));
  return r;
}

unsigned long mrand48(void)
{
  __iteration48();
  return (unsigned long)((__msa48[5]) + (((uint32_t)__msa48[4]) << 16));
}

unsigned long jrand48(unsigned short s[3])
{
  uint16_t b[3];
  unsigned long r;

  memcpy(b, __msa48 + 3, 3 * sizeof(uint16_t));
  memcpy(__msa48 + 3, s, 3 * sizeof(uint16_t));
  r = mrand48();
  memcpy(__msa48 + 3, b, 3 * sizeof(uint16_t));
  return r;
}

unsigned short *seed48(unsigned short n[7])
{
  memcpy(msa48_save, __msa48 + 3, 3 * sizeof(uint16_t));
  memcpy(__msa48 + 3, n, 3 * sizeof(uint16_t));
  memcpy(__msa48, msa48_def, 3 * sizeof(uint16_t));
  return msa48_save;
}

