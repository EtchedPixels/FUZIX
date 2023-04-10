#include <kernel.h>
#include <kdata.h>
#include <i2c.h>

#ifdef CONFIG_DEV_I2C

int devi2c_ioctl(uarg_t arg, char *data)
{
    struct i2c_msg msg;
    char buf[64];
    int r;

    if (arg != I2C_MSG)
        return -1;
    if (uget(data, &msg, sizeof(msg)))
        return -1;
    if (msg.len > 64) {
        udata.u_error = EMSGSIZE;
        return -1;
    }
    if (valaddr(msg.data, msg.len, (msg.addr & 1)))
        return -1;
    if ((msg.addr & 1) == 0)
        uget(msg.data, buf, msg.len);
    r = plt_i2c_msg(&msg, buf);
    if (r == 0 && (msg.addr & 1))
        uput(buf, msg.data, msg.len);
    if (r == 0)
        return r;
    udata.u_error = -r;
    return -1;
}

#endif
