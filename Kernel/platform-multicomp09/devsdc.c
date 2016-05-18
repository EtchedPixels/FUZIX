/*

  multicomp09 SD driver

  Derived from:

  CoCoSDC driver
  (c)2015 Brett M. Gordon GPL2

 * Needs work on extended SDC API stuff.
 * init / mounting stuff really needs to set/update blkdev structure for size
 * need to get rawmode=1 and 2 working.

*/

#include <kernel.h>
#include <kdata.h>
#include <blkdev.h>
#include <mbr.h>
/* not to be confused with devsd.h ..!! [NAC HACK 2016Apr26] */
#include <devsdc.h>
#include <printf.h>


/* multicomp09 hw registers */
#define SDDATA  (0xffd8)
#define SDCTL   (0xffd9)
#define SDLBA0  (0xffda)
#define SDLBA1  (0xffdb)
#define SDLBA2  (0xffdc)

#define SDC_IDLE_STAT (0x80)

#define SDC_WR_CMD (0x01)
#define SDC_RD_CMD (0x00)


#define sdc_reg_ctl  *((volatile uint8_t *)SDCTL)
#define sdc_reg_data *((volatile uint8_t *)SDDATA)
#define sdc_reg_lba0 *((volatile uint8_t *)SDLBA0)
#define sdc_reg_lba1 *((volatile uint8_t *)SDLBA1)
#define sdc_reg_lba2 *((volatile uint8_t *)SDLBA2)


/* a "simple" internal function pointer to which transfer
   routine to use.
*/
typedef void (*sdc_transfer_function_t)( void *addr);


/* blkdev method: flush drive */
int devsdc_flush( void )
{
	return 0;
}

/* [NAC HACK 2016Apr26] devsd version is called devsd_transfer_sector */

/* blkdev method: transfer sectors */
uint8_t devsdc_transfer(void)
{
	uint8_t *ptr;                  /* points to 32 bit lba in blk op */
	sdc_transfer_function_t fptr;  /* holds which xfer routine we want */
        int i;
        uint8_t tmp;

	/* wait for drive to go non-busy after previous command
	   (if any)
	*/
	while (sdc_reg_ctl != SDC_IDLE_STAT) {
	}

	/* [NAC HACK 2016May11] should not need this but real hardware seems
	   to need something here even tho CUBIX FORTH NITROS9 FLEX all work
	   without it and with seemingly equivalent code
	*/
	for (i=0; i<1000; i++) {
		tmp = sdc_reg_ctl;
	}

	/* load up block address. It's stored as a 32-bit value but we
	   ignore the MS byte because the SD controller only has a
	   24-bit address range
	*/
 	ptr=((uint8_t *)(&blk_op.lba))+1;
	sdc_reg_lba2 = ptr[0]; /* MS byte of 24-bit block address */
	sdc_reg_lba1 = ptr[1];
	sdc_reg_lba0 = ptr[2];


	/* send the command and set up the subroutine pointer */
	if( blk_op.is_read ){
		sdc_reg_ctl = SDC_RD_CMD;
		fptr = devsdc_read;
	}
	else{
		sdc_reg_ctl = SDC_WR_CMD;
		fptr = devsdc_write;
	}

	/* do the low-level data transfer (512 bytes) */
	fptr( blk_op.addr );

	/* No mechanism for failing so assume success! */
	return 1;
}

__attribute__((section(".discard")))
/* Returns true if SDC hardware seems to exist */
bool devsdc_exist()
{
	/* Only way to boot is through SDC so it must
	   exist!
	*/
	return 1;
}

__attribute__((section(".discard")))
/* Call this to initialize SDC/blkdev interface */
void devsd_init()
{
	blkdev_t *blk;

	kputs("SDC: ");
	if( devsdc_exist() ){
		/* register first drive */
		blk=blkdev_alloc();
		blk->driver_data = 0 ;
		blk->transfer = devsdc_transfer;
		blk->flush = devsdc_flush;
		blk->drive_lba_count=-1;
		blk->drive_lba_count=32764; /* [NAC HACK 2016Apr26]  hack!! */

		/* by inspection of dev/devsd_discard.c, vital piece missing from this code:
		   blkdev_scan(blk, 0)  - from dev/blkdev.c
		*/
		blkdev_scan(blk, 0);

		/* register second drive */
		/*		blk=blkdev_alloc();
		blk->driver_data = 1 ;
		blk->transfer = devsdc_transfer;
		blk->flush = devsdc_flush;
		blk->drive_lba_count=-1; */
		kputs("Ok.\n");
	}
	else kprintf("Not Found.\n");
}

