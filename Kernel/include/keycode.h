#ifndef _KEYCODE_H
#define _KEYCODE_H

/*
 *	Standardised keycodes. Used when we have 8bit character maps. Some
 *	platforms needing unicode symbols may well want to use 16bit maps
 *	and encode the table differently - please key the same esc/ctrl codes
 *	however.
 */


#define ESC(x)		(0x80|(x))

#define KEY_DEL		0x7F
#define KEY_BS		CTRL('H')
#define KEY_ESC		CTRL('[')

#define KEY_LEFT	ESC('D')
#define KEY_RIGHT	ESC('C')
#define KEY_UP		ESC('A')
#define KEY_DOWN	ESC('B')

#define KEY_HOME	ESC('H')
#define KEY_INSERT	ESC('i')
#define KEY_COPY	ESC('c')
#define KEY_PASTE	ESC('v')
#define KEY_CUT		ESC('x')
#define KEY_CANCEL	ESC('a')
#define KEY_EXTRA	ESC('e')
#define KEY_PRINT	ESC(']')
#define KEY_STOP	CTRL('C')
#define KEY_DELR	ESC('r')
#define KEY_PLUS	ESC('+')
#define KEY_MINUS	ESC('-')
#define KEY_EXIT	ESC('q')

#define KEY_F1		ESC('1')
#define KEY_F2		ESC('2')
#define KEY_F3		ESC('3')
#define KEY_F4		ESC('4')
#define KEY_F5		ESC('5')
#define KEY_F6		ESC('6')
#define KEY_F7		ESC('7')
#define KEY_F8		ESC('8')
#define KEY_F9		ESC('9')
#define KEY_F10		ESC('0')

/* Specials that vt.c will expand - in the range 0x80-0x9F. These are the odd
   2 byte codes that sneak into UK and US type keyboards */

#define KEY_POUND	0x80
#define KEY_HALF	0x81
#define KEY_EURO	0x82
#define KEY_DOT		0x83

#endif
