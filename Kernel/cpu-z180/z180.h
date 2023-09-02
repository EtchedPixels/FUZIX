#ifndef __Z180_DOT_H__
#define __Z180_DOT_H__

void copy_and_map_proc(uint16_t *pageptr);

/* irqvector values */
#define Z180_INT_UNUSED     0xFF
#define Z180_INT0           0
#define Z180_INT1           1
#define Z180_INT2           2
#define Z180_INT_TIMER0     3
#define Z180_INT_TIMER1     4
#define Z180_INT_DMA0       5
#define Z180_INT_DMA1       6
#define Z180_INT_CSIO       7
#define Z180_INT_ASCI0      8
#define Z180_INT_ASCI1      9

#pragma portmode z180 /* use in0/out0 */

__sfr __at (Z180_IO_BASE + 0x0C) TIME_TMDR0L;   /* Timer data register,    channel 0L         */
__sfr __at (Z180_IO_BASE + 0x0D) TIME_TMDR0H;   /* Timer data register,    channel 0H         */
__sfr __at (Z180_IO_BASE + 0x0E) TIME_RLDR0L;   /* Timer reload register,  channel 0L         */
__sfr __at (Z180_IO_BASE + 0x0F) TIME_RLDR0H;   /* Timer reload register,  channel 0H         */
__sfr __at (Z180_IO_BASE + 0x10) TIME_TCR;      /* Timer control register                     */
__sfr __at (Z180_IO_BASE + 0x14) TIME_TMDR1L;   /* Timer data register,    channel 1L         */
__sfr __at (Z180_IO_BASE + 0x15) TIME_TMDR1H;   /* Timer data register,    channel 1H         */
__sfr __at (Z180_IO_BASE + 0x16) TIME_RLDR1L;   /* Timer reload register,  channel 1L         */
__sfr __at (Z180_IO_BASE + 0x17) TIME_RLDR1H;   /* Timer reload register,  channel 1H         */
__sfr __at (Z180_IO_BASE + 0x18) TIME_FRC;      /* Timer Free running counter                 */

__sfr __at (Z180_IO_BASE + 0x00) ASCI_CNTLA0;   /* ASCI control register A channel 0          */
__sfr __at (Z180_IO_BASE + 0x01) ASCI_CNTLA1;   /* ASCI control register A channel 1          */
__sfr __at (Z180_IO_BASE + 0x02) ASCI_CNTLB0;   /* ASCI control register B channel 0          */
__sfr __at (Z180_IO_BASE + 0x03) ASCI_CNTLB1;   /* ASCI control register B channel 0          */
__sfr __at (Z180_IO_BASE + 0x04) ASCI_STAT0;    /* ASCI status register    channel 0          */
__sfr __at (Z180_IO_BASE + 0x05) ASCI_STAT1;    /* ASCI status register    channel 1          */
__sfr __at (Z180_IO_BASE + 0x06) ASCI_TDR0;     /* ASCI transmit data reg, channel 0          */
__sfr __at (Z180_IO_BASE + 0x07) ASCI_TDR1;     /* ASCI transmit data reg, channel 1          */
__sfr __at (Z180_IO_BASE + 0x08) ASCI_RDR0;     /* ASCI receive data reg,  channel 0          */
__sfr __at (Z180_IO_BASE + 0x09) ASCI_RDR1;     /* ASCI receive data reg,  channel 0          */
__sfr __at (Z180_IO_BASE + 0x12) ASCI_ASEXT0;   /* ASCI extension register channel 0          */
__sfr __at (Z180_IO_BASE + 0x13) ASCI_ASEXT1;   /* ASCI extension register channel 1          */
__sfr __at (Z180_IO_BASE + 0x1A) ASCI_ASTC0L;   /* ASCI time constant register channel 0 low  */
__sfr __at (Z180_IO_BASE + 0x1B) ASCI_ASTC0H;   /* ASCI time constant register channel 0 high */
__sfr __at (Z180_IO_BASE + 0x1C) ASCI_ASTC1L;   /* ASCI time constant register channel 1 low  */
__sfr __at (Z180_IO_BASE + 0x1D) ASCI_ASTC1H;   /* ASCI time constant register channel 1 high */

__sfr __at (Z180_IO_BASE + 0x0A) CSIO_CNTR;     /* CSI/O control/status register              */
__sfr __at (Z180_IO_BASE + 0x0B) CSIO_TRDR;     /* CSI/O transmit/receive data register       */

__sfr __at (Z180_IO_BASE + 0x36) Z180_RCR;	/* Refresh control register */
__sfr __at (Z180_IO_BASE + 0x3E) Z180_OMCR;	/* Output mode control register */
__sfr __at (Z180_IO_BASE + 0x3F) Z180_ICR;	/* I/O control register */
__sfr __at (Z180_IO_BASE + 0x1E) Z180_CMR;	/* Clock multiplier register */
__sfr __at (Z180_IO_BASE + 0x1F) Z180_CCR;	/* Clock divide/standby register */
__sfr __at (Z180_IO_BASE + 0x32) Z180_DCNTL;	/* DMA/WAIT control */

/* On Z80182 the MIMIC, ESCC, PIA and MISC registers are at fixed addresses */
__sfr __at (0xE0)                ESCC_CTRL_A;   /* ESCC Channel A control register            */
__sfr __at (0xE1)                ESCC_DATA_A;   /* ESCC Channel A data register               */
__sfr __at (0xE2)                ESCC_CTRL_B;   /* ESCC Channel B control register            */
__sfr __at (0xE3)                ESCC_DATA_B;   /* ESCC Channel B data register               */

__sfr __at (0xED)                PORT_A_DDR;    /* Port A data direction register             */
__sfr __at (0xEE)                PORT_A_DATA;   /* Port A data register                       */
__sfr __at (0xE4)                PORT_B_DDR;    /* Port B data direction register             */
__sfr __at (0xE5)                PORT_B_DATA;   /* Port B data register                       */
__sfr __at (0xDD)                PORT_C_DDR;    /* Port C data direction register             */
__sfr __at (0xDE)                PORT_C_DATA;   /* Port C data register                       */

#endif
