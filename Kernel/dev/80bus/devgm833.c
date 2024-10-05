/*
 *	GM833 RAM DISC
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#define track 0xFB
#define sector 0xFC
#define data 0xFD

static void gm833_inir_mapped(uint8_t *ptr);
static void gm833_otir_mapped(uint8_t *ptr);

uint8_t gm833_map;
static uint8_t num_gm833;

static int gm833_transfer(uint_fast8_t minor, bool is_read, uint_fast8_t rawflag)
{
    uint_fast8_t unit = minor << 4;
    unsigned ct;
    unsigned ret;
    unsigned lba;
    uint8_t *dptr;

    gm833_map = 0;
    if (rawflag == 1) {
        if (d_blkoff(BLKSHIFT))
            return -1;
        gm833_map = udata.u_page;
    } else if (rawflag == 2)
        gm833_map = swappage;

    /* On a swap udata gets overwritten so pull anything out first */
    lba = udata.u_block * 4;
    dptr = udata.u_dptr;
    ret = udata.u_nblock << 9; 
    ct = udata.u_nblock * 4;

    while(ct--) {
        out(sector, lba);
        out(track, (lba >> 8) | unit);
        if (is_read)
            gm833_inir_mapped(dptr);
        else
            gm833_otir_mapped(dptr);
        lba++;
        dptr += 128;
    }
    return ret;
}

int gm833_open(uint_fast8_t minor, uint16_t flag)
{
    if (minor >= num_gm833) {
        udata.u_error = ENODEV;
        return -1;
    }
    return 0;
}

int gm833_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    flag;minor;
    return gm833_transfer(minor, true, rawflag);
}

int gm833_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
    flag;minor;
    return gm833_transfer(minor, false, rawflag);
}

static uint_fast8_t gm833_probe_track(uint_fast8_t probe)
{
    uint_fast8_t tmp;
    out(track, probe);
    out(sector, 0);
    tmp = in(data);
    /* Writing the logical track reset the counter for the sector bytes */
    out(track, probe);
    out(data, 0xAA);
    out(track, probe);
    if (in(data) != 0xAA)
        return 0;
    out(track, probe);
    out(data, 0x55);
    out(track, probe);
    if (in(data) != 0x55)
        return 0;
    out(track, probe);
    out(data, tmp);
    return 1;
}

uint_fast8_t gm833_probe(void)
{
    unsigned i;
    /* Up to 16 boards with 16 tracks per board = 8MB. The system
       software only bothered supporting 4 max (2MB) */
    for (i = 0; i < 256; i+= 16) {
        if (gm833_probe_track(i) == 0)
            break;
        num_gm833++;
    }
    if (num_gm833)
        kprintf("gm833: %d found.\n", num_gm833);
    return num_gm833;
}
