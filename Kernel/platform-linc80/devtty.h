#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

#define SIO0_BASE 0x00
__sfr __at (SIO0_BASE + 0) SIOA_D;
__sfr __at (SIO0_BASE + 1) SIOB_D;
__sfr __at (SIO0_BASE + 2) SIOA_C;
__sfr __at (SIO0_BASE + 3) SIOB_C;

extern void sio2_otir(uint8_t port) __z88dk_fastcall;

void tty_putc(uint8_t minor, unsigned char c);

extern void sioa_txqueue(uint8_t c) __z88dk_fastcall;
extern void sioa_flow_control_on(void);
extern void sioa_flow_control_off(void);
extern uint16_t sioa_rx_get(void);
extern uint8_t sioa_error_get(void);

extern uint8_t sio_dropdcd[2];
extern uint8_t sio_flow[2];
extern uint8_t sio_rxl[2];
extern uint8_t sio_state[2];
extern uint8_t sio_txl[2];
extern uint8_t sio_wr5[2];

extern void siob_txqueue(uint8_t c) __z88dk_fastcall;
extern void siob_flow_control_on(void);
extern void siob_flow_control_off(void);
extern uint16_t siob_rx_get(void);
extern uint8_t siob_error_get(void);

extern void tty_drain_sio(void);


#endif
