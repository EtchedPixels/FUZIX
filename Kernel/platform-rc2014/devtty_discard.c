#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <devtty.h>
#include <rc2014.h>

extern unsigned char sio_type;

/* uart0_init - detect UART type, print it, enable FIFO if present
 */
void sio_init(void) {
        kprintf("UART0 type: SIO/2.\n");
}

void acia_init(void) {
        kprintf("UART0 type: ACIA.\n");
}


