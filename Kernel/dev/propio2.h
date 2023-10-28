#ifndef _DEV_PROPIO2
#define _DEV_PROPIO2
/*
 *	Propeller I/O 2 card interfaces
 */

extern void prop_tty_poll(uint_fast8_t minor);
extern void prop_tty_write(uint_fast8_t c);
//extern ttyready_t prop_tty_writeready(void);

extern uint_fast8_t prop_sd_probe(void);

extern void plt_prop_sd_read(uint8_t *dptr);
extern void plt_prop_sd_write(uint8_t *dptr);

#endif
