/*
 *	General PS/2 keyboard logic
 *
 *	The normal key flow for set 2 is
 *
 *	[Code]	= Key down
 *	[0xF0][Code] = Key up
 *
 *	[0xE0] acts as a shift and goes before up or down keys to allow for
 *	extra keys
 *
 *	There are then the special cases
 *	PrtScr is sent as two codes E012 + E07C
 *	Pause/Break is a key down only and sends the 8 byte sequence
 *	E11477 E1F014 E077
 *
 *	TODO
 *	- Pause
 *	- Alt keys
 *	- Numlock/num pad rules
 *	- Windows keys
 *	- LED setting
 *	- Lock keys
 */

#include <kernel.h>
#include <tty.h>
#include <vt.h>
#include <input.h>
#include <keycode.h>
#include <devinput.h>
#include <ps2kbd.h>

extern uint8_t inputtty;		/* FIXME */

static uint8_t keymap[256]  = {
    /* 00 - 0F */
    0, KEY_F9, 0, KEY_F5, KEY_F3, KEY_F1, KEY_F2, KEY_F12,
    0, KEY_F10, KEY_F8, KEY_F6, KEY_F4, CTRL('I'),'`',0,

    /* 10-1F */
    0, 0/*LA*/, 0/*LS*/, 0, 0/*LC*/, 'q', '1', 0,
    0, 0, 'z', 's', 'a', 'w', '2', 0,

#define LALT 0x11
#define LSHIFT 0x13
#define LCTRL 0x15

    /* 20-2F */
    0, 'c', 'x', 'd', 'e', '4', '3', 0,
    0, ' ', 'v', 'f', 't', 'r', '5', 0,

    /* 30-3F */    
    0, 'n', 'b', 'h', 'g', 'y', '6', 0,
    0, 0, 'm', 'j', 'u', '7', '8', 0,

    /* 40-4F */    
    0, ',', 'k', 'i', 'o', '0', '9', 0,
    0, '.', '/', 'l', ';', 'p', '-', 0,

    /* 50-5F */
    0, 0, '\'', 0, '[', '=', 0, 0,
    KEY_CAPSLOCK, 0/*RS*/, KEY_ENTER, ']', 0, '#', 0, 0,

#define CAPSLOCK 0x58    
#define RSHIFT 0x5A

    /* 60-6F */
    0, '\\', 0, 0, 0, 0, KEY_BS, 0,
    0, '1', 0, '4', '7', 0, 0, 0,		/* Keypad */

    /* 70-7F */
    0, '.', '2', '5', '6', '8', KEY_ESC, 0/*NLCK*/, 
    KEY_F11, '+', '3', '-', '*', '9', 0/*SCRLCK*/, 0,
    
    /* E0 Shifted codes */

    /* E0 00 - E0 0F */
    0, 0, 0, KEY_F7, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0/*FIXME*/, 0,

    /* E0 10 - E0 1F */
    0, 0/*RALT*/, 0, 0, 0/*RCTRL*/, 'Q', '!', 0,
    0, 0, 'Z', 'S', 'A', 'W', '"', 0/*LWIN*/,

#define RALT 0x81
#define RCTRL 0x84
#define LWIN 0x8F

    /* E0 20 - E0 2F */
    0, 'C', 'X', 'D', 'E', '$', KEY_POUND, 0/*RWIN*/,
    0, 0, 'V', 'F', 'T', 'R', '%', 0/*WINMENU*/,

#define RWIN 0x97

    /* E0 30 - E0 3F */
    0, 'N', 'B', 'H', 'G', 'Y', '^', 0,
    0, 0, 'M', 'J', 'U', '&', '*', 0,

    /* E0 40 - E0 4F */
    0, '<', 'K', 'T', 'O', ')', '(', 0,
    0, '>', '/', 'L', ':', 'P', '_', 0,		/* / is on keypad */

    /* E0 50 - E0 5F */
    0, 0, '@', 0, '{', '+', 0, 0,
    0, 0, KEY_ENTER, '}', 0, '~', 0, 0,
    
    /* E0 60 - E0 6F */
    0, '|', 0, 0, 0, 0, 0, 0,
    0, KEY_END, 0, KEY_LEFT, KEY_HOME, 0, 0, 0,

    /* E0 70 - E0 7F */
    KEY_INSERT, KEY_DEL, KEY_DOWN, 0, KEY_RIGHT, KEY_UP, 0, 0,
    0, 0, KEY_PGDOWN, 0, 0, KEY_PGUP, 0, 0    
};

static uint8_t shift_down;
static uint8_t ctrl_down;
static uint8_t capslock;
static uint8_t numlock;

static uint8_t alpha(uint8_t c)
{
    if (c >= 'A' && c <= 'Z')
        return 1;
    if (c >= 'a' && c <= 'z')
        return 1;
    return 0;
}


static void keycode(uint_fast8_t code, uint_fast8_t up, uint_fast8_t shifted)
{
    static uint8_t brk;
    uint8_t key;
    uint8_t m = 0;


    if (brk) {
        --brk;
        /* TODO when brk hits 0 report it properly */
        return;
    }

    /* The pause/brk key and the weird add on multimedia keys */
    if (code == 0xE1) {
        brk = 7;
        return;
    }
    /* A stray ACK */
    if (code == 0xAA)
        return;
    /* Lost char/error */
    if (code == 0xFF)
        return;

    if (shifted)
        code |= 0x80;

    if (up) {
        if (keyboard_grab == 3) {
            queue_input(KEYPRESS_UP);
            queue_input(code);
        }
        /* TODO - alt */
        if (code == LSHIFT)
            shift_down &= ~1;
        else if (code == RSHIFT)
            shift_down &= ~2;
        else if (code == LCTRL)
            ctrl_down &= ~1;
        else if (code == RCTRL)
            ctrl_down |= ~1;
        return;
    }
    if (code == LSHIFT)
        shift_down |= 1;
    else if (code == RSHIFT)
        shift_down |= 2;
    else if (code == LCTRL)
        ctrl_down |= 1;
    else if (code == RCTRL)
        ctrl_down |= 2;

    key = keymap[code];

    /* The keyboard deals with shifting but not the lock keys */

    /* FIXME: need to handle keypad / which is elsewhere
       - review - not all bits shiftable ? */
    if (numlock && code >= 0x68 && code < 0x7E)
        code ^= 0x80;

    if (shift_down)
        m = KEYPRESS_SHIFT;
    if (ctrl_down) {
        m |= KEYPRESS_CTRL;
        key &= 31;
    }

    if (capslock && alpha(key))
        key ^= 32;


    if (key) {
        switch(keyboard_grab) {
        case 0:
            vt_inproc(inputtty + 1, key);
            break;
        case 1:
            if (!input_match_meta(key)) {
                vt_inproc(inputtty + 1, key);
                break;
            }
            /* Fall through */
        case 2:
            queue_input(KEYPRESS_DOWN);
            queue_input(key);
            break;
        case 3:
            queue_input(KEYPRESS_DOWN | m);
            queue_input(code);
            break;
        }
    }
}    

void ps2kbd_byte(uint_fast8_t byte)
{
    static uint_fast8_t up, shifted;

    if (byte == 0xE0)
        shifted = 1;
    else if (byte == 0xF0)
        up = 1;
    else {
        keycode(byte, up,shifted);
        up = 0;
        shifted = 0;
    }
}

void ps2kbd_poll(void)
{
    int n = ps2kbd_get();
    if (n != -1)
        ps2kbd_byte(n);
}

static void kbd_set_leds(uint_fast8_t byte)
{
    ps2kbd_put(0xED);
    ps2kbd_put(byte & 7);
}

int ps2kbd_init(void)
{
    /* Try to reset, if it times out -> no keyboard */
    if (ps2kbd_put(0xFF) != 0xFA)
        return 0;
    ps2kbd_put(0xF6);	/* Restore default */
    ps2kbd_put(0xED);	/* LEDs off */
    ps2kbd_put(0x00);
    ps2kbd_put(0xF0);	/* Scan code 2 */
    ps2kbd_put(0x02);
    ps2kbd_put(0xF4);	/* Clear buffer and enable */
    return 1;
}
