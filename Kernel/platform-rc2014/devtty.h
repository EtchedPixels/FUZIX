#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

#define SIO_SIO 1

void tty_putc(uint8_t minor, unsigned char c);
void tty_pollirq_sio(void);
void tty_pollirq_acia(void);

void sio_init(void);
void acia_init(void);

#ifdef CONFIG_PPP
void tty_poll_ppp(void);
#endif
#endif
