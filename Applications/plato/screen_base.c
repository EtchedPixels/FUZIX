/**
 * PLATOTerm64 - A PLATO Terminal for the Commodore 64
 * Based on Steve Peltz's PAD
 * 
 * Author: Thomas Cherryhomes <thom.cherryhomes at gmail dot com>
 *
 * screen_base.c - Display output functions (base)
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "tgi.h"
#include "screen.h"
#include "protocol.h"
#include "io.h"

uint8_t CharWide=8;
uint8_t CharHigh=16;
padPt TTYLoc;
uint8_t pal[2];

extern padBool FastText; /* protocol.c */

extern unsigned short scalex[];
extern unsigned short scaley[];

extern uint8_t font[];
extern uint8_t fontm23[];
extern uint16_t fontptr[];
extern uint8_t FONT_SIZE_X;
extern uint8_t FONT_SIZE_Y;

/**
 * screen_init() - Set up the screen
 */
void screen_init(void)
{
  screen_load_driver();
  tgi_init();
  screen_init_hook();
  screen_update_colors();
  tgi_setpalette(pal);
  tgi_clear();
}

/**
 * screen_clear - Clear the screen
 */
void screen_clear(void)
{
  tgi_clear();
}

/**
 * screen_block_draw(Coord1, Coord2) - Perform a block fill from Coord1 to Coord2
 */
void screen_block_draw(padPt* Coord1, padPt* Coord2)
{
  if (CurMode==ModeErase || CurMode==ModeInverse)
    tgi_setcolor(0);
  else
    tgi_setcolor(1);

  tgi_bar(scalex[Coord1->x],scaley[Coord1->y],scalex[Coord2->x],scaley[Coord2->y]);
}

/**
 * screen_dot_draw(Coord) - Plot a mode 0 pixel
 */
void screen_dot_draw(padPt* Coord)
{
  if (CurMode==ModeErase || CurMode==ModeInverse)
    tgi_setcolor(0);
  else
    tgi_setcolor(1);
  tgi_setpixel(scalex[Coord->x],scaley[Coord->y]);
}

/**
 * screen_line_draw(Coord1, Coord2) - Draw a mode 1 line
 */
void screen_line_draw(padPt* Coord1, padPt* Coord2)
{
  uint16_t x1=scalex[Coord1->x];
  uint16_t x2=scalex[Coord2->x];
  uint16_t y1=scaley[Coord1->y];
  uint16_t y2=scaley[Coord2->y];

  if (CurMode==ModeErase || CurMode==ModeInverse)
    tgi_setcolor(0);
  else
    tgi_setcolor(1);

  tgi_line(x1,y1,x2,y2);
}

/**
 * screen_char_draw(Coord, ch, count) - Output buffer from ch* of length count as PLATO characters
 */
void screen_char_draw(padPt* Coord, unsigned char* ch, unsigned char count)
{
  int16_t offset; /* due to negative offsets */
  uint16_t x;      /* Current X and Y coordinates */
  uint16_t y;
  uint16_t* px;   /* Pointers to X and Y coordinates used for actual plotting */
  uint16_t* py;
  uint8_t i; /* current character counter */
  uint8_t a; /* current character byte */
  uint8_t j,k; /* loop counters */
  int8_t b; /* current character row bit signed */
  uint8_t width=FONT_SIZE_X;
  uint8_t height=FONT_SIZE_Y;
  uint16_t deltaX=1;
  uint16_t deltaY=1;
  uint8_t mainColor=1;
  uint8_t altColor=0;
  uint8_t *p;
  uint8_t* curfont;
  
  switch(CurMem)
    {
    case M0:
      curfont=font;
      offset=-32;
      break;
    case M1:
      curfont=font;
      offset=64;
      break;
    case M2:
      curfont=fontm23;
      offset=-32;
      break;
    case M3:
      curfont=fontm23;
      offset=32;      
      break;
    }

  if (CurMode==ModeRewrite)
    {
      altColor=0;
    }
  else if (CurMode==ModeInverse)
    {
      altColor=1;
    }
  
  if (CurMode==ModeErase || CurMode==ModeInverse)
    mainColor=0;
  else
    mainColor=1;

  tgi_setcolor(mainColor);

  x=scalex[(Coord->x&0x1FF)];
  y=scaley[((Coord->y)+14)&0x1FF];
  
  if (FastText==padF)
    {
      goto chardraw_with_fries;
    }

  /* the diet chardraw routine - fast text output. */
  
  for (i=0;i<count;++i)
    {
      a=*ch;
      ++ch;
      a+=offset;
      p=&curfont[fontptr[a]];
      
      for (j=0;j<FONT_SIZE_Y;++j)
  	{
  	  b=*p;
	  
  	  for (k=0;k<FONT_SIZE_X;++k)
  	    {
  	      if (b<0) /* check sign bit. */
		{
		  tgi_setcolor(mainColor);
		  tgi_setpixel(x,y);
		}

	      ++x;
  	      b<<=1;
  	    }

	  ++y;
	  x-=width;
	  ++p;
  	}

      x+=width;
      y-=height;
    }

  return;

 chardraw_with_fries:
  if (Rotate)
    {
      deltaX=-abs(deltaX);
      width=-abs(width);
      px=&y;
      py=&x;
    }
    else
    {
      px=&x;
      py=&y;
    }
  
  if (ModeBold)
    {
      deltaX = deltaY = 2;
      width<<=1;
      height<<=1;
    }
  
  for (i=0;i<count;++i)
    {
      a=*ch;
      ++ch;
      a+=offset;
      p=&curfont[fontptr[a]];
      for (j=0;j<FONT_SIZE_Y;++j)
  	{
  	  b=*p;

	  if (Rotate)
	    {
	      px=&y;
	      py=&x;
	    }
	  else
	    {
	      px=&x;
	      py=&y;
	    }

  	  for (k=0;k<FONT_SIZE_X;++k)
  	    {
  	      if (b<0) /* check sign bit. */
		{
		  tgi_setcolor(mainColor);
		  if (ModeBold)
		    {
		      tgi_setpixel(*px+1,*py);
		      tgi_setpixel(*px,*py+1);
		      tgi_setpixel(*px+1,*py+1);
		    }
		  tgi_setpixel(*px,*py);
		}
	      else
		{
		  if (CurMode==ModeInverse || CurMode==ModeRewrite)
		    {
		      tgi_setcolor(altColor);
		      if (ModeBold)
			{
			  tgi_setpixel(*px+1,*py);
			  tgi_setpixel(*px,*py+1);
			  tgi_setpixel(*px+1,*py+1);
			}
		      tgi_setpixel(*px,*py); 
		    }
		}

	      x += deltaX;
  	      b<<=1;
  	    }

	  y+=deltaY;
	  x-=width;
	  ++p;
  	}

      Coord->x+=width;
      x+=width;
      y-=height;
    }

  return;
  
}

/**
 * screen_tty_char - Called to plot chars when in tty mode
 */
void screen_tty_char(padByte theChar)
{
  if ((theChar >= 0x20) && (theChar < 0x7F)) {
    screen_char_draw(&TTYLoc, &theChar, 1);
    TTYLoc.x += CharWide;
  }
  else if ((theChar == 0x0b)) /* Vertical Tab */
    {
      TTYLoc.y += CharHigh;
    }
  else if ((theChar == 0x08) && (TTYLoc.x > 7))	/* backspace */
    {
      TTYLoc.x -= CharWide;
      tgi_setcolor(0);
      tgi_bar(scalex[TTYLoc.x],scaley[TTYLoc.y],scalex[TTYLoc.x+CharWide],scaley[TTYLoc.y+CharHigh]);
      tgi_setcolor(1);
    }
  else if (theChar == 0x0A)			/* line feed */
    TTYLoc.y -= CharHigh;
  else if (theChar == 0x0D)			/* carriage return */
    TTYLoc.x = 0;
  
  if (TTYLoc.x + CharWide > 511) {	/* wrap at right side */
    TTYLoc.x = 0;
    TTYLoc.y -= CharHigh;
  }
  
  if (TTYLoc.y < 0) {
    tgi_clear();
    TTYLoc.y=495;
  }

}

/**
 * screen_done()
 * Close down TGI
 */
void screen_done(void)
{
  tgi_done();
}
