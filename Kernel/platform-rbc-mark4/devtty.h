#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__
void tty_pollirq_asci0(void);
void tty_pollirq_asci1(void);

#ifdef CONFIG_PROPIO2
void tty_poll_propio2(void);
#endif
#endif
