#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>     /* defines: STDIN_FILENO, usleep */
#include <termios.h>    /* defines: termios, TCSANOW, ICANON, ECHO */

#include "draw.h"


#define CELL_WIDTH  11
#define CELL_HEIGHT 5
#define ROW_OFFCET  2


/******************************************************
 *                 Private functions                  *
 ******************************************************/

static Color _get_color(u16 nb);
static void _print_cell(u16 nb, i8 r, i8 c);


static Color _get_color(u16 nb) {

	switch (nb) {
	case    2: return WHITE;
	case    4: return YELLOW;
	case    8: return GREEN;
	case   16: return CYAN;
	case   32: return BLUE;
	case   64: return MAGENTA;
	case  128: return RED;
	case  256: return BLUE;
	case  512: return CYAN;
	case 1024: return GREEN;
	case 2048: return YELLOW;
	}

	return WHITE;
}

static void _print_cell(u16 nb, i8 r, i8 c) {
	if (nb == 0) {
		SET_TEXT_COLOR(WHITE);
	} else {
		SET_TEXT_COLOR(_get_color(nb));
	}

	int nb_spaces_before = 0, nb_spaces_after = 0;
	if (nb < 10)         { nb_spaces_before = 4; nb_spaces_after = 4; }
	else if (nb < 100)   { nb_spaces_before = 5; nb_spaces_after = 3; }
	else if (nb < 1000)  { nb_spaces_before = 5; nb_spaces_after = 3; }
	else if (nb < 10000) { nb_spaces_before = 6; nb_spaces_after = 2; }

	if (!opt_small) {
		/* normal display */

		/* top half */
		MOVE_CURSOR(r * CELL_HEIGHT + ROW_OFFCET + 1, c * CELL_WIDTH + 1);
		printf("+--------+");
		MOVE_CURSOR(r * CELL_HEIGHT + ROW_OFFCET + 2, c * CELL_WIDTH + 1);
		printf("|        |");
		/* middle part */
		MOVE_CURSOR(r * CELL_HEIGHT + ROW_OFFCET + 3, c * CELL_WIDTH + 1);
		if (nb == 0) printf("|        |");
		else printf("|%*d%*s|", nb_spaces_before, nb, nb_spaces_after, "");
		/* bottom half */
		MOVE_CURSOR(r * CELL_HEIGHT + ROW_OFFCET + 4, c * CELL_WIDTH + 1);
		printf("|        |");
		MOVE_CURSOR(r * CELL_HEIGHT + ROW_OFFCET + 5, c * CELL_WIDTH + 1);
		printf("+--------+");
	} else {
		/* small display */

		nb_spaces_before -= 2;
		nb_spaces_after -= 2;
		MOVE_CURSOR(r * 3 + 2 + 1, c * 7 + 2 + 1);
		printf("+----+");
		MOVE_CURSOR(r * 3 + 2 + 2, c * 7 + 2 + 1);
		if (nb == 0) printf("|    |");
		else printf("|%*d%*s|", nb_spaces_before, nb, nb_spaces_after, "");
		MOVE_CURSOR(r * 3 + 2 + 3, c * 7 + 2 + 1);
		printf("+----+");
	}

	RESET_FORMATING;
}


/******************************************************
 *                 Public functions                   *
 ******************************************************/

void setBufferedInput(bool enable)
{
	static bool enabled = true;
	static struct termios old;
	struct termios new;

	if (enable && !enabled)
	{
		/* restore the former settings */
		tcsetattr(STDIN_FILENO, TCSANOW, &old);
		/* make cursor visible, reset all modes */
		printf("\033e");
		/* set the new state */
		enabled = true;
	}
	else if (!enable && enabled)
	{
		/* get the terminal settings for standard input */
		tcgetattr(STDIN_FILENO, &new);
		/* we want to keep the old setting to restore them at the end */
		old = new;
		/* disable canonical mode (buffered i/o) and local echo */
		new.c_lflag &= (~ICANON & ~ECHO);
		new.c_cc[VMIN] = 1;
		/* set the new settings immediately */
		tcsetattr(STDIN_FILENO, TCSANOW, &new);
		/* set the new state */
		enabled = false;
	}
}

void print_score(u32 score) {
	MOVE_CURSOR(1, opt_small ? 1 : 2);

	static u32 last_score = 0;
	int swidth = opt_small ? 22 : 32;

	SET_TEXT_COLOR(YELLOW); printf("Score"); RESET_FORMATING;

	if (score - last_score != 0) {
		SET_TEXT_COLOR(GREEN);
		int len = snprintf(NULL, 0, "  + %d", score - last_score);
		printf("  + %d", score - last_score);
		RESET_FORMATING;
		printf("%*d", swidth - len, score);
	} else {
		printf("%*d", swidth, score);
	}

	printf(" pts");

	last_score = score;
}

void print_board(u16 board[SIZE][SIZE]) {
	i8 r, c;
	for (r = 0; r < SIZE; r++) {
		for (c = 0; c < SIZE; c++) {
			_print_cell(board[r][c], r, c);
		}
	}
}

void diff_board(u16 old[SIZE][SIZE], u16 new[SIZE][SIZE]) {
	i8 r, c;
	for (r = 0; r < SIZE; r++) {
		for (c = 0; c < SIZE; c++) {
			if (new[r][c] != old[r][c]) {
				_print_cell(new[r][c], r, c);
			}
		}
	}
}

void print_indicators(void) {
	/* back */
	MOVE_CURSOR(opt_small ? 16 : 24, 1);
	SET_TEXT_COLOR(CYAN);
	printf("B");
	RESET_FORMATING;
	printf("ack  ");

	/* restart */
	SET_TEXT_COLOR(GREEN);
	printf("R");
	RESET_FORMATING;
	printf("estart   ");

	if (!opt_small) {
		SET_TEXT_COLOR(MAGENTA);
		printf("WASD, HJKL, Cursors   ");
		RESET_FORMATING;
	} else {
		printf("%11s", "");
	}

	/* quit */
	SET_TEXT_COLOR(RED);
	printf("Q");
	RESET_FORMATING;
	printf("uit");
}

void print_win(void) {
	MOVE_CURSOR(opt_small ? 2 : 1, opt_small ? 12 : 18);
	SET_TEXT_COLOR(CYAN);
	printf("You win !");
	RESET_FORMATING;
}

void print_game_over(void) {
	MOVE_CURSOR(opt_small ? 2 : 1, opt_small ? 12 : 18);
	SET_TEXT_COLOR(RED);
	printf("GAME OVER");
	RESET_FORMATING;
}
