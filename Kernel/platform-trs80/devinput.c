#include <kernel.h>
#include <kdata.h>
#include <input.h>
#include <devinput.h>

__sfr __at 0x00 stick;
uint8_t old_stick;

static char buf[32];
static struct s_queue kqueue = {
 buf, buf, buf, sizeof(buf), 0, sizeof(buf) / 2
};

/* Queue a character to the input device */
void queue_input(uint8_t c)
{
    insq(&kqueue, c);
    wakeup(&kqueue);
}

int plt_input_read(uint8_t *slot)
{
    uint8_t r, k;
    if (remq(&kqueue, &r)) {
        remq(&kqueue, &k);
	*slot++ = KEYPRESS_CODE | r;
	*slot++ = k;
	return 2;
    }

    r = ~stick;
    if (r == old_stick)
        return 0;

    old_stick = r;

    k = 0;
    /* Legacy joystick encoding U|D = FIRE */
    if ((r & 3) == 3) {
        r &= ~3;
        r |= 16;
    }
    if (r & 1)
        k = STICK_DIGITAL_U;
    if (r & 2)
        k |= STICK_DIGITAL_D;
    if (r & 4)
        k |= STICK_DIGITAL_L;
    if (r & 8)
        k |= STICK_DIGITAL_R;
    if (r & 16)
        k |= BUTTON(0);
    *slot++ = STICK_DIGITAL;
    *slot++ = k;
    return 2;
}

void plt_input_wait(void)
{
    psleep(&kqueue);	/* We wake this on timers so it works for sticks */
}

int plt_input_write(uint8_t flag)
{
    flag;
    udata.u_error = EINVAL;
    return -1;
}

void poll_input(void)
{
    if (~stick != old_stick)
        wakeup(&kqueue);
}
