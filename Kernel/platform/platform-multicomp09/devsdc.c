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

#define SD_IDLE_STAT (0x80)

#define SD_WR_CMD (0x01)
#define SD_RD_CMD (0x00)


#define sd_reg_ctl  *((volatile uint8_t *)SDCTL)
#define sd_reg_data *((volatile uint8_t *)SDDATA)
#define sd_reg_lba0 *((volatile uint8_t *)SDLBA0)
#define sd_reg_lba1 *((volatile uint8_t *)SDLBA1)
#define sd_reg_lba2 *((volatile uint8_t *)SDLBA2)


/* a "simple" internal function pointer to which transfer
   routine to use.
*/
typedef void (*sd_transfer_function_t)( void *addr);


/* blkdev method: flush drive */
int devsd_flush( void )
{
	return 0;
}


/* blkdev method: transfer sectors */
uint8_t devsd_transfer_sector(void)
{
	uint8_t *ptr;                  /* points to 32 bit lba in blk op */
	sd_transfer_function_t fptr;   /* holds which xfer routine we want */

	/* wait for drive to go non-busy after previous command
	   (if any)
	*/
	while (sd_reg_ctl != SD_IDLE_STAT) {
	}

	/* load up block address. It's stored as a 32-bit value but we
	   ignore the MS byte because the SD controller only has a
	   24-bit address range.
           The hardware seems a bit fussy about having the addresses sent
           in this order (lba0..lba2). With them send lba2 first, a
           delay loop was needed between the sr_reg_ctl poll and the lba2
           write in order to work on real hardware.
	*/
 	ptr=((uint8_t *)(&blk_op.lba))+1;
	sd_reg_lba0 = ptr[2];
	sd_reg_lba1 = ptr[1];
	sd_reg_lba2 = ptr[0]; /* MS byte of 24-bit block address */


	/* send the command and set up the subroutine pointer */
	if( blk_op.is_read ){
		sd_reg_ctl = SD_RD_CMD;
		fptr = devsd_read;
	}
	else{
		sd_reg_ctl = SD_WR_CMD;
		fptr = devsd_write;
	}

	/* do the low-level data transfer (512 bytes) */
	fptr( blk_op.addr );

	/* No mechanism for failing so assume success! */
	return 1;
}

__attribute__((section(".discard")))
/* Returns true if SD hardware seems to exist */
bool devsd_exist()
{
	/* Only way to boot is through SD so it must
	   exist!
	*/
	return 1;
}

__attribute__((section(".discard")))
/* Call this to initialize SD/blkdev interface */
void devsd_init()
{
	blkdev_t *blk;

	kputs("SD: ");
	if( devsd_exist() ){
		/* there is only 1 drive. Register it. */
		blk=blkdev_alloc();
		blk->driver_data = 0 ;
		blk->transfer = devsd_transfer_sector;
		blk->flush = devsd_flush;
		blk->drive_lba_count=-1;
		blk->drive_lba_count=32764; /* [NAC HACK 2016Apr26]  hack!! */
		blkdev_scan(blk, 0);

		kputs("ok.\n");
	}
	else kprintf("Not Found.\n");
}

