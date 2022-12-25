#include <stdio.h>
#include "utk.h"

struct utk_window test = {
	NULL,
	NULL,
	{ 2, 2, 10, 10 },
	"Demo",
	NULL,
	NULL
};

struct utk_window test2 = {
	NULL,
	NULL,
	{ 5, 5, 20, 12 },
	"Demo2 ",
	NULL,
	NULL
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
	return 0;

}
