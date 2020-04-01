#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devsys.h>
#include <audio.h>
#include <netdev.h>
#include <devmem.h>
#include <input.h>
#include <gpio.h>
#include <i2c.h>
#include <net_native.h>

/*
 *	System devices:
 *
 *	Minor	0	null
 *	Minor 	1	kmem    (kernel memory)
 *	Minor	2	zero
 *	Minor	3	proc
 *	Minor   4       mem     (physical memory)
 *	Minor	5	rtc
 *	Minor	6	platform (Platform/CPU specific ioctls)
 *	Minor   7	i2c
 *	Minor	8	gpio
 *	Minor	64	audio
 *	Minor	65	net_native
 *	Minor	66	input
 *
 *	Use Minor 128+ for platform specific devices
 */

int sys_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	unsigned char *addr = (unsigned char *) ptab;

	used(rawflag);
	used(flag);

	switch (minor) {
	case 0:
		return 0;
	case 1:
		if (uput((unsigned char *) udata.u_offset, udata.u_base,
			       udata.u_count))
			return -1;
		return umove(udata.u_count);
	case 2:
		if (udata.u_sysio)
			memset(udata.u_base, 0, udata.u_count);
		else
			uzero(udata.u_base, udata.u_count);
		return udata.u_count;
	case 3:
		if (udata.u_count > sizeof(struct p_tab))
			udata.u_count = sizeof(struct p_tab);
		if (udata.u_offset + udata.u_count > PTABSIZE * sizeof(struct p_tab))
			return 0;
		if (uput(addr + udata.u_offset, udata.u_base, udata.u_count))
			return -1;
		return umove(udata.u_count);
#ifdef CONFIG_DEV_MEM
        case 4:
                return devmem_read();
#endif
#ifdef CONFIG_RTC_FULL
	case 5:
		return platform_rtc_read();
#endif
#ifdef CONFIG_DEV_I2C
	case 7:
		return devi2c_read();
#endif
#ifdef CONFIG_NET_NATIVE
	case 65:
		return netdev_read(flag);
#endif
#ifdef CONFIG_INPUT
	case 66:
		return inputdev_read(flag);
#endif
	default:
		udata.u_error = ENXIO;
		return -1;
	}
}

int sys_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
	used(rawflag);
	used(flag);

	switch (minor) {
	case 0:
	case 2:
		return udata.u_count;
	case 1:
		if(uget((unsigned char *) udata.u_offset, udata.u_base,
			       udata.u_count))
			return -1;
		return umove(udata.u_count);
	case 3:
		udata.u_error = EINVAL;
		return -1;
#ifdef CONFIG_DEV_MEM
        case 4:
                return devmem_write();
#endif
#ifdef CONFIG_RTC_FULL
	case 5:
		return platform_rtc_write();
#endif
#ifdef CONFIG_DEV_I2C
	case 7:
		return devi2c_write();
#endif
#ifdef CONFIG_NET_NATIVE
	case 65:
		return netdev_write(flag);
#endif
#ifdef CONFIG_INPUT
	case 66:
		return inputdev_write(flag);
#endif
	default:
		udata.u_error = ENXIO;
		return -1;
	}
}

#define PIO_TABSIZE	1
#define PIO_ENTRYSIZE	2

int sys_ioctl(uint_fast8_t minor, uarg_t request, char *data)
{
#ifdef CONFIG_RTC_EXTENDED
	if (minor == 5)
		return platform_rtc_ioctl(request, data);
#endif
#ifdef CONFIG_DEV_PLATFORM
	if (minor == 6)
		return platform_dev_ioctl(request, data);
#endif
#ifdef CONFIG_DEV_GPIO
	if (minor == 8)
		return gpio_ioctl(request, data);
#endif
#ifdef CONFIG_AUDIO
	if (minor == 64)
		return audio_ioctl(request, data);
#endif
#ifdef CONFIG_NET_NATIVE
	if (minor == 65)
		return netdev_ioctl(request, data);
#endif
#ifdef CONFIG_INPUT
	if (minor == 66)
		return inputdev_ioctl(request, data);
#endif
	if (minor != 3)
		return -1;

	switch (request) {
	case PIO_TABSIZE:
		uputi(maxproc, data);
		break;

	case PIO_ENTRYSIZE:
		uputi(sizeof(struct p_tab), data);
		break;

	default:
		return -1;
	}
	return 0;
}

int sys_close(uint_fast8_t minor)
{
	used(minor);
#ifdef CONFIG_NET_NATIVE
	if (minor == 65)
		return netdev_close(minor);
#endif
#ifdef CONFIG_INPUT
	if (minor == 66)
		return inputdev_close();
#endif
	return 0;
}
