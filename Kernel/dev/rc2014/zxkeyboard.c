/*
 *	High level support for the ZXKey interface. The real work is done
 *	in the asm code
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <keycode.h>
#include <vt.h>
#include <tty.h>
#include <input.h>
#include <devinput.h>
#include <rc2014.h>
#include <zxkey.h>


struct vt_repeat keyrepeat = { 50, 5 };

extern uint16_t keybits;

void zxkey_poll(void)
{
    unsigned int r = zxkey_scan();
    uint8_t c = r;

    if (c == 0)
        return;

#ifdef CONFIG_VT_MULTI
    /* We pass a special flag back for a console change */
    if ((r >> 8) == 0x40) {
        if (inputtty != c)
            do_conswitch(c);
        return;
    }
#endif

    switch(keyboard_grab) {
    case 0:
        vt_inproc(inputtty, c);
        break;
    case 1:
        if (!input_match_meta(c)) {
            vt_inproc(inputtty, c);
            break;
        }
        /* Fall through */
    case 2:
        queue_input(KEYPRESS_DOWN);
        queue_input(c);
        break;
    case 3:
	/* Queue an event giving the base key (unshifted)
	   and the state of shift/ctrl/alt */
	queue_input(KEYPRESS_DOWN | (r >> 8));
	queue_input(keyboard[keybits >> 8][keybits & 0xFF]);
    }
}

/* Helper from the asm code */
void zxkey_queue_key(uint8_t row, uint8_t col)
{
    queue_input(KEYPRESS_UP);
    queue_input(keyboard[row][col]);
}

