/*
 *	68HC11 SCI support routines
 */

extern uint8_t sci_tx_queue(uint8_t c);
extern void sci_tx_console(uint8_t c);
extern uint8_t sci_tx_space(void);
extern int16_t sci_rx_get(void);
extern uint8_t sci_error_get(void);

extern uint8_t sci_rxl;		/* 0-127 bytes queued for rx */
extern uint8_t sci_txl;		/* 0-127 bytes queued for tx */
extern uint16_t sci_rxp;	/* 256 byte aligned lower 128 bytes */
extern uint16_t sci_rxe;	/* rxp is the head rxe the tail */
extern uint16_t sci_txp;	/* Ditto but upper 128 bytes */
extern uint16_t sci_txe;

