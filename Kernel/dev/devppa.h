/*
 *	PPA ZIP drives
 */

extern uint8_t ppa_init(void);

/* Caller provides the I/O methods. These should provide
   the bits in IBM parallel port form including which
   bits are inverted */
extern void ppa_write_data(uint8_t data);
extern void ppa_write_ctrl(uint8_t ctrl);
extern uint8_t ppa_read_status(void);

/* 512 byte burst read/write possibly to other banks */
extern void ppa_block_read(void);
extern void ppa_block_write(void);
