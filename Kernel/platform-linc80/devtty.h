#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

#define SIO0_BASE 0x00
__sfr __at (SIO0_BASE + 0) SIOA_D;
__sfr __at (SIO0_BASE + 1) SIOB_D;
__sfr __at (SIO0_BASE + 2) SIOA_C;
__sfr __at (SIO0_BASE + 3) SIOB_C;

extern void sio2_otir(uint8_t port) __z88dk_fastcall;

void tty_putc(uint8_t minor, unsigned char c);

extern void sio2a_txqueue(uint8_t c) __z88dk_fastcall;
extern void sio2a_flow_control_on(void);
extern uint16_t sio2a_rx_get(void);
extern uint8_t sio2a_error_get(void);
extern uint8_t sio2a_rxl;
extern uint8_t sio2a_txl;
extern uint8_t sio2a_wr5;

extern void sio2b_txqueue(uint8_t c) __z88dk_fastcall;
extern void sio2b_flow_control_on(void);
extern uint16_t sio2b_rx_get(void);
extern uint8_t sio2b_error_get(void);
extern uint8_t sio2b_rxl;
extern uint8_t sio2b_txl;
extern uint8_t sio2b_wr5;

extern void tty_drain_sio(void);


#endif
