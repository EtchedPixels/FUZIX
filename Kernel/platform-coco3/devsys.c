#include <kernel.h>
#include <kdata.h>
#include <cpu_ioctl.h>

int platform_dev_ioctl(uarg_t request, char *data)
{
    if (!valaddr((unsigned char *)data, 2))
	goto bad;
    switch (request){
    case CPUIOC_6809SWI2:
	_uputw(data, (uint16_t *)0xfc);
	break;
    case CPUIOC_6809SWI3:
	_uputw(data, (uint16_t *)0xfe);
	break;
    default:
	goto bad;
    }
    return 0;
 bad:
    udata.u_error = EINVAL;
    return -1;
}
