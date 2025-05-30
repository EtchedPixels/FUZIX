#ifndef _SCI_H
#define _SCI_HJ

extern void sci_tx_console(uint8_t c);
extern uint8_t sci_tx_space();
extern void sci_tx_queue(uint8_t c);
extern int16_t sci_rx_get();

#endif
