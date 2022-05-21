#include <kernel.h>
#include <kdata.h>
#include <input.h>
#include <devinput.h>

static uint8_t buf[32];

static struct s_queue kqueue = {
 buf, buf, buf, sizeof(buf), 0, sizeof(buf) / 2
};

/* Queue a character to the input device */
void queue_input(uint8_t c)
{
    insq(&kqueue, c);
    wakeup(&kqueue);
}

static uint8_t js_data[2]= {255, 255};

uint8_t read_js(uint8_t *slot, uint8_t n)
{
    uint8_t d = 0;
    uint8_t r = in(n + 1);
    r &= 63;
    if (r == js_data[n])
        return 0;
    js_data[n] = r;
    if (r & 1)
        d = STICK_DIGITAL_U;
    if (r & 2)
        d |= STICK_DIGITAL_D;
    if (r & 4)
        d |= STICK_DIGITAL_L;
    if (r & 8)
        d |= STICK_DIGITAL_R;
    if (r & 16)
        d |= BUTTON(0);
    if (r & 32)
        d |= BUTTON(1);
    *slot++ = STICK_DIGITAL | (n + 1);
    *slot = d;
    return 2;
}

int plt_input_read(uint8_t *slot)
{
    uint8_t r, k;
    if (remq(&kqueue, &r)) {
        remq(&kqueue, &k);
	*slot++ = KEYPRESS_CODE | r;
	*slot = k;
	return 2;
    }
    if (read_js(slot, 0))
        return 2;
    if (read_js(slot, 1))
        return 2;
    return 0;
}

void plt_input_wait(void)
{
    psleep(&kqueue);	/* We wake this on timers so it works for sticks */
}

void poll_input(void)
{
    if ((in(1) & 63) != js_data[0] ||
        ((in(2) & 63) != js_data[1]))
            wakeup(&kqueue);
}

int plt_input_write(uint8_t flag)
{
    udata.u_error = EINVAL;
    return -1;
}
