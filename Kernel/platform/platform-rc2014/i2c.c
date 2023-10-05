#include <kernel.h>
#include <i2c.h>
#include <pcf8584.h>

int plt_i2c_msg(struct i2c_msg *m, uint8_t *kbuf)
{
    if (m->bus)
        return -ENODEV;
    if (m->addr & 1)
        return pcf8584_rx_msg(m->addr, kbuf, m->len);
    else
        return pcf8584_tx_msg(m->addr, kbuf, m->len);
}
