/* Should probably clean this up, add some of the Z180 defines and put it
   in lib ? */

__sfr __banked __at (Z180_IO_BASE + 0x00) ASCI_CNTLA0;   /* ASCI control register A channel 0          */
__sfr __banked __at (Z180_IO_BASE + 0x01) ASCI_CNTLA1;   /* ASCI control register A channel 1          */
__sfr __banked __at (Z180_IO_BASE + 0x02) ASCI_CNTLB0;   /* ASCI control register B channel 0          */
__sfr __banked __at (Z180_IO_BASE + 0x03) ASCI_CNTLB1;   /* ASCI control register B channel 0          */
__sfr __banked __at (Z180_IO_BASE + 0x04) ASCI_STAT0;    /* ASCI status register    channel 0          */
__sfr __banked __at (Z180_IO_BASE + 0x05) ASCI_STAT1;    /* ASCI status register    channel 1          */
__sfr __banked __at (Z180_IO_BASE + 0x06) ASCI_TDR0;     /* ASCI transmit data reg, channel 0          */
__sfr __banked __at (Z180_IO_BASE + 0x07) ASCI_TDR1;     /* ASCI transmit data reg, channel 1          */
__sfr __banked __at (Z180_IO_BASE + 0x08) ASCI_RDR0;     /* ASCI receive data reg,  channel 0          */
__sfr __banked __at (Z180_IO_BASE + 0x09) ASCI_RDR1;     /* ASCI receive data reg,  channel 0          */
__sfr __banked __at (Z180_IO_BASE + 0x12) ASCI_ASEXT0;   /* ASCI extension register channel 0          */
__sfr __banked __at (Z180_IO_BASE + 0x13) ASCI_ASEXT1;   /* ASCI extension register channel 1          */
__sfr __banked __at (Z180_IO_BASE + 0x1A) ASCI_ASTC0L;   /* ASCI time constant register channel 0 low  */
__sfr __banked __at (Z180_IO_BASE + 0x1B) ASCI_ASTC0H;   /* ASCI time constant register channel 0 high */
__sfr __banked __at (Z180_IO_BASE + 0x1C) ASCI_ASTC1L;   /* ASCI time constant register channel 1 low  */
__sfr __banked __at (Z180_IO_BASE + 0x1D) ASCI_ASTC1H;   /* ASCI time constant register channel 1 high */

extern void z180_setup(uint8_t use_timer) __z88dk_fastcall;
