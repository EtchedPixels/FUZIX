#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

void tty_putc(uint_fast8_t minor, uint_fast8_t c);
void tty_drain_sio(void);

extern void sioa_txqueue(uint8_t c);
extern void sioa_flow_on(void);
extern void sioa_flow_off(void);
extern uint16_t sioa_rx_get(void);
extern uint8_t sioa_error_get(void);

extern void siob_txqueue(uint8_t c);
extern void siob_flow_on(void);
extern void siob_flow_off(void);
extern uint16_t siob_rx_get(void);
extern uint8_t siob_error_get(void);

extern void sioc_txqueue(uint8_t c);
extern void sioc_flow_on(void);
extern void sioc_flow_off(void);
extern uint16_t sioc_rx_get(void);
extern uint8_t sioc_error_get(void);

extern void siod_txqueue(uint8_t c);
extern void siod_flow_on(void);
extern void siod_flow_off(void);
extern uint16_t siod_rx_get(void);
extern uint8_t siod_error_get(void);

extern uint8_t sio_dropdcd[4];
extern uint8_t sio_flow[4];
extern uint8_t sio_rxl[4];
extern uint8_t sio_state[4];
extern uint8_t sio_txl[4];
extern uint8_t sio_wr5[4];


#endif
