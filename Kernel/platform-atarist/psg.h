#ifndef _PSG_H
#define _PSG_H

/*
 *	YM-2149 sound generator access. Includes poking around at stuff
 *	wired to its GPIO.
 *
 *	Based on EmuTOS
 */

struct psg {
    uint8_t control;
    uint8_t pad0;
    uint8_t data;
};

#define PSG	((volatile struct psg *)0xff8800)

#define PSG_PORT_A	0x0E
#define PSG_PORT_B	0x0F

#endif
