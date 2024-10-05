#include <kernel.h>
#include <kdata.h>
#include <input.h>
#include <devinput.h>
#include <ps2mouse.h>

#ifdef CONFIG_RC2014_EXTREME
__sfr __banked __at 0x01B8	js1;
__sfr __banked __at 0x02B8	js2;
#else
__sfr __at 0x01 js1;
__sfr __at 0x02 js2;
#endif

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

uint8_t read_js(uint8_t *slot, uint8_t n, uint8_t r)
{
    uint8_t d = 0;
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

static uint8_t ps2event[4];
static uint8_t ps2pend = 0;

static void merge(uint8_t *a, uint8_t *b)
{
    int8_t p;
    int8_t n;
    int8_t r;
    uint8_t i = 0;

    while(i < 3) {
        p = (int8_t)*a;
        n = (int8_t)*b;
        r = p + n;
        /* Signed overflow check */
        if ((r ^ p) & (r ^ n) & 0x80)
            return;
        *a++ = (int8_t)r;
        b++;
    }
}

/* The PS/2 layer sent us a new event. On on 8bit micro we get rather
   too many events so we fuse them together */
void plt_ps2mouse_event(uint8_t *event)
{
    if (ps2pend == 0)
        memcpy(ps2event, event, 4);
    else {
        /* Update the button state */
        ps2event[1] = event[1];
        /* Merge the movement into the pending event */
        merge(ps2event + 2, event + 2);
    }
    ps2pend = 1;
    wakeup(&kqueue);
}

int plt_input_read(uint8_t *slot)
{
    uint8_t r, k;
    if (ps2pend) {
        irqflags_t irq = di();
        memcpy(slot, ps2event, 4);
        ps2pend = 0;
        irqrestore(irq);
        return 4;
    }
    if (remq(&kqueue, &r)) {
        remq(&kqueue, &k);
	*slot++ = KEYPRESS_CODE | r;
	*slot = k;
	return 2;
    }
    if (read_js(slot, 0, js1))
        return 2;
    if (read_js(slot, 1, js2))
        return 2;
    return 0;
}

void plt_input_wait(void)
{
    psleep(&kqueue);	/* We wake this on timers so it works for sticks */
}

void poll_input(void)
{
    if ((js1 & 63) != js_data[0] ||
        ((js2 & 63) != js_data[1]))
            wakeup(&kqueue);
}

int plt_input_write(uint8_t flag)
{
    flag;
    udata.u_error = EINVAL;
    return -1;
}
