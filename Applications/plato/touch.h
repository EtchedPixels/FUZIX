/**
 * PLATOTerm64 - A PLATO Terminal for the Commodore 64
 * Based on Steve Peltz's PAD
 * 
 * Author: Thomas Cherryhomes <thom.cherryhomes at gmail dot com>
 *
 * touch.c - Touchscreen functions
 */

#ifndef TOUCH_H
#define TOUCH_H

#include "protocol.h"

/**
 * touch_init() - Set up touch screen
 */
void touch_init(void);

/**
 * touch_main() - Main loop for touch screen
 */
void touch_main(void);

/**
 * touch_allow - Set whether touchpanel is active or not.
 */
void touch_allow(padBool allow);

/**
 * handle_mouse - Process mouse events and turn into scaled touch events
 */
void handle_mouse(void);

/**
 * touch_hide() - hide the mouse cursor
 */
void touch_hide(void);

/**
 * touch_done() - Stop the mouse driver
 */
void touch_done(void);

#endif /* TOUCH_H */
