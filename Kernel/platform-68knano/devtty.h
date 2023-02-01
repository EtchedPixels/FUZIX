#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

extern void uartspi_tx(uint16_t byte);
extern uint_fast8_t uartspi_rx(void);
extern void uartspi_tx_sector(uint8_t *ptr);
extern void uartspi_rx_sector(uint8_t *ptr);

#endif
