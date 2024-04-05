/*
 *	Low level SASI driver
 */

#include <kernel.h>
#include <printf.h>
#include <timer.h>
#include <tinyscsi.h>
#include <lobo.h> 

extern void sasi_inblock(uint8_t *buf);
extern void sasi_outblock(uint8_t *buf);

uint16_t sasi_len;
uint8_t scsi_status[2];

/* We don't support multi-master */
static unsigned scsi_select(uint_fast8_t dev)
{
    unsigned t = set_timer_ms(1000);

    lobo_io[0x7F0] = (1 << scsi_id) | (1 << dev);	/* Set our bit too */
    lobo_io[0x7F4] = 0x02;		/* SEL */

    while(!timer_expired(t)) {
        if (lobo_io[0x7F4] & 0x04)	/* Target responded */
            return 1;
    }
    return 0;
}

static uint8_t lobo_rstat(void)
{
    uint_fast8_t r, s;
    while(!(s = lobo_io[0x7F4] & 0x01)) {
        if ((s & 0x04) != 0x04)
            return 0xFF;
    }
    r = lobo_io[0x7F0];
    lobo_io[0x7F4] = 1;
    while((s = lobo_io[0x7F4]) & 0x01) {
        if ((s & 0x04) != 0x04)
            break;
    }    
    lobo_io[0x7F4] = 0;
}

int scsi_cmd(uint_fast8_t dev, register uint8_t *cmd, uint8_t *data, uint16_t len)
{
    uint_fast8_t r, s;
    unsigned i;

    /* Bus is not free */
    if (lobo_io[0x7F4])
        return 1;
    /* Drive selection failed */
    if (scsi_select(dev) == 0)
        return 1;
    /* Clear selection, device holds bus with BSY */
    lobo_io[0x7F4] = 0x00;		/* Drop SEL */

    /* Send command bytes until the device drops C/D */
    while(i < 10) {
        if (!(lobo_io[0x7F4] & 0x10))	/* Command state ? */
            break;
        while(!((s = lobo_io[0x7F4]) & 0x01)) {
            if (!(s & 0x04))
                return 1;
        }
        lobo_io[0x7F0]= *cmd++;
        lobo_io[0x7F4] = 1;		/* ACK */
        while((s = lobo_io[0x7F4]) & 0x01) {
            if (!(s & 0x04)) {
                lobo_io[0x7F4] = 0;
                return 1;
            }
        }
        lobo_io[0x7F4] = 0;		/* ACK off */
        i++;
    }
    /* Didn't transfer a whole command */
    if (i < 6)
        return 1;
    /* See which way the transfer goes ? */
    if (len) {
        sasi_len = len;
        if (lobo_io[0x7F4] & 0x08)
            sasi_outblock(data);
        else
            sasi_inblock(data);
    }
    /* Fell off the bus somewhere ? */
    if ((lobo_io[0x7F4] & 0x04) == 0)
        return 1;
    /* And the status */
    scsi_status[0] = lobo_rstat();
    scsi_status[1] = lobo_rstat();
    return 0;
}

void scsi_reset(void)
{
    unsigned t = set_timer_ms(3000);
    kputs("Resetting SCSI bus... ");
    lobo_io[0x7F4] = 4;
    lobo_io[0x7F4] = 0;
    while(!timer_expired(t));
    kputs("OK\n");
}
