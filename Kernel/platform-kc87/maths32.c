/*
 *	32bit maths helpers. We build them here as we need banked versions for
 *	the non LBA kernel
 */

/*-------------------------------------------------------------------------
   Extracted from the sdcc library code:

   Copyright (C) 1999, Sandeep Dutta . sandeep.dutta@usa.net
   Bug fixes by Martijn van Balen, aed@iae.nl

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library; see the file COPYING. If not, write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
   MA 02110-1301, USA.

   As a special exception, if you link this library with other files,
   some of which are compiled with SDCC, to produce an executable,
   this library does not by itself cause the resulting executable to
   be covered by the GNU General Public License. This exception does
   not however invalidate any other reasons why the executable file
   might be covered by the GNU General Public License.
-------------------------------------------------------------------------*/

#define MSB_SET(x) ((x >> (8*sizeof(x)-1)) & 1)

unsigned long
_modulong (unsigned long a, unsigned long b)
{
  unsigned char count = 0;

  while (!MSB_SET(b))
  {
     b <<= 1;
     if (b > a)
     {
        b >>=1;
        break;
     }
     count++;
  }
  do
  {
    if (a >= b)
      a -= b;
    b >>= 1;
  }
  while (count--);
  return a;
}

unsigned long
_divulong (unsigned long x, unsigned long y)
{
  unsigned long reste = 0L;
  unsigned char count = 32;
  _Bool c;

  do
  {
    // reste: x <- 0;
    c = MSB_SET(x);
    x <<= 1;
    reste <<= 1;
    if (c)
      reste |= 1L;

    if (reste >= y)
    {
      reste -= y;
      // x <- (result = 1)
      x |= 1L;
    }
  }
  while (--count);
  return x;
}

struct some_struct {
	short a ;
	char b;
	long c ;};

/* little endian order */
union bil {
        struct {unsigned char b0,b1,b2,b3 ;} b;
        struct {unsigned short lo,hi ;} i;
        unsigned long l;
        struct { unsigned char b0; unsigned short i12; unsigned char b3;} bi;
};

#define bcast(x) ((union bil *)&(x))

long _mullong (long a, long b)
{
  unsigned short i12;

  /* only (a->i.lo * b->i.lo) 16x16->32 to do. asm? */
  bcast(a)->i.hi += bcast(a)->b.b1 * bcast(b)->b.b1;

  i12 = bcast(b)->b.b0 * bcast(a)->b.b1;
  bcast(b)->bi.i12 = bcast(a)->b.b0 * bcast(b)->b.b1;

  /* add up the two partial result, store carry in b3 */
  bcast(b)->b.b3 = ((bcast(b)->bi.i12 += i12) < i12);

  bcast(a)->i.lo  = bcast(a)->b.b0 * bcast(b)->b.b0;

  bcast(b)->bi.b0 = 0;

  return a + b;
}
