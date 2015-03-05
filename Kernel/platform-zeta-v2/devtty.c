#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>
#include <zeta-v2.h>

char tbuf1[TTYSIZ];

#ifdef CONFIG_PPP
char tbufp[TTYSIZ];
#endif

struct  s_queue  ttyinq[NUM_DEV_TTY+1] = {       /* ttyinq[0] is never used */
    {   NULL,    NULL,    NULL,    0,        0,       0    },
    {   tbuf1,   tbuf1,   tbuf1,   TTYSIZ,   0,   TTYSIZ/2 },
#ifdef CONFIG_PPP
    {   tbufp,   tbufp,   tbufp,   TTYSIZ,   0,   TTYSIZ/2 },
#endif
};

/* FIXME: implement */
void tty_setup(uint8_t minor)
{
    minor;
}

/* FIXME: implement (although /DCD is hardwired to GND) */
int tty_carrier(uint8_t minor)
{
    minor;
    return 1;
}

void tty_interrupt(void)
{
	uint8_t reg = UART0_LSR;
	if (reg & 0x01) {
		/* data available */
		reg = UART0_RBR;
		tty_inproc(1, reg);
	}
}

#ifdef CONFIG_PPP
void tty_poll_ppp(void)
{
    while(PROPIO2_STAT & 0x20)
        tty_inproc(3, PROPIO2_TERM);
}
#endif

void tty_putc(uint8_t minor, unsigned char c)
{
	if (minor == 1) {
		while(!(UART0_LSR & 0x20));
		UART0_THR = c;
#ifdef CONFIG_PPP
	} else if (minor = 2) {
		/* FIXME: implement */
#endif
	}
}

void tty_sleeping(uint8_t minor)
{
	minor;
}

ttyready_t tty_writeready(uint8_t minor)
{
	uint8_t reg;
	if (minor == 1) {
		reg = UART0_LSR;
		return (reg & 0x20) ? TTY_READY_NOW : TTY_READY_SOON;
	}
	return TTY_READY_NOW;
}

/* kernel writes to system console -- never sleep! */
void kputchar(char c)
{
    tty_putc(TTYDEV - 512, c);
    if(c == '\n')
        tty_putc(TTYDEV - 512, '\r');
}
