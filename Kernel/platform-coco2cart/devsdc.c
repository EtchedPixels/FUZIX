/*
 * CoCoSDC driver
 * (c)2015 Brett M. Gordon GPL2
 *
 * Needs work on extended SDC API stuff.
 * init / mounting stuff really needs to set/update blkdev structure for size
 *
 * Only supports simple memory models.
 */

#include <kernel.h>
#include <kdata.h>
#include <blkdev.h>
#include <mbr.h>
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

static uint8_t sdc_present;

/* Assembler glue */

extern void sdc_read_data(uint8_t *p);
extern void sdc_write_data(uint8_t *p);
extern uint8_t sdcpage;

/* a "simple" internal function pointer to which transfer
   routine to use. The is_read var might be better stored
   in a way that the asm helpers could do self modifying.
*/
typedef void (*sdc_transfer_function_t)( unsigned char *addr);



/* blkdev method: transfer sectors */
static uint16_t sdc_transfer(uint8_t minor, bool is_read, uint8_t rawflag)
{
       	uint8_t nb = 0;
	uint32_t lba;             /* holds 32 bit lsn */
	uint8_t *ptr = ((uint8_t *)&lba) + 1;      /* points to 24 bit lba in blk op */
	uint8_t t;                /* temporarory sdc status holder */
	uint8_t cmd;              /* holds SDC command value */
	sdc_transfer_function_t fptr;  /* holds which xfer routine we want */

	if (rawflag == 1 && d_blkoff(9))
		return -1;

	/* pass rawflag to assembler */
	sdcpage = rawflag;


	/* we get 256 bytes per lba in SDC so convert */
	udata.u_nblock *= 2;
	lba = udata.u_block * 2;
	ptr[0] |= (minor & 0x7f) << 1;

	/* setup cmd pointer and command value from blk_op */
	if (is_read) {
		cmd = 0x80;
		fptr = sdc_read_data;
	} else {
		cmd = 0xa0;
		fptr = sdc_write_data;
	}

	/* apply our drive value 0 or 1 */
	if (minor & 0x80)
		cmd++;

	while (udata.u_nblock--) {
		/* load up registers */
		sdc_reg_param1 = ptr[0];
		sdc_reg_param2 = ptr[1];
		sdc_reg_param3 = ptr[2];
		sdc_reg_cmd= cmd;
		asm("\texg x,x\n");     /* delay 16us minimum */
		asm("\texg x,x\n");
		/* wait till SDC is ready */
		do {
			t=sdc_reg_stat;
			if (t & SDC_FAIL)
				goto fail;
		} while(!(t & SDC_READY));
		/* do our low-level xfer function */
		fptr(udata.u_dptr);
		/* and wait will ready again, and test for failure */
		do {
			t=sdc_reg_stat;
			if( t & SDC_FAIL )
				goto fail;
		} while (t & SDC_BUSY);
		/* increment our blk_op values for next 256 bytes sector */
		udata.u_dptr += 256;
		lba++;
		nb++;
	}
	/*  Huzzah!  success! */
	return nb << 8;
	/* Boo!  failure */
 fail:	sdc_reg_ctl = 0x00;
	udata.u_error = EIO;
	return -1;
}


int sdc_read(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    return sdc_transfer(minor, true, rawflag);
}

int sdc_write(uint8_t minor, uint8_t rawflag, uint8_t flag)
{
    return sdc_transfer(minor, false, rawflag);
}

int sdc_open(uint8_t minor, uint16_t flag)
{
	if (!sdc_present) {
		udata.u_error = ENODEV;
		return -1;
	}
	return 0;
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
	else return 0;
}

__attribute__((section(".discard")))
/* Call this to initialize SDC/blkdev interface */
void devsdc_init(void)
{
	kputs("SDC: ");
	if (devsdc_exist()) {
	    	/* turn on uber-secret SDC LBA mode*/
		sdc_reg_ctl = 0x43; 
		sdc_present = 1;
		kputs("Ok.\n");
	}
	else kprintf("Not found.\n");
}

