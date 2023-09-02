/*
 *	GM833 RAM DISC
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

__sfr __at 0xFB track;
__sfr __at 0xFC sector;
__sfr __at 0xFD data;

static void gm833_inir_mapped(uint8_t *ptr) __z88dk_fastcall;
static void gm833_otir_mapped(uint8_t *ptr) __z88dk_fastcall;

static uint8_t map;
static uint8_t num_gm833;

static int gm833_transfer(uint_fast8_t minor, bool is_read, uint_fast8_t rawflag)
{
    uint_fast8_t unit = minor << 4;
    unsigned ct;
    unsigned ret;
    unsigned lba;
    uint8_t *dptr;

    map = 0;
    if (rawflag == 1) {
        if (d_blkoff(BLKSHIFT))
            return -1;
        map = udata.u_page;
    } else if (rawflag == 2)
        map = swappage;

    /* On a swap udata gets overwritten so pull anything out first */
    lba = udata.u_block * 4;
    dptr = udata.u_dptr;
    ret = udata.u_nblock << 9; 
    ct = udata.u_nblock * 4;

    while(ct--) {
        sector = lba;
        track = (lba >> 8) | unit;
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
    track = probe;
    sector = 0;
    tmp = data;
    /* Writing the logical track reset the counter for the sector bytes */
    track = probe;
    data = 0xAA;
    track = probe;
    if (data != 0xAA)
        return 0;
    track = probe;
    data = 0x55;
    track = probe;
    if (data != 0x55)
        return 0;
    track = probe;
    data = tmp;
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

COMMON_MEMORY

static void gm833_inir_mapped(uint8_t *ptr) __z88dk_fastcall __naked
{
__asm
    ld a,(_map)
    or a
    call nz, map_proc_a
    ld bc,#0x80FD
    inir
    jp map_kernel
__endasm;
}

static void gm833_otir_mapped(uint8_t *ptr) __z88dk_fastcall __naked
{
__asm
    ld a,(_map)
    or a
    call nz,map_proc_a
    ld bc,#0x80FD
    otir
    jp map_kernel
__endasm;
}
