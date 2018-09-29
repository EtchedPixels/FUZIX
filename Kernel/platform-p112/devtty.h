#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

void tty_hw_init(void);
void tty_pollirq_escc(void);
void tty_pollirq_asci0(void);
void tty_pollirq_asci1(void);
void tty_pollirq_com1(void);

#ifdef _DEVTTY_PRIVATE

/* it's our old friend the 16550AF */
#define COM1_BASE        0x98
#define COM1_RBR         (COM1_BASE + 0) /* DLAB=0, read only */
#define COM1_THR         (COM1_BASE + 0) /* DLAB=0, write only */
#define COM1_IER         (COM1_BASE + 1) /* DLAB=0 */
#define COM1_IIR         (COM1_BASE + 2) /* read only */
#define COM1_FCR         (COM1_BASE + 2) /* write only */
#define COM1_LCR         (COM1_BASE + 3)
#define COM1_MCR         (COM1_BASE + 4)
#define COM1_LSR         (COM1_BASE + 5)
#define COM1_MSR         (COM1_BASE + 6)
#define COM1_SCR         (COM1_BASE + 7)
#define COM1_DLL         (COM1_BASE + 0) /* DLAB=1 */
#define COM1_DLM         (COM1_BASE + 1) /* DLAB=1 */

__sfr __at COM1_RBR TTY_COM1_RBR;
__sfr __at COM1_THR TTY_COM1_THR;
__sfr __at COM1_IER TTY_COM1_IER;
__sfr __at COM1_IIR TTY_COM1_IIR;
__sfr __at COM1_FCR TTY_COM1_FCR;
__sfr __at COM1_LCR TTY_COM1_LCR;
__sfr __at COM1_MCR TTY_COM1_MCR;
__sfr __at COM1_LSR TTY_COM1_LSR;
__sfr __at COM1_MSR TTY_COM1_MSR;
__sfr __at COM1_SCR TTY_COM1_SCR;
__sfr __at COM1_DLL TTY_COM1_DLL;
__sfr __at COM1_DLM TTY_COM1_DLM;

#endif
#endif
