/**
 * PLATOTerm64 - A PLATO Terminal for the Commodore 64
 * Based on Steve Peltz's PAD
 * 
 * Author: Thomas Cherryhomes <thom.cherryhomes at gmail dot com>
 *
 * keyboard.c - Keyboard functions (apple2)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include "../screen.h"
#include "../keyboard.h"
#include "../protocol.h"
#include "key.h"

static uint8_t shift_lock = false;
static uint8_t is_escape = false;
extern uint8_t xoff_enabled;
extern padBool TTY;
extern uint8_t io_eof;

/**
 * keyboard_main - Handle the keyboard presses
 */
void keyboard_main(void)
{
	uint8_t ch;

	fflush(stdout);
	switch (read(0, &ch, 1)) {
	case 0:
		return;
	case -1:
		if (errno != EAGAIN) {
			perror("read");
			exit(1);
		}
		return;
	}
	if (is_escape == true && ch == 0x1A) {	// ESC CTRL-Z
		io_eof = 1;
		return;
	}
	if (is_escape == true && ch == 0x1B)	// ESC
	{
		screen_beep();

		if (shift_lock == true)
			shift_lock = false;
		else
			shift_lock = true;

		is_escape = false;
	} else if (is_escape == false && ch == 0x1B)
		is_escape = true;
	else if (TTY) {
		keyboard_out_tty(ch);
	} else if (is_escape == true) {
		keyboard_out(esc_key_to_pkey[ch]);
		is_escape = false;
	} else if (shift_lock == true) {
		keyboard_out(shiftlock_key_to_pkey[ch]);
	} else {
		keyboard_out(key_to_pkey[ch]);
	}
}

/**
 * keyboard_clear() - Clear the keyboard buffer
 */
void keyboard_clear(void)
{
	/* We can do this but do we need to ? */
}
