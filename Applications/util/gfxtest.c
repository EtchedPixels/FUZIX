#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/graphics.h>

static struct display d;
static struct videomap v;

__sfr __at 0x00 hrg1b_off;
__sfr __at 0x01 hrg1b_on;
__sfr __at 0x02 hrg1b_l;
__sfr __at 0x03 hrg1b_h;
__sfr __at 0x04 hrg1b_r;
__sfr __at 0x05 hrg1b_w;

static int test_hrg1b(void)
{
    int line, col, row;
    uint8_t c;
    if (ioctl(0, GFXIOC_MAP, &v)) {
        perror("GFXIOC_MAP");
        exit(1);
    }
    if (!(v.flags & MAP_PIO)) {
        fprintf(stderr, "PIO map required.\n");
        exit(1);
    }
    if (d.format != FMT_MONO_WB) {
        fprintf(stderr, "Format should be mono.\n");
        exit(1);
    }
    printf("HRG1 at 0x%02X\n", (unsigned int)v.pio);
    if ((unsigned int)v.pio != 0) {
        fprintf(stderr, "HR1G reported on wrong port.\n");
        exit(1);
    }
    hrg1b_on = 255;
    for (col = 0; col < 64; col++) {
        for (row = 0; row < 16; row ++) {
            for (line = 0; line < 12; line += 2) {
                uint16_t v = col | (row << 6) | (line << 10);
                hrg1b_l = v;
                hrg1b_h = (v >> 8);
                hrg1b_w = 0x2A;
                v += (1 << 10);
                hrg1b_h = (v >> 8);
                hrg1b_w = 0x15;
                
            }
        }
    }
    c = hrg1b_r;
    getchar();
    hrg1b_off = 255;
    if (c != 0x15) {
        fprintf(stderr, "hrg1: read fail.\n");
        exit(1);
    }
}

__sfr __at 0xEC le18_data;
__sfr __at 0xED le18_x;
__sfr __at 0xEE le18_y;
__sfr __at 0xEF le18_ctrl;

static int test_le18(void)
{
    int col, row;
    uint8_t c;
    if (ioctl(0, GFXIOC_MAP, &v)) {
        perror("GFXIOC_MAP");
        exit(1);
    }
    if (!(v.flags & MAP_PIO)) {
        fprintf(stderr, "PIO map required.\n");
        exit(1);
    }
    if (d.format != FMT_MONO_WB) {
        fprintf(stderr, "Format should be mono.\n");
        exit(1);
    }
    printf("LE18 at 0x%02X\n", (unsigned int)v.pio);
    if ((unsigned int)v.pio != 0xEC) {
        fprintf(stderr, "LE18 reported on wrong port.\n");
        exit(1);
    }
    le18_ctrl = 1;
    for (col = 0; col < 64; col++) {
        le18_x = col;
        for (row = 0; row < 192; row++) {
            le18_y = row++;
            le18_data = 0x2A;
            le18_y = row;
            le18_data = 0x15;
        }
    }
    c = le18_data;
    getchar();
    le18_ctrl = 0;
    if ((c & 0x3F) != 0x15) {
        fprintf(stderr, "le18: read fail.\n");
        exit(1);
    }
}

__sfr __at 0xFF microlabs_ctrl;
uint8_t *microlabs_fb = (uint8_t *)0x3C00;

static void test_microlabs(void)
{
    int line, col, row;
    if (ioctl(0, GFXIOC_MAP, &v)) {
        perror("GFXIOC_MAP");
        exit(1);
    }
    if ((v.flags & (MAP_FBMEM|MAP_PIO)) != (MAP_FBMEM | MAP_PIO)) {
        fprintf(stderr, "PIO and FBMEM map required.\n");
        exit(1);
    }
    if (d.format != FMT_MONO_WB) {
        fprintf(stderr, "Format should be mono.\n");
        exit(1);
    }
    printf("Grafyx at 0x%02X\n", (unsigned int)v.pio);
    if ((unsigned int)v.pio != 0xFF) {
        fprintf(stderr, "Model 3 Grafyx reported on wrong port.\n");
        exit(1);
    }
    if (v.fbmem != microlabs_fb || v.fbsize != 0x0400) {
        fprintf(stderr,"Model 3 Graftx fbmem should be 3C00-3FFF.\n");
        exit(1);
    }
    
    for (col = 0; col < 64; col++) {
        for (row = 0; row < 16; row ++) {
            for (line = 0; line < 12; line ++) {
                microlabs_ctrl = 0xE0 | (line << 1);
                microlabs_fb[(row << 6) | col] = (line & 1) ? 0xAA : 0x55;
            }
        }
    }
    getchar();
    microlabs_ctrl = 0;    
}
    
__sfr __at 0x80 trs80_x;
__sfr __at 0x81 trs80_y;
__sfr __at 0x82 trs80_d;
__sfr __at 0x83 trs80_ctrl;

void test_trs80gfx(void)
{
    int col, row;
    uint8_t c;
    if (ioctl(0, GFXIOC_MAP, &v)) {
        perror("GFXIOC_MAP");
        exit(1);
    }
    if (!(v.flags & MAP_PIO)) {
        fprintf(stderr, "PIO map required.\n");
        exit(1);
    }
    if (d.format != FMT_MONO_WB) {
        fprintf(stderr, "Format should be mono.\n");
        exit(1);
    }
    printf("TRS80 Hi Res Graphics at 0x%02X\n", (unsigned int)v.pio);
    if ((unsigned int)v.pio != 0x80) {
        fprintf(stderr, "TRS80 HRG reported on wrong port.\n");
        exit(1);
    }
    trs80_ctrl = 0xB3;	/* clock x on write */
    for (row = 0; row < 240; row++) {
        trs80_y = row++;
        trs80_x = 0;
        /* The card is designed so you can effectively use otir and friends */
        for (col = 0; col < 80; col++)
            trs80_d = 0xAA;
        trs80_y = row;
        trs80_x = 0;
        for (col = 0; col < 80; col++)
            trs80_d = 0x55;
    }
    trs80_y = 0;
    trs80_x = 0;
    c = trs80_d;
    getchar();
    trs80_ctrl = 0;
    if (c != 0xAA) {
        fprintf(stderr, "trs80gfx: read fail.\n");
        exit(1);
    }
}

void test_mono(void)
{
    int x,y;
    uint8_t c = 0xAA;
    uint8_t *p = v.fbmem;

    for (y = 0; y < d.height; y++) {
        uint8_t *r = p;
        for (x = 0; x < d.width/8; x++)
            *r++ = c;
        p += d.stride;
        c ^=255;
    }
    getchar();
}

void test_text(void)
{
    int x,y;
    uint8_t c = '#';
    uint8_t *p = v.fbmem;

    for (y = 0; y < d.height; y++) {
        uint8_t *r = p ;
        for (x = 0; x < d.width; x++)
            *r++ = c;
        p += d.stride;
    }
    getchar();
}

void unaccelerated(void)
{
    if (ioctl(0, GFXIOC_MAP, &v)) {
        fprintf(stderr, "Only mapped formats supported.\n");
        exit(1);
    }
    switch(d.format) {
    case FMT_TEXT:
        test_text();
        break;
    case FMT_MONO_BW:
    case FMT_MONO_WB:
        test_mono();
        break;
    default:
        fprintf(stderr, "unsupported unaccelerated format %d.\n", d.format);
    }
}
    
int main(int argc, char *argv[])
{
    if (ioctl(0, GFXIOC_GETINFO, &d)) {
        perror("GFXIOC_GETINFO");
        exit(1);
    }
    switch(d.hardware) {
    case HW_HRG1B:
        test_hrg1b();
        break;
    case HW_MICROLABS:
        test_microlabs();
        break;
    case HW_TRS80GFX:
        test_trs80gfx();
        break;
    case HW_LOWE_LE18:
        test_le18();
        break;
    case HW_UNACCEL:
        unaccelerated();
        break;
    default:
        fprintf(stderr, "Hardware type %d not supported.\n", d.hardware);
        exit(1);
    }
    exit(0);
}
