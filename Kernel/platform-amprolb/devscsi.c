/*
 *	Glue to the asm NCR 5380 driver code
 */

#include <kernel.h>
#include <printf.h>
#include <timer.h>
#include <tinyscsi.h>
#include "ncr5380.h"

int scsi_cmd(uint_fast8_t dev, uint8_t *cmd, uint8_t *data, uint16_t len)
{
    uint8_t r;
    scsi_target = 1 << dev;
    memcpy(scsicmd, cmd, 16);
    /* Quick way to optimize the usual block transfer sizes */
    if (len & 0xFF)
        scsi_burst = 1;
    else
        scsi_burst = 0;	/* 256 */
    scsi_dbuf = data;
    r = ncr5380_command();
    return r;
}

void scsi_reset(void)
{
    unsigned t = set_timer_ms(3000);
    scsi_idbits =  1 << scsi_id;
    kputs("Resetting SCSI bus... ");
    ncr5380_reset_on();
    ncr5380_reset_off();
    while(!timer_expired(t));
    kputs("OK\n");
}

__sfr __at 0x29	id;

void scsi_init(void)
{
    scsi_probe(id & 7);
}
