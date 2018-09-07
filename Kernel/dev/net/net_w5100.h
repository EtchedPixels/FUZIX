/* Provided by the W5100 on top of the standard net interface hooks */
extern void w5100_event(void);
extern void w5100_poll(void);

/* Provided by the platform */

/* Read a byte sized register. Platform can use any mode it likes - SPI, direct
   or inidrect */
extern uint8_t w5100_readb(uint16_t off);
/* Read a word the same way: turn it native endian */
extern uint16_t w5100_readw(uint16_t off);
/* Read a block to kernel keeping byte order */
extern void w5100_bread(uint16_t off, uint8_t *p, uint16_t n);
/* Read a block to current user space keeping byte order */
extern void w5100_breadu(uint16_t off, uint8_t *p, uint16_t n);
/* Write methods equivalent to the above */
extern void w5100_writeb(uint16_t off, uint8_t n);
extern void w5100_writew(uint16_t off, uint16_t n);
extern void w5100_bwrite(uint16_t off, uint8_t *p, uint16_t n);
extern void w5100_bwriteu(uint16_t off, uint8_t *p, uint16_t n);

/* Called early on to do any initial set up for communications. For example
   to set up the SPI port or put the device into indirect mode */
extern void w5100_setup(void);
