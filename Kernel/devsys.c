#include <kernel.h>
#include <version.h>
#include <kdata.h>
#include <devsys.h>

/*
 *	System devices:
 *
 *	Minor	0	null
 *	Minor 	1	mem
 *	Minor	2	zero
 *	Minor	3	proc
 *
 *	Use Minor 128+ for platform specific devices
 */

int sys_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
  unsigned char *addr = (unsigned char *) ptab;

  rawflag;flag;

  switch(minor){
  case 0:
    return 0;
  case 1:
        return uputsys((unsigned char *)udata.u_offset, udata.u_count);
  case 2:
	if (udata.u_sysio)
		memset(udata.u_base, 0, udata.u_count);
	else
		uzero(udata.u_base, udata.u_count);
	return udata.u_count;
  case 3:
	if (udata.u_offset >= PTABSIZE * sizeof(struct p_tab))
		return 0;
	return uputsys(addr + udata.u_offset, udata.u_count);
  default:
    udata.u_error = ENXIO;
    return -1;
  }
}

int sys_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
  unsigned char *addr = (unsigned char *) ptab;

  rawflag;flag;

  switch(minor){
  case 0:
  case 2:
    return udata.u_count;
  case 1:
    /* FIXME: this needs to be a per CPU value */
    if (udata.u_offset < 0x0100) {
      udata.u_error = EPERM;
      return -1;
    }
    return ugetsys((unsigned char *)udata.u_offset, udata.u_count);
  case 3:
    udata.u_error = EINVAL;
    return -1;
  default:
    udata.u_error = ENXIO;
    return -1;
  }
}

#define PIO_TABSIZE	1
#define PIO_ENTRYSIZE	2

int sys_ioctl(uint8_t minor, uint16_t request, char *data)
{
	if (minor != 3) {
          udata.u_error = ENOTTY;
	  return -1;
        }

	switch (request) {
	case PIO_TABSIZE:
		uputw(maxproc, data);
		break;

	case PIO_ENTRYSIZE:
		uputw(sizeof(struct p_tab), data);
		break;

	default:
		udata.u_error = EINVAL;
		return (-1);
	}
	return 0;
}
