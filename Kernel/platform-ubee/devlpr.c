#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devlpr.h>
#include <ubee.h>

__sfr __at 0x00 lpdata;		/* I/O 0 PIO A data */

int lpr_open(uint8_t minor, uint16_t flag)
{
    used(flag);
    if (ubee_parallel != UBEE_PARALLEL_LPR || minor) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int lpr_close(uint8_t minor)
{
    used(minor);
    return 0;
}

static volatile uint8_t lpready = 1;

void lpr_interrupt(void)
{
    lpready = 1;
    wakeup(&lpready);
}

int lpr_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    used(minor);
    used(rawflag);

    /* Unusually for an 8bit micro  the MicroBee has interrupt driven
       parallel managed via the Z80PIOA. It's not always used for a printer
       so we do need to fix interactions if we add other devices for that
       port and interlock them */
    while(udata.u_done < udata.u_count) {
        /* Avoid IRQ race */
        irqflags_t irq = di();
        if (!lpready && psleep_flags_io(&lpready, flag)) {
            irqrestore(irq);
            break;
        }
        irqrestore(irq);
        lpready = 0;
        lpdata = ugetc(udata.u_base++);
        udata.u_done++;
    }
    return udata.u_done;
}
