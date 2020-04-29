/**
 * PLATOTerm64 - A PLATO Terminal for the Commodore 64
 * Based on Steve Peltz's PAD
 * 
 * Author: Thomas Cherryhomes <thom.cherryhomes at gmail dot com>
 *
 * plato.c - main program
 */

#include <stdbool.h>
#include "terminal.h"
#include "screen.h"
#include "touch.h"
#include "keyboard.h"
#include "io.h"

uint8_t already_started=false;
extern padByte splash[];
extern short splash_size;

extern int io_eof;

/**
 * greeting(void) - Show terminal greeting
 */
void greeting(void)
{
  ShowPLATO(splash,splash_size);
  t_initial_position();
}

void main(void)
{
  screen_init();
  io_init();
  touch_init();
  t_init();
  greeting();
  screen_beep();
  
  already_started=true;
  
  // And do the terminal
  while (!io_eof)
    {
      io_main();
      keyboard_main();
      touch_main();
    }
  
  screen_done();
  touch_done();
}
