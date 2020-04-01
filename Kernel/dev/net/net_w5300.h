/* Provided by the W5300 on top of the standard net interface hooks */
extern void w5300_event(void);
extern void w5300_poll(void);

/* Read a word: turn it native endian */
extern uint16_t w5300_readw(uint16_t off);
/* Write a word: turn native endian to big endian */
extern void w5300_writew(uint16_t off, uint16_t n);
/* Write a word; already native endian */
extern void w5300_writebe(uint16_t off, uint16_t n);

/* Called early on to do any initial set up for communications. For example
   to set up the SPI port or put the device into indirect mode */
extern void w5300_setup(void);
