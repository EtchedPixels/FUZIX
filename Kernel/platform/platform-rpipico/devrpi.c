#include <kernel.h>
#include "picosdk.h"
#include <pico/bootrom.h>

int rpi_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    switch(minor)
    {
        case 0:
            reset_usb_boot(0, 0);
            return 0; /* Will never execute */
        default:
            return -1;
    }
}