/**
 * PLATOTerm64 - A PLATO Terminal for the Commodore 64
 * Based on Steve Peltz's PAD
 * 
 * Author: Thomas Cherryhomes <thom.cherryhomes at gmail dot com>
 *
 * terminal.c - Terminal state functions
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include "terminal.h"
#include "screen.h"

/**
 * ASCII Features to return in Features
 */
#define ASC_ZFGT        0x01
#define ASC_ZPCKEYS     0x02
#define ASC_ZKERMIT     0x04
#define ASC_ZWINDOW     0x08

/**
 * protocol.c externals
 */
extern CharMem CurMem;
extern padBool TTY;
extern padBool ModeBold;
extern padBool Rotate;
extern padBool Reverse;
extern DispMode CurMode;
extern padBool FlowControl;

/**
 * screen.c externals
 */
extern uint8_t CharWide;
extern uint8_t CharHigh;
extern padPt TTYLoc;

extern uint8_t already_started;

/**
 * t_init()
 * Initialize terminal state
 */

static struct termios saved_term;

void exit_cleanup(void)
{
	tcsetattr(0, TCSADRAIN, &saved_term);
}

void cleanup(int sig)
{
	exit(1);
}

void t_init(void)
{
	struct termios term;

	if (tcgetattr(0, &term) == 0) {
		memcpy(&saved_term, &term, sizeof(saved_term));
		atexit(exit_cleanup);
		signal(SIGINT, cleanup);
		signal(SIGQUIT, cleanup);
		signal(SIGPIPE, cleanup);
		term.c_lflag &= ~(ICANON | ECHO);
		term.c_cc[VMIN] = 0;
		term.c_cc[VTIME] = 1;
		term.c_cc[VINTR] = 0;
		term.c_cc[VSUSP] = 0;
		term.c_cc[VSTOP] = 0;
		tcsetattr(0, TCSADRAIN, &term);
	}
	t_set_tty();
}

/**
 * t_initial_position()
 * Set terminal initial position after splash screen.
 */
void t_initial_position(void)
{
	TTYLoc.x = 0;
	TTYLoc.y = 100;		// Right under splashscreen.
}

/**
 * t_set_tty(void) - Switch to TTY mode
 */
void t_set_tty(void)
{
	if (already_started)
		screen_clear();
	TTY = true;
	ModeBold = padF;
	Rotate = padF;
	Reverse = padF;
	CurMem = M0;
	/* CurMode=ModeRewrite; */
	CurMode = ModeWrite;	/* For speed reasons. */
	CharWide = 8;
	CharHigh = 16;
	TTYLoc.x = 0;		// leftmost coordinate on screen
	TTYLoc.y = 495;		// Top of screen - one character height
}

/**
 * t_set_plato(void) - Switch to PLATO mode
 */
void t_set_plato(void)
{
	TTY = false;
	screen_clear();
}

/**
 * t_get_features(void) - Inquire about terminal ASCII features
 */
uint8_t t_get_features(void)
{
	return ASC_ZFGT;	/* This terminal can do Fine Grained Touch (FGT) */
}

/**
 * t_get_type(void) - Return the appropriate terminal type
 */
uint8_t t_get_type(void)
{
	return 12;		/* ASCII terminal type */
}

/**
 * t_get_subtype(void) - Return the appropriate terminal subtype
 */
uint8_t t_get_subtype(void)
{
	return 1;		/* ASCII terminal subtype IST-III */
}

/**
 * t_get_load_file(void) - Return the appropriate terminal loadfile (should just be 0)
 */
uint8_t t_get_load_file(void)
{
	return 0;		/* This terminal does not load its resident from the PLATO system. */
}

/**
 * t_get_configuration(void) - Return the terminal configuration
 */
uint8_t t_get_configuration(void)
{
	return 0x40;		/* Touch panel is present. */
}

/**
 * t_get_char_address(void) - Return the base address of the character set.
 */
uint16_t t_get_char_address(void)
{
	return 0x3000;		/* What the? Shouldn't this be 0x3800? */
}

/**
 * t_mem_read - Read a byte of program memory.
 * not needed for our terminal, but must
 * be decoded.
 */
padByte t_mem_read(padWord addr)
{
	return (0xFF);
}

/**
 * t_mem_load - Write a byte to non-character memory.
 * not needed for our terminal, but must be decoded.
 */
void t_mem_load(padWord addr, padWord value)
{
	/* Not Implemented */
}

/**
 * Mode5, 6, and 7 are basically stubbed.
 */
void t_mode_5(padWord value)
{
}

void t_mode_6(padWord value)
{
}

void t_mode_7(padWord value)
{
}

/**
 * t_ext_allow - External Input allowed. Not implemented.
 */
void t_ext_allow(padBool allow)
{
	/* Not Implemented */
}

/**
 * t_set_ext_in - Set which device to get input from.
 * Not implemented
 */
void t_set_ext_in(padWord device)
{
}

/**
 * t_set_ext_out - Set which device to send external data to.
 * Not implemented
 */
void t_set_ext_out(padWord device)
{
}

/**
 * t_ext_in - get an external input from selected device.
 * Not implemented.
 */
padByte t_ext_in(void)
{
	return 0;
}

/**
 * t_ext_out - Send an external output to selected device
 * Not implemented.
 */
void t_ext_out(padByte value)
{
}

/**
 * t_char_load - load character in user set
 * Not implemented.
 */
void t_char_load(padWord charnum, charData theChar)
{
}

