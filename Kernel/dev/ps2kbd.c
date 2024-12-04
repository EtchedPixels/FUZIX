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
 *	- Numlock
 *	- Windows keys
 *	- LED setting
 *	- Keyboard map setting
 *	- Add bottom halves to the Z80 irq logic so we can dispatch
 *	  work like the LED and beeper
 */

#include <kernel.h>
#include <tty.h>
#include <vt.h>
#include <input.h>
#include <keycode.h>
#include <devinput.h>
#include <ps2kbd.h>
#include <printf.h>

#ifdef CONFIG_VT_MULTI
extern uint8_t inputtty;		/* FIXME */
#else
#define inputtty 1
#endif

static const uint8_t keymap[256]  = {
    /* 00 - 0F */
    0, KEY_F9, 0, KEY_F5, KEY_F3, KEY_F1, KEY_F2, KEY_F12,
    0, KEY_F10, KEY_F8, KEY_F6, KEY_F4, CTRL('I'),'`',0,

    /* 10-1F */
    0, 0/*LA*/, 0/*LS*/, 0, 0/*LC*/, 'q', '1', 0,
    0, 0, 'z', 's', 'a', 'w', '2', 0,

#define LALT 0x11
#define LSHIFT 0x12
#define LCTRL 0x14

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
    0/*CL*/, 0/*RS*/, KEY_ENTER, ']', 0, '#', 0, 0,

#define CAPSLOCK 0x58
#define RSHIFT 0x59

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

#define RALT 0x91
#define RCTRL 0x94
#define LWIN 0x9F

    /* E0 20 - E0 2F */
    0, 'C', 'X', 'D', 'E', '$', KEY_POUND, 0/*RWIN*/,
    0, 0, 'V', 'F', 'T', 'R', '%', 0/*WINMENU*/,

#define RWIN 0xA7

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
static uint8_t alt_down;
static uint8_t capslock;
static uint8_t numlock;

static uint_fast8_t alpha(uint_fast8_t c)
{
    if (c >= 'A' && c <= 'Z')
        return 1;
    if (c >= 'a' && c <= 'z')
        return 1;
    return 0;
}


static void keycode(uint_fast8_t code, uint_fast8_t up, uint_fast8_t shifted)
{
    static uint_fast8_t brk;
    uint_fast8_t key;
    uint_fast8_t m = 0;


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
            ctrl_down |= ~2;
        else if (code == LALT)
            alt_down &= ~1;
        else if (code == RALT)
            alt_down &= ~2;
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
    else if (code == LALT)
        alt_down |= 1;
    else if (code == RALT)
        alt_down |= 2;

    /* caps lock, shift and friends all send autorepeat so care needed */
    if (code == CAPSLOCK) {
        capslock ^= up;		/* On the up toggle capslock */
        /* Eventually we need to drive the LEDs */
        return;
    }

    key = keymap[code];

#ifdef CONFIG_VT_MULTI
    if (alt_down && key >= KEY_F1 && key <= KEY_F12) {
        ps2kbd_conswitch(key - KEY_F1 + 1);
        return;
    }
#endif


/*    kprintf("Code %d Key %d KG %d IT %d\n",  *
 *        code, key, keyboard_grab, inputtty); */

    /* FIXME: need to handle keypad / (0xCA) which is elsewhere
       - review - not all bits shiftable ? */
    if (numlock && code >= 0x68 && code < 0x7E)
        code ^= 0x80;
    if (shift_down) {
        m = KEYPRESS_SHIFT;
        /* FIXME: duplicate any blank slots to remove second condition */
        if (code < 0x80 && keymap[code + 0x80])
            key = keymap[code + 0x80];
    }
    if (ctrl_down) {
        m |= KEYPRESS_CTRL;
        key &= 31;
    }
    if (alt_down)
        m |= KEYPRESS_ALT;

    if (capslock && alpha(key))
        key ^= 32;

    if (key) {
        switch(keyboard_grab) {
        case 0:
            vt_inproc(inputtty, key);
            break;
        case 1:
            if (!input_match_meta(key)) {
                vt_inproc(inputtty, key);
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
        keycode(byte, up, shifted);
        up = 0;
        shifted = 0;
    }
}

uint_fast8_t ps2busy = 0;

static void kbd_set_leds(uint_fast8_t byte)
{
    ps2busy = 1;
    ps2kbd_put(0xED);
    ps2kbd_put(byte & 7);
    ps2busy = 0;
}

int ps2kbd_init(void)
{
    int16_t r;
    uint_fast8_t present = 0;
    uint_fast8_t i;

    ps2busy = 1;

    /* We may have FF or FF AA or FF AA 00 or other info queued before
       our reset, if so empty it out */
    for (i = 0; i < 4; i++) {
        r = ps2kbd_get();
        if (r == PS2_NOCHAR)
            break;
        if (r >= 0)		/* It sent us data */
            present = 1;
    }

    /* Try to reset, if it times out -> no keyboard */
    r = ps2kbd_put(0xFF);
    if (r == 0)
        r = ps2kbd_get();
    if (r != 0xFA) {
        ps2kbd_get();
        if ((r & 0x8000) && present == 0) {
            ps2busy = 0;
            return 0;
        }
    }

    ps2kbd_put(0xF6);	/* Restore default */
    ps2kbd_get();	/* Eat the reply */
    ps2kbd_put(0xED);	/* LEDs off */
    ps2kbd_put(0x00);
    ps2kbd_put(0xF0);	/* Scan code 2 */
    ps2kbd_get();
    ps2kbd_put(0x02);
    ps2kbd_get();
    ps2kbd_put(0xF4);	/* Clear buffer and enable */

    present = 1;

    ps2busy = 0;
    return present;
}
