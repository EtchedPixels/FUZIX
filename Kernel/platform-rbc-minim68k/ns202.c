/* ns202.c   version for FUZIX on Mini-M68k board	*/
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
#include <kernel.h>
#include <config.h>
#include <mfpic.h>
#include <ns202.h>
#include <iomem.h>


void ns202_init(void)
{
	uint16_t counth = (hertz/TICKSPERSEC) - 1;
	uint16_t countl =
#if defined(TICKSPERSECL)
			(hertz/TICKSPERSECL) - 1;
#elif defined(COUNTLOW)
			COUNTLOW - 1;
#else
			65535;
#endif
	uint16_t intmask = 0xFFFF;
	uint8_t  cctlv = 0;
	uint8_t  ciptrv = 0xFF;		/* assign to int 15 by default */

#ifdef INT_TIMER
	intmask ^= (1<<INT_TIMER);
	cctlv |= 8;	/* set CRUNH bit */
	ciptrv = (ciptrv & 0x0F)|(INT_TIMER<<4);
#endif
#ifdef INT_TIMERL
	intmask ^= (1<<INT_TIMERL);
	cctlv |= 4;	/* set CRUNL bit */
	cciptrv = (ciptrv & 0xF0)|(INT_TIMERL);
#endif
#ifdef INT_TTY1
	intmask ^= (1<<INT_TTY1); 
#endif
	
	
	
/*  CFRZ COUTD coutm clkm . FRZ 0 NTAR t16n8  */
	icu_outp(mctl, 0xCA);

/*  ccon cfnps cout1 cout0 . crunh crunl cdcrh cdcrl  */
	icu_outp(cctl, 0x00);

/*  initialize counters  prescaling in use  */
	icu_outpw(lcsv, countl);
	icu_outpw(hcsv, counth);
	icu_outp(ciptr, ciptrv);

	icu_outpw(lccv, countl);
	icu_outpw(hccv, counth);

/*  cerh cirh CIEH WENH cerl cirl ciel WENL  */
	icu_outp(cictl, 0x31);

/*  I/O port select:  0 = I/O, 1 = interrupt  */
	icu_outp(ips, 0xFF);

/*  port direction:  0 = output, 1 = input  */
	icu_outp(pdir, 0xFF);

/*  output clock assignment:  0 = not used, 1 = assigned */
	icu_outp(ocasn, 0x00);

/*  port data (pdat) -- not touched  */

/*  in service interrupts  */
	icu_outpw(isrv, 0x0000);

/*  cascade source  */
	icu_outpw(csrc, 0x0000);

/*  software vector assignment  */
	icu_outp(svct, 16);			/* vector 16, not 24 !!!  */

/***  value for the Motorola 68000 and 68008 ***/
	icu_outp(mf_cfg, 64+4);		/* overrides the above */
/* ******************  Use 16 vectors from 64..79
	IRQ9 is reserved for the PPIDE interrupt on the MF/PIC
	IRQ12 is reserved for the Serial UART
	Timers are yet to be assigned interrupt locations
**************************************************************************** */


/*  MF_CFG register on other CPU's */
#if 0
/***  value for the Z80 in IM 0  */
	icu_outp(mf_cfg, 0xFB);

/***  value for the Z80 in IM 2  */
		addr = 0xFFE0; e.g.		vector table aligned on 16-byte boundary
		Z80_ireg = addr >> 8;		high byte for I-register
		IM2_cfg = addr & 0xF0;		mask low bits correctly
	icu_outp(mf_cfg, IM2_cfg + 1);
#endif



/*  first priority  xxxx  on read;  xf  on write  */
#ifdef INT_TIMER
	icu_outp(fprt, INT_TIMER);
#else
	icu_outp(fprt, 0);
#endif

/*  triggering polarity (high, or rising) */
	icu_outpw(tpl, 0xFFFF);

/*  LEVEL/edge triggering  */
	icu_outpw(eltg, 0xFFFF);

/*  reset COUTD to start internal sampling oscillator  */
/*  cfrz coutd coutm clkm . frz 0 NTAR t16n8  */
	icu_outp(mctl, 0x02);

/*  ccon cfnps cout1 cout0 . CRUNH crunl cdcrh cdcrl  */
	icu_outp(cctl, cctlv);

/*  clear some mask bits  */
	icu_outpw(imsk, intmask);

}

