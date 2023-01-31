#define _IDE_PRIVATE

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <timer.h>
#include <devide.h>
#include <blkdev.h>


/* We know we have one drive only */

void ide_resume(void)
{
    devide_writeb(ide_reg_devhead, 0xE0);
    if (devide_wait(IDE_STATUS_READY)) {
        devide_writeb(ide_reg_features,0x01);
        devide_writeb(ide_reg_command, IDE_CMD_SET_FEATURES);
        if (devide_wait(IDE_STATUS_READY))
            return;
    }
    panic("IDE resume failed!");
}
