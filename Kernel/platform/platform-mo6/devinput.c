#include <kernel.h>
#include <kdata.h>
#include <input.h>
#include <devtty.h>
#include <devinput.h>

int plt_input_read(uint8_t *buf)
{
    /* TODO joystick */
    return 0;
}

void plt_input_wait(void)
{
    inputwait++;
    psleep(&inputwait);	/* We wake this on timers so it works for polled */
    inputwait--;
}

int plt_input_write(uint8_t flag)
{
    flag;
    udata.u_error = EINVAL;
    return -1;
}
