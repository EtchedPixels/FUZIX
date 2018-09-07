/**
 * PLATOTerm64 - A PLATO Terminal for the Commodore 64
 * Based on Steve Peltz's PAD
 * 
 * Author: Thomas Cherryhomes <thom.cherryhomes at gmail dot com>
 *
 * screen.c - Display output functions
 */

#include <stdint.h>
#include <unistd.h>

static uint8_t bp = 0;
static uint8_t bd = 0;

/**
 * screen_load_driver()
 * Load the TGI driver
 */
void screen_load_driver(void)
{
}

/**
 * screen_init_hook()
 * Called after tgi_init to set any special features, e.g. nmi trampolines.
 */
void screen_init_hook(void)
{
}

/**
 * Set the terminal colors
 */
void screen_update_colors(void)
{
}

/**
 * Wait(void) - Sleep for approx 16.67ms
 */
void screen_wait(void)
{
	// TODO: 60hz wait
}

/**
 * screen_beep(void) - Beep the terminal
 */
void screen_beep(void)
{
	char c = '\007';
	write(1, &c, 1);
}
