/*
 *	Extract out the VT support and clean it up
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <tty.h>
#include <graphics.h>
#include <devtty.h>
#include <rcbus.h>
#include <vt.h>
#include "multivt.h"

uint8_t curvid;
uint8_t vidcard[5];
uint8_t outputtty = 1, inputtty = 1;

struct vt_switch ttysave[VT_CON];

void cursor_off(void)
{
        if (curvid == VID_TMS9918A)
            tms_cursor_off();
        else if (curvid == VID_EF9345)
            ef_cursor_off();
        else if (curvid == VID_MACCA)
            ma_cursor_off();
}

void cursor_disable(void)
{
        if (curvid == VID_TMS9918A)
            tms_cursor_disable();
        else if (curvid == VID_EF9345)
            ef_cursor_disable();
        else if (curvid == VID_MACCA)
            ma_cursor_disable();
}

void cursor_on(int8_t y, int8_t x)
{
        if (curvid == VID_TMS9918A)
            tms_cursor_on(y,x);
        else if (curvid == VID_EF9345)
            ef_cursor_on(y,x);
        else if (curvid == VID_MACCA)
            ma_cursor_on(y,x);
}

void plot_char(int8_t y, int8_t x, uint16_t c)
{
        if (curvid == VID_TMS9918A)
            tms_plot_char(y, x, c);
        else if (curvid == VID_EF9345)
            ef_plot_char(y, x, c);
        else if (curvid == VID_MACCA)
            ma_plot_char(y, x, c);
}

void clear_lines(int8_t y, int8_t ct)
{
        if (curvid == VID_TMS9918A)
            tms_clear_lines(y, ct);
        else if (curvid == VID_EF9345)
            ef_clear_lines(y, ct);
        else if (curvid == VID_MACCA)
            ma_clear_lines(y, ct);
}

void clear_across(int8_t y, int8_t x, int16_t l)
{
        if (curvid == VID_TMS9918A)
            tms_clear_across(y, x, l);
        else if (curvid == VID_EF9345)
            ef_clear_across(y, x, l);
        else if (curvid == VID_MACCA)
            ma_clear_across(y, x, l);
}

void vtattr_notify(void)
{
        if (curvid == VID_EF9345)
            ef_vtattr_notify();
        else if (curvid == VID_MACCA)
            ma_vtattr_notify();
}

void scroll_up(void)
{
        if (curvid == VID_TMS9918A)
            tms_scroll_up();
        else if (curvid == VID_EF9345)
            ef_scroll_up();
        else if (curvid == VID_MACCA)
            ma_scroll_up();
}

void scroll_down(void)
{
        if (curvid == VID_TMS9918A)
            tms_scroll_down();
        else if (curvid == VID_EF9345)
            ef_scroll_down();
        else if (curvid == VID_MACCA)
            ma_scroll_down();
}

/* Callback from the keyboard driver for a console switch */
/* TODO: allow multiple console types at once */
void do_conswitch(uint8_t c)
{
	uint8_t v = vidcard[c];

	/* No switch if the console is locked for graphics */
	if (vswitch)
		return;

	/* No card on this VT port */
	if (v == VID_NONE)
		return;

	/* Cursor off on whichever display loses focus */
	vt_cursor_off();

	curvid = v;

	switch(curvid) {
	case VID_TMS9918A:
		/* TMS9918A */
		tms_setoutput(inputtty);
		inputtty = c;
		tms_set_console();
		tms_setoutput(c);
		break;
	case VID_EF9345:
		inputtty = c;
		ef_setoutput(inputtty);
		ef_set_console();
		break;
	case VID_MACCA:
		inputtty = c;
		ma_setoutput(inputtty);
		ma_set_console();
		break;
	}
	vt_cursor_on();
	/* Now restore the output state - this is ugly and needs work */
	curvid = vidcard[outputtty];
	switch(curvid) {
	case VID_TMS9918A:
		tms_setoutput(outputtty);
		break;
	case VID_EF9345:
		ef_setoutput(outputtty);
		break;
	case VID_MACCA:
		ma_setoutput(outputtty);
		break;
	}
}

/* PS/2 call back for alt-Fx */
void ps2kbd_conswitch(uint8_t console)
{
	if (console > 4 || console == inputtty || vswitch)
		return;
	do_conswitch(console);
}
