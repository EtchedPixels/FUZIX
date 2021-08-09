/* Provided by the W5x00 on top of the standard net interface hooks */
extern void w5x00_event(void);
extern void w5x00_poll(void);

/* Provided by the platform - modes are up to the platform */

/* Read control space */
extern uint8_t w5x00_readcb(uint16_t off);
extern uint16_t w5x00_readcw(uint16_t off);
/* Write a control space byte */
extern void w5x00_writecb(uint16_t off, uint8_t val);
extern void w5x00_writecw(uint16_t off, uint16_t val);
/* Read socket space */
extern uint8_t w5x00_readsb(uint8_t sock, uint16_t off);
extern uint16_t w5x00_readsw(uint8_t sock, uint16_t off);
/* Write socket space */
extern void w5x00_writesb(uint8_t sock, uint16_t off, uint8_t val);
extern void w5x00_writesw(uint8_t sock, uint16_t off, uint16_t val);

/* Block transfers kernel or user. */
extern void w5x00_bread(uint16_t bank, uint16_t off, void *p, uint16_t n);
/* Read a block to current user space keeping byte order */
extern void w5x00_breadu(uint16_t bank, uint16_t off, void *p, uint16_t n);
/* Write methods equivalent to the above */
extern void w5x00_bwrite(uint16_t bank, uint16_t off, void *p, uint16_t n);
extern void w5x00_bwriteu(uint16_t bank, uint16_t off, void *p, uint16_t n);

/* Called early on to do any initial set up for communications. For example
   to set up the SPI port */
extern void w5x00_setup(void);
