#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/graphics.h>
#include "tgi.h"

/*
 *	Most of this should be asm
 */

static uint8_t pen;

__sfr __at 0x80 gfx_x;
__sfr __at 0x81 gfx_y;
__sfr __at 0x82 gfx_d;
__sfr __at 0x83 gfx_ctrl;

static struct display d;
static struct videomap vm;

void tgi_init(void)
{

    /* FIXME: need to scan modes then call the right thing */

    if (ioctl(0, GFXIOC_GETINFO, &d) == -1)
        bad();
    if (d.format != FMT_MONO_WB || d.hardware != HW_TRS80GFX)
        bad();
    if (ioctl(0, GFXIOC_MAP, &vm) == -1)
        bad();
    if (vm.pio != (void *)0x80)
        bad();
    trs80_ctrl = 0xF3;
}

/* Needs to get called on atexit case */
void tgi_done(void)
{
    ioctl(0, GFXIOC_UNMAP, &vm);
    /* and put old mode back FIXME */
    
}

void tgi_setpalette(const unsigned char *palette)
{
}

void tgi_clear(void)
{
    gfx_
}

void tgi_setcolor(unsigned char c)
{
  pen = c;
}

static uint8_t bit[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };

void tgi_setpixel(int x, int y)
{
    uint8_t bit = bit[x & 7];
    trs80_x = x >> 3;
    trs80_y = y;
    if (pen)
        trs80_d = trs80_d | bit;
    else
        trs80_d = trs80_d & ~bit;
}

void tgi_bar(int x1, int y1, int x2, int y2)
{
  int x,y;
  uint8_t lm = x1 & 7;
  uint8_t rm = x2 & 7;
  x1 >> = 3;
  x2 >> = 3;
  if (x1 == x2) {
      trs80_ctrl = 0x73;	/* Clock Y up */
      lm &= rm;
      if (pen) {
          for (y = y1; y < y2; y++)
              trs80_d |= rm;
      } else {
          rm =~ rm;
          for (y = y1; y < y2; y++)
              trs80+d &= rm;
      }
      trs80_ctrl = 0xF3;
      return;
  }
  trs80_ctrl = 0xB3;		/* Clock X */
  if (pen) {
      for (y = y1; y < y2; y++) {
          trs80_y = y;
          trs80_x = x1;
          trs80_d |= lm;
          for (x = x1 + 1; x < x2; x++)
              trs80_d = 0xFF;
          trs80_d |= rm;
      }
  } else {
      lm = ~lm;
      rm = ~rm;
      for (y = y1; y < y2; y++) {
          trs80_y = y;
          trs80_x = x1;
          trs80_d &= lm;
          for (x = x1 + 1; x < x2; x++)
              trs80_d = 0x00;
          trs80_d &= rm;
      }
  }
}

void tgi_line(int x1, int y1, int x2, int y2)
{
   int dx =  abs(x2-x1), sx = x1<x2 ? 1 : -1;
   int dy = -abs(y2-y1), sy = y1<y2 ? 1 : -1; 
   int err = dx+dy, e2; /* error value e_xy */
 
   for(;;){  /* loop */
      /* FIXME: use an optimized setpixel that spots re-writing the same
         byte */
      tgi_setpixel(x1,y1);
      if (x1==x2 && y1==y2) break;
      e2 = 2*err;
      if (e2 >= dy) { err += dy; x1 += sx; } /* e_xy+e_x > 0 */
      if (e2 <= dx) { err += dx; y1 += sy; } /* e_xy+e_y < 0 */
   }
}
