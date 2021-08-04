/* Provided by the W5300 on top of the standard net interface hooks */
extern void w5300_event(void);
extern void w5300_poll(void);

/*
 *	WARNING: Although it isn't well documented the FIFO on the W5300
 *	advances on the low byte read. This means your read/readn routines
 *	must always access the byte high then low when in 8bit direct or
 *	indirect mode.
 */

/* Read a word: turn it native endian */
extern uint16_t w5300_read(uint16_t off);
/* Read a word: already native endian */
extern uint16_t w5300_readn(uint16_t off);
/* Write a word: turn native endian to big endian */
extern void w5300_write(uint16_t off, uint16_t n);
/* Write a word; already native endian */
extern void w5300_writen(uint16_t off, uint16_t n);

/* Called early on to do any initial set up for communications. For example
   to put the device into indirect mode */
extern void w5300_setup(void);
