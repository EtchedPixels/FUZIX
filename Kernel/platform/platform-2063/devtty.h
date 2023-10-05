#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

void tty_putc(uint8_t minor, unsigned char c);
void tty_drain_sio(void);
int rctty_open(uint8_t minor, uint16_t flag);

extern void sioa_txqueue(uint8_t c) __z88dk_fastcall;
extern void sioa_flow_control_on(void);
extern void sioa_flow_control_off(void);
extern uint16_t sioa_rx_get(void);
extern uint8_t sioa_error_get(void);

extern void siob_txqueue(uint8_t c) __z88dk_fastcall;
extern void siob_flow_control_on(void);
extern void siob_flow_control_off(void);
extern uint16_t siob_rx_get(void);
extern uint8_t siob_error_get(void);

extern uint8_t sio_dropdcd[2];
extern uint8_t sio_flow[2];
extern uint8_t sio_rxl[2];
extern uint8_t sio_state[2];
extern uint8_t sio_txl[2];
extern uint8_t sio_wr5[2];

extern int8_t vt_twidth;
extern int8_t vt_tright;
extern uint8_t vdptype;

extern int vdptty_ioctl(uint8_t minor, uarg_t arg, char *ptr);

#endif
