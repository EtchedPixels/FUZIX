#ifndef MSP430FR5969_LD_H
#define MSP430FR5969_LD_H

/* Must be 0x0080 plus a multiple of 0x0200 */
#define USER_BASE 0xa380
#define UDATA_BASE (USER_BASE - 0x0200)

#endif

