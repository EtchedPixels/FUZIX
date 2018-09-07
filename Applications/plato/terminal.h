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
 * terminal_init()
 * Initialize terminal state
 */
void terminal_init(void);

/**
 * terminal_initial_position()
 * Set terminal initial position after splash screen.
 */
void terminal_initial_position(void);

/**
 * terminal_set_tty(void) - Switch to TTY mode
 */
void terminal_set_tty(void);

/**
 * terminal_set_plato(void) - Switch to PLATO mode
 */
void terminal_set_plato(void);

/**
 * terminal_get_features(void) - Inquire about terminal ASCII features
 */
uint8_t terminal_get_features(void);

/**
 * terminal_get_type(void) - Return the appropriate terminal type
 */
uint8_t terminal_get_type(void);

/**
 * terminal_get_subtype(void) - Return the appropriate terminal subtype
 */
uint8_t terminal_get_subtype(void);

/**
 * terminal_get_load_file(void) - Return the appropriate terminal loadfile (should just be 0)
 */
uint8_t terminal_get_load_file(void);

/**
 * terminal_get_configuration(void) - Return the terminal configuration
 */
uint8_t terminal_get_configuration(void);

/**
 * terminal_get_char_address(void) - Return the base address of the character set.
 */
uint16_t terminal_get_char_address(void);

/**
 * terminal_mem_read - Read a byte of program memory.
 * not needed for our terminal, but must
 * be decoded.
 */
padByte terminal_mem_read(padWord addr);

/**
 * terminal_mem_load - Write a byte to non-character memory.
 * not needed for our terminal, but must be decoded.
 */
void terminal_mem_load(padWord addr, padWord value);

/**
 * terminal_char_load - Store a character into the user definable
 * character set.
 */
void terminal_char_load(padWord charnum, charData theChar);

/**
 * terminal_mode_5, 6, and 7 are basically stubbed.
 */
void terminal_mode_5(padWord value);
void terminal_mode_6(padWord value);
void terminal_mode_7(padWord value);

/**
 * terminal_ext_allow - External Input allowed. Not implemented.
 */
void terminal_ext_allow(padBool allow);

/**
 * terminal_set_ext_in - Set which device to get input from.
 * Not implemented
 */
void terminal_set_ext_in(padWord device);

/**
 * terminal_set_ext_out - Set which device to send external data to.
 * Not implemented
 */
void terminal_set_ext_out(padWord device);

/**
 * terminal_ext_in - get an external input from selected device.
 * Not implemented.
 */
padByte terminal_ext_in(void);

/**
 * terminal_ext_out - Send an external output to selected device
 * Not implemented.
 */
void terminal_ext_out(padByte value);

#endif
