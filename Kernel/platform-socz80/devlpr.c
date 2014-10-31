#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devlpr.h>

int lpr_open(uint8_t minor, uint16_t flag)
{
    minor; flag; // shut up compiler
    udata.u_error = ENODEV;
    return (-1);
}

int lpr_close(uint8_t minor)
{
    minor; // shut up compiler
    udata.u_error = ENODEV;
    return (-1);
}

int lpr_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    minor; rawflag; flag; // shut up compiler
    udata.u_error = EINVAL;
    return (-1);
}

int lpr_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    minor; rawflag; flag; // shut up compiler
    udata.u_error = ENODEV;
    return (-1);
}
