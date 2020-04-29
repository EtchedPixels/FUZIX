/**
 * PLATOTerm64 - A PLATO Terminal for the Commodore 64
 * Based on Steve Peltz's PAD
 * 
 * Author: Thomas Cherryhomes <thom.cherryhomes at gmail dot com>
 *
 * terminal.c - Terminal state functions
 */

#ifndef TERMINAL_H
#define TERMINAL_H

#include "protocol.h"

/**
 * t_init()
 * Initialize terminal state
 */
void t_init(void);

/**
 * t_initial_position()
 * Set terminal initial position after splash screen.
 */
void t_initial_position(void);

/**
 * t_set_tty(void) - Switch to TTY mode
 */
void t_set_tty(void);

/**
 * t_set_plato(void) - Switch to PLATO mode
 */
void t_set_plato(void);

/**
 * t_get_features(void) - Inquire about terminal ASCII features
 */
uint8_t t_get_features(void);

/**
 * t_get_type(void) - Return the appropriate terminal type
 */
uint8_t t_get_type(void);

/**
 * t_get_subtype(void) - Return the appropriate terminal subtype
 */
uint8_t t_get_subtype(void);

/**
 * t_get_load_file(void) - Return the appropriate terminal loadfile (should just be 0)
 */
uint8_t t_get_load_file(void);

/**
 * t_get_configuration(void) - Return the terminal configuration
 */
uint8_t t_get_configuration(void);

/**
 * t_get_char_address(void) - Return the base address of the character set.
 */
uint16_t t_get_char_address(void);

/**
 * t_mem_read - Read a byte of program memory.
 * not needed for our terminal, but must
 * be decoded.
 */
padByte t_mem_read(padWord addr);

/**
 * t_mem_load - Write a byte to non-character memory.
 * not needed for our terminal, but must be decoded.
 */
void t_mem_load(padWord addr, padWord value);

/**
 * t_char_load - Store a character into the user definable
 * character set.
 */
void t_char_load(padWord charnum, charData theChar);

/**
 * t_mode_5, 6, and 7 are basically stubbed.
 */
void t_mode_5(padWord value);
void t_mode_6(padWord value);
void t_mode_7(padWord value);

/**
 * t_ext_allow - External Input allowed. Not implemented.
 */
void t_ext_allow(padBool allow);

/**
 * t_set_ext_in - Set which device to get input from.
 * Not implemented
 */
void t_set_ext_in(padWord device);

/**
 * t_set_ext_out - Set which device to send external data to.
 * Not implemented
 */
void t_set_ext_out(padWord device);

/**
 * t_ext_in - get an external input from selected device.
 * Not implemented.
 */
padByte t_ext_in(void);

/**
 * t_ext_out - Send an external output to selected device
 * Not implemented.
 */
void t_ext_out(padByte value);

#endif
