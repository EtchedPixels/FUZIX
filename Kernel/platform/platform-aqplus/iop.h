/*
 *	I/O coprocessor interface for the Aq+
 */

/* I/O interface */
extern void iop_cmd(uint_fast8_t cmd);
extern void iop_data(uint_fast8_t cmd);
extern uint_fast8_t iop_rx(void);
extern void iop_read(uint8_t *buf, unsigned len);
extern void iop_write(uint8_t *buf, unsigned len);
extern void iop_probe(void);

/* Input device helpers */
extern void queue_input(uint8_t c);

/* Disk I/O block transfers */
extern void iop_data_in(uint8_t *p);
extern void iop_data_out(uint8_t *p);
