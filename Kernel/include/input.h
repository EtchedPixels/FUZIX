#ifndef _INPUT_H
#define _INPUT_H

#define INPUT_MAX_META	8

/* Low four bits indicate device number usually */

#define MOUSE_REL	0x00		/* 8bit deltas - may need 16 ? */
#define MOUSE_ABS	0x10		/* 16bit x 16bit virtual coords */
#define STICK_DIGITAL	0x20		/* UDLR + buttons byte */
#define 	STICK_DIGITAL_U	0x80
#define		STICK_DIGITAL_D 0x40
#define		STICK_DIGITAL_L	0x20
#define		STICK_DIGITAL_R	0x10
#define STICK_ANALOG	0x30		/* 16bit signed X / Y, sign */

/* No device number - but 3 bits reserved if needed */
#define KEYPRESS_CODE	0x40		/* Followed by keycode byte. Number
                                           bits show u/d and modifiers */
#define		KEYPRESS_DOWN	0x00
#define		KEYPRESS_UP	0x01
#define		KEYPRESS_SHIFT	0x02
#define		KEYPRESS_CTRL	0x04
#define		KEYPRESS_ALT	0x08

#define	LIGHTPEN_ABS	0x50		/* Light pen or similar, 16 x 16bit virtual coords */

#define MOUSE_REL_WHEEL	0x60		/* As MOUSE_REL but with a wheel byte */

#define BUTTON(x)	(1 << (x))	/* Button 1-7 (top bit never used) */
#define BUTTONS(x)	((x)&0x07)	/* 0-7 buttons */


#define INPUT_GRABKB	0x0520
#define		INPUT_GRAB_NONE		0	/* No grab */
#define		INPUT_GRAB_META		1	/* Special keys only */
#define		INPUT_GRAB_TYPED	2	/* Typed input */
#define		INPUT_GRAB_ALL		3	/* Up and down events */
#define INPUT_SETMETA	0x0521

/*
 *	Input device methods
 */
extern int inputdev_read(uint_fast8_t flag);
extern int inputdev_write(uint_fast8_t flag);
extern int inputdev_ioctl(uarg_t request, char *data);
extern int inputdev_close(void);

/*
 *	Exposed for keyboard drivers
 */
extern uint8_t keyboard_grab;
extern uint_fast8_t input_match_meta(uint_fast8_t);
/*
 *	Platform methods for input device if present
 */
extern int platform_input_read(uint8_t *);
extern void platform_input_wait(void);
extern int platform_input_write(uint_fast8_t);
extern uint_fast8_t platform_input_init(void);

#endif
