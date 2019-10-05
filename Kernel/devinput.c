#include <kernel.h>
#include <kdata.h>
#include <input.h>

#ifdef CONFIG_INPUT

uint8_t keyboard_grab;
static uint8_t metamap[INPUT_MAX_META];	/* forces it 0 terminated */

uint_fast8_t input_match_meta(uint_fast8_t c)
{
    uint8_t *cp = metamap;
    if (!c)
        return 0;
    while(cp < metamap + INPUT_MAX_META) {
        if (*cp++ == c)
            return 1;
    }
    return 0;
}

int inputdev_read(uint_fast8_t flag)
{
    uint8_t m[8];
    int s;

    while(1)
    {
        s = platform_input_read(m);
        if (s > 0) {
            udata.u_count = min(udata.u_count, s);
            if (uput(m, udata.u_base, udata.u_count))
                return -1;
            return udata.u_count;
        }
        if (s == 0 && (flag & O_NDELAY)) {
            udata.u_error = EAGAIN;
            return -1;
        }
        if (chksigs()) {
            udata.u_error = EINTR;
            return -1;
        }
        if (s < 0)
            return -1;
        platform_input_wait();
    }
}

int inputdev_write(uint_fast8_t flag)
{
    return platform_input_write(flag);
}

int inputdev_ioctl(uarg_t request, char *data)
{
    uint_fast8_t r;
#ifdef CONFIG_INPUT_GRABMAX
    if (request == INPUT_GRABKB) {
        r = ((uint8_t)data) & 0x03;
        if (r > CONFIG_INPUT_GRABMAX) {
            udata.u_error = EOPNOTSUPP;
            return -1;
        }
        keyboard_grab = r;
        return 0;
    }
#endif
    if (request == INPUT_SETMETA)
        return uget(data, metamap, sizeof(metamap));
    return -1;
}

int inputdev_close(void)
{
    keyboard_grab = INPUT_GRAB_NONE;
    return 0;
}

#endif
