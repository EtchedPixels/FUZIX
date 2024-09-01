#include <stdio.h>
#include "utk.h"

struct utk_menu edit = {
	NULL,
	"Edit",
	"Cut/Copy/Paste/",
	5,
	3,
	7
};

struct utk_menu menu = {
	&edit,
	"File",
	"New/Open.../Close/Exit/",
	5,
	4,
	8
};

struct utk_window test = {
	{
		{ NULL, { 2, 2, 10, 10 }, UTK_WINDOW, 8, 8 },
		NULL,
	},
	NULL,
	NULL,
	"Demo 1",
	NULL,
};

struct utk_window test2 = {
	{
		{ NULL, { 4, 8, 15, 25 }, UTK_WINDOW, 11, 17 },
		NULL,
	},
	NULL,
	NULL,
	"Demo 2",
	&menu
};


int main(int argc, char *argv[])
{
	struct utk_event *ev;

	screen.right = 79;
	screen.bottom = 24;
	ui_init();
	ui_redraw_all();
	ui_win_create(&test);
	ui_win_create(&test2);
	utk_refresh();
	while(1) {
		ev = utk_event();
		if (ev->type == EV_QUIT)
			break;
		if (ev->type == EV_CLOSE)
			ui_win_delete(ev->window);
	}
	ui_exit();
	return 0;

}
