#define _DEVTTY_PRIVATE
#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include "config.h"
#include <z180.h>
#ifdef CONFIG_P112_FLOPPY
#include "devfd.h"
#endif
#include <devide.h>
#include <ds1302.h>

void init_hardware_c(void)
{
    ramsize = 1024;
    procmem = 1024 - 64 - (DEV_RD_RAM_PAGES<<2);
    /* zero out the initial bufpool */
    memset(bufpool, 0, (char*)bufpool_end - (char*)bufpool);
}

void pagemap_init(void)
{
    int i;

    /* P112 has RAM across the full physical 1MB address space
     * First 64K is used by the kernel. 
     * Each process gets the full 64K for now.
     * Page size is 4KB. */
    for(i = 0x10; i < ((1024 - (DEV_RD_RAM_PAGES<<2))>>2); i+=0x10)
        pagemap_add(i);
}

void map_init(void)
{
    /* clone udata and stack into a regular process bank, return with common memory
       for the new process loaded */
    copy_and_map_proc(&init_process->p_page);
    /* kernel bank udata (0x300 bytes) is never used again -- could be reused? */
}

/* called from devices.c to set up tty5 */
void tty_hw_init(void)
{
    /* Note: setup for tty1, tty2 (ESCC) is found in p112.s */

    /* setup for tty5 (16550) for 115200,8n1 */
    /* 37C655GT feature: Out2 in MCR is the master UART interrupt enable */
    TTY_COM1_MCR = 0x0B; /* disable loopback, set DTR and RTS, enable Out2 */
    TTY_COM1_LCR = 0x80; /* enable DLAB to access divisor */
    TTY_COM1_DLL = 0x01; /* program divisor for 115200bps */
    TTY_COM1_DLM = 0x00;
    TTY_COM1_LCR = 0x03; /* disable DLAB, program 8 bits, no parity */
    TTY_COM1_FCR = 0x07; /* enable and clear FIFOs, interrupt threshold 1 byte */
    TTY_COM1_IER = 0x01; /* enable only receive interrupts */
}

void device_init(void)
{
    devide_init();
    ds1302_init();
    tty_hw_init();
}

uint8_t plt_param(char *p)
{
    used(p);
    return 0;
}
