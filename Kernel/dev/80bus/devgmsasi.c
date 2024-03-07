/*
 *	GM8x9 SASI/SCSI controllers
 *	Currently the controller is hardcoded to LUN 7
 */

#include <kernel.h>
#include <tinydisk.h>
#include <tinyscsi.h>
#include <timer.h>
#include <gmsasi.h>

uint8_t sasi_mask;
uint8_t sasi_fast;
uint16_t sasi_datalen;
uint8_t *sasi_dptr;
uint16_t sasi_cmdlen;
uint8_t sasi_cmdbuf[16];
uint8_t scsi_status[2];

int scsi_cmd(uint_fast8_t dev, uint8_t *cmd, uint8_t *data, uint16_t len)
{
    unsigned r;
    sasi_mask = 1 << dev;
    memcpy(sasi_cmdbuf, cmd, 16);
    if (len == 512)
        sasi_fast = 1;
    sasi_datalen = len;
    sasi_dptr = data;
    r = gm849_cmd();
    if (r > 0) {
        kprintf("scsi%d: error %d\n", dev, r);
        return 0;
    }
    return 1;
}

void scsi_reset(void)
{
    unsigned t = set_timer_ms(3000);
    kputs("Resetting SCSI bus... ");
    out(0xE6, 4);
    out(0xE6, 0);
    while(!timer_expired(t));
    kputs("OK\n");
}

void gm849_sasi_init(void)
{
    scsi_probe(7);
}
