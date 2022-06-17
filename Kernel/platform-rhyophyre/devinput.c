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
    return 0;
}

void plt_input_wait(void)
{
    psleep(&kqueue);	/* We wake this on timers so it works for polled I/O */
}

void poll_input(void)
{
}

int plt_input_write(uint8_t flag)
{
    flag;
    udata.u_error = EINVAL;
    return -1;
}
