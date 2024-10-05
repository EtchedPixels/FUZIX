/* iomem.h -- memory based I/O to bypass what we
	cannot use in  cpu-68000/cpu.h			*/

#ifndef _IOMEM_DOT_H
#define _IOMEM_DOT_H

/*  'in' and 'out' handle 8-bit data on an 8-bit port
   'inp' and 'outp' handle 8-bit data on a 16-bit port
    Needed by I/O to the NS32202 chip on the MF/PIC board */

#define inp(a) (*IOMEM16(a))
#define outp(a,y) do { *IOMEM16(a)=(uint8_t)(y) ; } while (0)

/* NS32202 word I/O is a very special case */


#endif
/* _IOMEM_DOT_H */
