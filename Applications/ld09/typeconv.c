
/*
 * Type conversion routines, these have been rewritten for portability.
 *
 * The only requirement is now that the u2_t and u4_t must be big enough.
 */

#include "syshead.h"
#include "const.h"
#include "type.h"
#include "globvar.h"

void xxerr P((char *));
void xxerr(x) char * x; { write(2, x, strlen(x)); }

static int no_swap   = 1;

static int long_off[4] = {0,1,2,3};
static int int_off[2] = {0,1};

PUBLIC bool_pt typeconv_init(big_endian, long_big_endian)
bool_pt big_endian;
bool_pt long_big_endian;
{
   int i;
   no_swap = (!big_endian && !long_big_endian);

   for(i=0; i<4; i++) long_off[i] = i;
   for(i=0; i<2; i++) int_off[i] = i;

   if( long_big_endian )
   {
      i = long_off[0]; long_off[0] = long_off[2]; long_off[2] = i;
      i = long_off[1]; long_off[1] = long_off[3]; long_off[3] = i;
   }
   if( big_endian )
   {
      i = long_off[2]; long_off[2] = long_off[3]; long_off[3] = i;
      i = long_off[0]; long_off[0] = long_off[1]; long_off[1] = i;

      i = int_off[0]; int_off[0] = int_off[1]; int_off[1] = i;
   }
   return 1;
}

PUBLIC void u2c2(buf, offset)
char *buf;
u2_pt offset;
{
#ifdef __AS386_16__
   if( no_swap )
   {
      *((unsigned short*)buf) = offset;	/* UNALIGNED ACCESS! */
      return;
   }
#endif
   buf[int_off[0]] = offset;
   buf[int_off[1]] = (offset>>8);
}

PUBLIC void u4c4(buf, offset)
char *buf;
u4_t offset;
{
   int i;
#ifdef __AS386_16__
   if( no_swap )
   {
      *((unsigned long*)buf) = offset;	/* UNALIGNED ACCESS! */
      return;
   }
#endif
   for(i=0; i<4; i++)
   {
      buf[long_off[i]] = offset;
      offset >>= 8;
   }
}

PUBLIC void u4cn(buf, offset, count)
char *buf;
u4_t offset;
unsigned count;
{
    switch(count)
    {
    case 1:
	buf[0] = (char) offset;
	return;
    case 2:
	u2c2(buf, (u2_pt) offset);
	return;
    case 4:
	u4c4(buf, (u4_t) offset);
	return;
    default:
        xxerr("WARNING: typeconv.c(u4cn) illegal count\n");
	return;
    }
}

PUBLIC void u2cn(buf, offset, count)
char *buf;
u2_pt offset;
unsigned count;
{
    switch(count)
    {
    case 1:
	buf[0] = (char) offset;
	return;
    case 2:
	u2c2(buf, (u2_pt) offset);
	return;
    case 4:
	u4c4(buf, (u4_t) offset);
	return;
    default:
        xxerr("WARNING: typeconv.c(u2cn) illegal count\n");
	return;
    }
}

PUBLIC u2_pt c2u2(buf)
char *buf;
{
    u2_pt res;
#ifdef __AS386_16__
   if( no_swap ) return *((u2_pt *)buf);	/* UNALIGNED ACCESS! */
#endif

    res  =   ((unsigned char *)buf) [int_off[0]]
         + ((((unsigned char *)buf) [int_off[1]]) << 8);
    return res;
}

PUBLIC u4_t c4u4(buf)
char *buf;
{
    u4_t res;
    int i;
#ifdef __AS386_16__
   if( no_swap ) return *((u4_t *)buf);		/* UNALIGNED ACCESS! */
#endif
    res = 0;
    for(i=3; i>=0; i--)
    {
        res = (res<<8) + ((unsigned char *)buf) [long_off[i]];
    }
    return res;
}

PUBLIC u4_t cnu4(buf, count)
char *buf;
unsigned count;
{
    switch (count)
    {
    case 0:
    	return 0;
    case 1:
	return buf[0] & 0xFF;
    case 2:
	return c2u2(buf);
    case 4:
	return c4u4(buf);
    default:
        xxerr("WARNING: typeconv.c(cnu4) illegal count\n");
	return 0;
    }
}

PUBLIC u2_pt cnu2(buf, count)
char *buf;
unsigned count;
{
    switch (count)
    {
    case 0:
	return 0;
    case 1:
	return buf[0] & 0xFF;
    case 2:
	return c2u2(buf);
    case 4:
	return (u2_pt) c4u4(buf);
    default:
        xxerr("WARNING: typeconv.c(cnu2) illegal count\n");
	return 0;
    }
}
