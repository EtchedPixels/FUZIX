/**
 * PLATOTerm64 - A PLATO Terminal for the Commodore 64
 * Based on Steve Peltz's PAD
 * 
 * Author: Thomas Cherryhomes <thom.cherryhomes at gmail dot com>
 *
 * touch.c - Touchscreen functions
 */

#include <stdbool.h>
#include <string.h>
#include "touch.h"

static padBool TouchActive;

static uint16_t screen_w;
static uint16_t screen_h;
static uint8_t mouse_present=false;
static uint8_t mou_res=0;

extern uint16_t scaletx[];
extern uint16_t scalety[];

/**
 * touch_init() - Set up touch screen
 */
void touch_init(void)
{
}

/**
 * touch_allow - Set whether touchpanel is active or not.
 */
void touch_allow(padBool allow)
{
  TouchActive=allow;
}

/**
 * touch_main - Process mouse events and turn into scaled touch events
 */
void touch_main(void)
{
}

/**
 * touch_hide() - hide the mouse cursor
 */
void touch_hide(void)
{
}

/**
 * touch_done() - Stop the mouse driver
 */
void touch_done(void)
{
}
