/*
 *	PS/2 port abstraction
 */

#include <kernel.h>
#include <ps2mouse.h>
#include <ps2kbd.h>
#include "rcbus.h"

int ps2_type = PS2_NONE;

/* Bitbang */
extern uint16_t ps2bmouse_get(void);
extern uint16_t ps2bmouse_put(uint8_t c) __fastcall;
extern uint16_t ps2bkbd_get(void);
extern uint16_t ps2bkbd_put(uint8_t c) __fastcall;
extern void ps2bkbd_beep(void);

/* Direct */
extern uint16_t ps2dmouse_get(void);
extern uint16_t ps2dmouse_put(uint8_t c);
extern uint16_t ps2dkbd_get(void);
extern uint16_t ps2dkbd_put(uint8_t c);
extern void ps2dkbd_beep(void);

uint16_t ps2kbd_get(void)
{
    switch(ps2_type) {
    case PS2_DIRECT:
        return ps2dkbd_get();
    case PS2_BITBANG:
        return ps2bkbd_get();
    }
    return -1;
}

int16_t ps2kbd_put(uint8_t c)
{
    switch(ps2_type) {
    case PS2_DIRECT:
        return ps2dkbd_put(c);
    case PS2_BITBANG:
        return ps2bkbd_put(c);
    }
    return -1;
}

uint16_t ps2mouse_get(void)
{
    switch(ps2_type) {
    case PS2_DIRECT:
        return ps2dmouse_get();
    case PS2_BITBANG:
        return ps2bmouse_get();
    }
    return -1;
}

int16_t ps2mouse_put(uint8_t c)
{
    switch(ps2_type) {
    case PS2_DIRECT:
        return ps2dmouse_put(c);
    case PS2_BITBANG:
        return ps2bmouse_put(c);
    }
    return -1;
}

void ps2kbd_beep(void)
{
    switch(ps2_type) {
    case PS2_DIRECT:
        ps2dkbd_beep();
        break;
    case PS2_BITBANG:
        ps2bkbd_beep();
        break;
    }
}
        