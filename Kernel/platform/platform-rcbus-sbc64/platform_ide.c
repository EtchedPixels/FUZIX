#define _IDE_PRIVATE

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <timer.h>
#include <tinyide.h>
#include <plt_ide.h>

/* We know we have one drive only */

void ide_resume(void)
{
    ide_write(devh, 0xE0);
    while(ide_read(status) & 0x80);	/* Wait for !BSY */
    ide_write(error,0x01);
    ide_write(cmd, 0xEF);		/* Set Features */
    while((ide_read(status) & 0x40) == 0x40);	/* Wait for DRDY */
}
