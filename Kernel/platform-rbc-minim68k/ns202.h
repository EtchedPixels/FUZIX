/* ns202.h	*/
/*
	Copyright (C) 2011,2022 John R. Coffman.

***********************************************************************

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    in the file COPYING in the distribution directory along with this
    program.  If not, see <http://www.gnu.org/licenses/>.

**********************************************************************/
/*	*/
/*   Register definitions for the NS32202 interrupt controller	*/
/*	*/
/*	*/
#ifndef _ns202_h
#define _ns202_h 1
#include <mfpic.h>
#include <iomem.h>


#define icu_reg(x) (mf_202 + (x)*256)

/* hvct	        =  0	     b	  hardware vector - read by INTA only	*/
#define hvct icu_reg(0)
/* svct	        =  1	     b	  software vector - pending	*/
#define svct icu_reg(1)
/* svcti           =  svct+32   b   software vector - in service	*/
#define svcti icu_reg(1+32)
/* eltg	        =  2	     w	  edge/level triggering	*/
#define eltg icu_reg(2)
/* tpl	        =  4	     w	  triggering polarity	*/
#define tpl icu_reg(4)
/* ipnd	        =  6	     w	  interrupts pending	*/
#define ipnd icu_reg(6)
/* isrv	        =  8	     w	  interrupts in service	*/
#define isrv icu_reg(8)
/* imsk	        = 10	     w	  interrupt mask	*/
#define imsk icu_reg(10)
/* csrc	        = 12	     w	  cascaded source	*/
#define csrc icu_reg(12)
/* fprt	        = 14	     w	  first priority	*/
#define fprt icu_reg(14)
/* mctl	        = 16	     b	  mode control	*/
#define mctl icu_reg(16)
/* ocasn	        = 17	     b	  output clock assignment	*/
#define ocasn icu_reg(17)
/* ciptr	        = 18	     b	  counter interrupt pointer	*/
#define ciptr icu_reg(18)
/* pdat	        = 19	     b	  port data	*/
#define pdat icu_reg(19)
/* ips	        = 20	     b	  interrupt/port select	*/
#define ips icu_reg(20)
/* pdir	        = 21	     b	  port direction	*/
#define pdir icu_reg(21)
/* cctl	        = 22	     b	  counter control	*/
#define cctl icu_reg(22)
/* cictl	        = 23	     b	  counter interrupt control	*/
#define cictl icu_reg(23)
/* lcsv	        = 24	     w	  l-counter starting value	*/
#define lcsv icu_reg(24)
/* hcsv	        = 26	     w	  h-counter starting value	*/
#define hcsv icu_reg(26)
/* lccv	        = 28	     w	  l-counter current value	*/
#define lccv icu_reg(28)
/* hccv	        = 30	     w	  h-counter current value	*/
#define hccv icu_reg(30)

/* eoi             = hvct+32    b   read for End Of Interrupt signal	*/
#define eoi icu_reg(0+32)		/* hvct + A13 */


/* counter_clock	=	1843200     input counter (no prescaling yet) */
#define counter_clock 1843200
#define hertz (counter_clock/4)





#if CPU>68010
#define CALLS 1
int c_usec_delay(int microseconds);	/* delay resolution about 16usec */
#else
#define CALLS 0
#endif

#if CALLS
int32 icu_inp(uint8_t *reg)
{
	c_usec_delay(16);
	return (int32)(*reg);
}
#else
# define		icu_inp(r)  ((uint16_t)inp(r))
#endif

#if CALLS
uint16_t icu_inpw(uint16_t reg)
{
	uint16_t result;

	result = icu_inp(reg+256) << 8;
	result |= icu_inp(reg);

	return result;
}
#else
# define icu_inpw(r)      ((uint16_t)inp(r)+((uint16_t)inp(r+256)<<8))
#endif

#if CALLS
void icu_outp(uint8_t *reg, int32 bval)
{
	c_usec_delay(16);
	*reg = (uint8_t)bval;
}
#else
# define		icu_outp(r,b) outp((r),(b))
#endif

#if CALLS
void icu_outpw(uint16_t reg, uint16_t wval)
{
	icu_outp(reg, wval & 255);
	icu_outp(reg+256, wval>>8);
}
#else
# define  icu_outpw(r,v)  do { outp((r),(v)); outp((r+256),(v>>8)); } while (0)
#endif


#endif	/* end _ns202_h */

