#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "tgi.h"

/*
 *	Silly functions to get us going
 */

static uint8_t pen;

void tgi_init(void)
{
}

void tgi_done(void)
{
}

void tgi_setpalette(const unsigned char *palette)
{
}

void tgi_clear(void)
{
//  printf("[CLR]");
  printf("\033[0;0H\033[J");
  fflush(stdout);
}

void tgi_setcolor(unsigned char c)
{
  pen = c;
}

void tgi_setpixel(int x, int y)
{
  printf("\033[%d;%dH%c", y, x, " #"[pen]);
//  printf("[%d,%d->%d]", x, y, pen);
}

void tgi_bar(int x1, int y1, int x2, int y2)
{
  int x,y;
  for (y = y1; y <= y2; y++) {
    printf("\033[%d;%dH", y, x1);
    if (pen)
      for(x = x1; x <= x2; x++)
        putchar('#');
    else
      for(x = x1; x <= x2; x++)
        putchar(' ');
  }
//  printf("[R %d,%d,%d,%d->%d]", x1, y1, x2,y2, pen);
}

void tgi_line(int x1, int y1, int x2, int y2)
{
   int dx =  abs(x2-x1), sx = x1<x2 ? 1 : -1;
   int dy = -abs(y2-y1), sy = y1<y2 ? 1 : -1; 
   int err = dx+dy, e2; /* error value e_xy */
 
   for(;;){  /* loop */
      tgi_setpixel(x1,y1);
      if (x1==x2 && y1==y2) break;
      e2 = 2*err;
      if (e2 >= dy) { err += dy; x1 += sx; } /* e_xy+e_x > 0 */
      if (e2 <= dx) { err += dx; y1 += sy; } /* e_xy+e_y < 0 */
   }

//  printf("[L %d,%d,%d,%d->%d]", x1, y1, x2,y2, pen);
}
