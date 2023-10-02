/*

 CoCoSDC driver
 (c)2015 Brett M. Gordon GPL2

 * Needs work on extended SDC API stuff.
 * init / mounting stuff really needs to set/update blkdev structure for size
 * need to get rawmode=1 and 2 working.

*/

#include <kernel.h>
#include <kdata.h>
#include <tinydisk.h>
#include <devsdc.h>
#include <printf.h>


#define SDC_REG_BASE 0xff40
#define SDC_REG_CTL (SDC_REG_BASE+0x00 )
#define SDC_REG_DATA (SDC_REG_BASE+0x02 )
#define SDC_REG_FCTL (SDC_REG_BASE+0x03 )
#define SDC_REG_CMD  (SDC_REG_BASE+0x08 )
#define SDC_REG_STAT (SDC_REG_BASE+0x08 )
#define SDC_REG_PARAM1 (SDC_REG_BASE+0x09 )
#define SDC_REG_PARAM2 (SDC_REG_BASE+0x0a )
#define SDC_REG_PARAM3 (SDC_REG_BASE+0x0b )

#define SDC_BUSY  0x01
#define SDC_READY 0x02
#define SDC_FAIL  0x80

#define sdc_reg_ctl *((volatile uint8_t *)SDC_REG_CTL)
#define sdc_reg_data *((volatile uint8_t *)SDC_REG_DATA)
#define sdc_reg_fctl *((volatile uint8_t *)SDC_REG_FCTL)
#define sdc_reg_cmd *((volatile uint8_t *)SDC_REG_CMD)
#define sdc_reg_stat *((volatile uint8_t *)SDC_REG_STAT)
#define sdc_reg_param1 *((volatile uint8_t *)SDC_REG_PARAM1)
#define sdc_reg_param2 *((volatile uint8_t *)SDC_REG_PARAM2)
#define sdc_reg_param3 *((volatile uint8_t *)SDC_REG_PARAM3)

/* a "simple" internal function pointer to which transfer
   routine to use.
*/
typedef void (*sdc_transfer_function_t)(unsigned char *addr);

/* blkdev method: transfer sectors */
static int sdc_xfer(uint8_t dev, bool is_read, uint32_t lba, uint8_t * dptr)
{
	uint8_t ret = 0;	/* our return value, preset to failure */
	uint8_t *ptr;		/* points to 32 bit lba in blk op */
	int i;			/* iterator */
	uint8_t t;		/* temporarory sdc status holder */
	uint8_t cmd;		/* holds SDC command value */
	sdc_transfer_function_t fptr;	/* holds which xfer routine we want */

	/* turn on uber-secret SDC LBA mode */
	sdc_reg_ctl = 0x43;

	/* we get 256 bytes per lba in SDC so convert */
	lba *= 2;

	/* setup cmd pointer and command value from blk_op */
	if (is_read) {
		cmd = 0x80;
		fptr = devsdc_read;
	} else {
		cmd = 0xa0;
		fptr = devsdc_write;
	}

	/* or in our drive value */
	cmd |= dev;

	/* get/send two sectors (512 bytes) of data */
	for (i = 0; i < 2; i++) {
		/* load up registers */
		ptr = ((uint8_t *) (&lba)) + 1;
		sdc_reg_param1 = ptr[0];
		sdc_reg_param2 = ptr[1];
		sdc_reg_param3 = ptr[2];
		sdc_reg_cmd = cmd;
		asm("\texg x,x\n");	/* delay 16us minimum */
		asm("\texg x,x\n");
		/* wait till SDC is ready */
		do {
			t = sdc_reg_stat;
			if (t & SDC_FAIL)
				goto fail;
		} while (!(t & SDC_READY));
		/* do our low-level xfer function */
		fptr(dptr);
		/* and wait will ready again, and test for failure */
		do {
			t = sdc_reg_stat;
			if (t & SDC_FAIL)
				goto fail;
		} while (t & SDC_BUSY);
		/* increment our blk_op values for next 256 bytes sector */
		lba++;
		dptr += 256;
	}
	/*  Huzzah!  success! */
	ret = 1;
	/* Boo!  failure */
fail:	sdc_reg_ctl = 0x00;
	return ret;
}

__attribute__((section(".discard")))
/* Returns true if SDC hardware seems to exist */
bool devsdc_exist(void)
{
	uint8_t t;
	sdc_reg_data = 0x64;
	t = sdc_reg_fctl;
	sdc_reg_data = 0x00;
	if ((sdc_reg_fctl ^ t) == 0x60)
		return -1;
	else
		return 0;
}

__attribute__((section(".discard")))
/* Call this to initialize SDC/blkdev interface */
void devsdc_probe(void)
{
	if (devsdc_exist()) {
		td_register(0, sdc_xfer, td_ioctl_none, 1);
		td_register(1, sdc_xfer, td_ioctl_none, 1);
	}
}
