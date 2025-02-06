/*-----------------------------------------------------------------------*/
/* Fuzix CH375 USB block device driver                                   */
/* 2025 Warren Toomey                                                    */
/*                                                                       */
/* This one is different from the one in Kernel/dev as it assumes that   */
/* the CH375 device will send interrupts.                                */
/*-----------------------------------------------------------------------*/

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <stdbool.h>
#include <blkdev.h>
#include <devch375.h>
#define CHDATARD 0xFFF001	/* One of the CH375 registers */

#ifdef CONFIG_CH375

/****************************************************************************/
/* Code in this file is used only once, at startup, so we want it to live   */
/* in the DISCARD segment. sdcc only allows us to specify one segment for   */
/* each source file.                                                        */
/****************************************************************************/

extern uint_fast8_t devch375_transfer_sector(void);
extern int check_read_byte(void * ptr);

void devch375_init(void)
{
    blkdev_t *blk;
    uint8_t status;
    uint8_t buf[8];
    int i;

    kprintf("CH375 device: ");

    /* See if there is a CH375 device attached by trying */
    /* to read from one of the registers */
    if (check_read_byte((void *)CHDATARD) == 0) {
        kprintf("not found (no register)\n");
        return;
    }

    /* See if the device exists */
    /* Send the reset command and wait 50mS */
    ch375_send_cmd(CMD_RESET_ALL);
    cpu_delay(50);

    /* Now set the USB mode to 6. This should */
    /* cause a level 5 interrupt which will */
    /* update the CH375 status in memory. */
    ch375_send_cmd(CMD_SET_USB_MODE);
    ch375_send_data(6);

    /* Get the CH375 status after a short delay. */
    /* We expect to get USB_INT_CONNECT */
    cpu_delay(50);
    status= ch375_get_status_now();
    if (status != USB_INT_CONNECT) {
        kprintf("not found\n");
        return;
    }

    /* Allocate and set up a blk struct */
    blk = blkdev_alloc();
    if(!blk)
        return;

    blk->transfer = devch375_transfer_sector;
    blk->driver_data = 0;	/* Unused */

    /* Now initialise the disk. In the real world, this */
    /* might return USB_INT_DISCONNECT. In this case, */
    /* the code should prompt the user to attach a USB key */
    /* and try again. TO ADD!! */
    ch375_send_cmd(CMD_DISK_INIT);
    status= ch375_get_status();
    if (status != USB_INT_SUCCESS) {
      	kprintf("ch375 initialisation failed!\n");
      	return;
    }

    /* Get the size of the attached block device */
    blk->drive_lba_count = 0;
    ch375_send_cmd(CMD_DISK_SIZE);
    status= ch375_get_status();
    if (status != USB_INT_SUCCESS) {
      	kprintf("ch375 initialisation failed!\n");
      	return;
    }

    /* Read eight bytes back from the CH375 */
    for (i=0; i<8; i++)
      buf[i]= ch375_read_data();
    blk->drive_lba_count = (buf[0] << 24) + (buf[1] << 16) +
			   (buf[2] <<  8) +  buf[3];

    /* Bytes 4,5,6,7 should be 0,0,2,0 indicating a block */
    /* size of 0x200 (512) */

    blkdev_scan(blk, 0);
}

#endif
