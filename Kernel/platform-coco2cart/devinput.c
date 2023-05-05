#include <kernel.h>
#include <kdata.h>
#include <input.h>
#include <devinput.h>
#include <ps2mouse.h>

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

static uint8_t js_event[8];
static uint8_t jsdata[8];

int plt_input_read(uint8_t *slot)
{
    static uint8_t j;
    uint8_t r, k;
    if (remq(&kqueue, &r)) {
        remq(&kqueue, &k);
	*slot++ = KEYPRESS_CODE | r;
	*slot = k;
	return 2;
    }
    /* Alternate sticks so you get readings from each */
    if (j++ == 0) {
        jsread(jsdata);
        *js_event = STICK_ANALOG;
        js_event[1] = (jsdata[0] - 0x20) << 2;
        js_event[3] = (jsdata[2] - 0x20) << 2;
        if (jsdata[3] & 1)
            js_event[5] = BUTTON(0);
        else
            js_event[5] = 0;
    } else {
        *js_event = STICK_ANALOG | 1;
        js_event[1] = (jsdata[4] - 0x20) << 2;
        js_event[3] = (jsdata[6] - 0x20) << 2;
        if (jsdata[7] & 1)
            js_event[5] = BUTTON(0);
        else
            js_event[5] = 0;
        j = 0;
    }
    return 6;
}

void plt_input_wait(void)
{
    psleep(&kqueue);	/* We wake this on timers so it works for sticks */
}

void poll_input(void)
{
    wakeup(&kqueue);
}

int plt_input_write(uint8_t flag)
{
    flag;
    udata.u_error = EINVAL;
    return -1;
}
