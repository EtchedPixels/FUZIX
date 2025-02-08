#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>     /* defines: STDIN_FILENO, usleep */
#include <termios.h>    /* defines: termios, TCSANOW, ICANON, ECHO */
#include <termcap.h>
#include <string.h>

#include "draw.h"


#define CELL_WIDTH  11
#define CELL_HEIGHT 5
#define ROW_OFFSET  2

/* Termcap entry buffer */
static char tc_entry[1024];
/* Termcap data buffer (allocated) */
static char *tc_data = NULL;
/* Termcap entries - pointers within tc_data or NULL */
static char *tc_cl = NULL;  /* Clear screen and cursor home */
static char *tc_cm = NULL;  /* Cursor move to row %1, col %2 (on screen) */
static char *tc_ve = NULL;  /* Normal cursor visible */
static char *tc_vi = NULL;  /* Cursor invisible */


/******************************************************
 *                 Private functions                  *
 ******************************************************/

static int tputchar(int c);
static void print_cell(u16 nb, i8 r, i8 c);

static const char *fuzix_colours[NUM_COLOURS] = {
	"\033b\047",  /* reset (white) */
	"\033b\040",  /* black */
	"\033b\042",  /* red */
	"\033b\044",  /* green */
	"\033b\046",  /* yellow */
	"\033b\041",  /* blue */
	"\033b\043",  /* magenta */
	"\033b\045",  /* cyan */
	"\033b\047",  /* white */
};

static const char *ansi_colours[NUM_COLOURS] = {
	"\033[0m",   /* reset */
	"\033[90m",  /* black */
	"\033[91m",  /* red */
	"\033[92m",  /* green */
	"\033[93m",  /* yellow */
	"\033[94m",  /* blue */
	"\033[95m",  /* magenta */
	"\033[96m",  /* cyan */
	"\033[97m",  /* white */
};

static const char **colours = NULL;

static u8 get_colour(u16 nb) {

	switch (nb) {
	case    2: return COLOUR_WHITE;
	case    4: return COLOUR_YELLOW;
	case    8: return COLOUR_GREEN;
	case   16: return COLOUR_CYAN;
	case   32: return COLOUR_BLUE;
	case   64: return COLOUR_MAGENTA;
	case  128: return COLOUR_RED;
	case  256: return COLOUR_BLUE;
	case  512: return COLOUR_CYAN;
	case 1024: return COLOUR_GREEN;
	case 2048: return COLOUR_YELLOW;
	}

	return COLOUR_RESET;
}

static void set_text_colour(u8 c) {
	if (colours && c < NUM_COLOURS) {
		fputs(colours[c], stdout);
	}
}

static void move_cursor(int row, int col) {
	tputs(tgoto(tc_cm, col, row), 0, tputchar);
}

static void print_cell(u16 nb, i8 r, i8 c) {
	int nb_spaces_before = 0, nb_spaces_after = 0;

	if (nb == 0) {
		set_text_colour(COLOUR_RESET);
	} else {
		set_text_colour(get_colour(nb));
	}

	if (nb < 10)         { nb_spaces_before = 4; nb_spaces_after = 4; }
	else if (nb < 100)   { nb_spaces_before = 5; nb_spaces_after = 3; }
	else if (nb < 1000)  { nb_spaces_before = 5; nb_spaces_after = 3; }
	else if (nb < 10000) { nb_spaces_before = 6; nb_spaces_after = 2; }

	if (!opt_small) {
		/* normal display */

		/* top half */
		move_cursor(r * CELL_HEIGHT + ROW_OFFSET, c * CELL_WIDTH);
		printf("+--------+");
		move_cursor(r * CELL_HEIGHT + ROW_OFFSET + 1, c * CELL_WIDTH);
		printf("|        |");
		/* middle part */
		move_cursor(r * CELL_HEIGHT + ROW_OFFSET + 2, c * CELL_WIDTH);
		if (nb == 0) printf("|        |");
		else printf("|%*d%*s|", nb_spaces_before, nb, nb_spaces_after, "");
		/* bottom half */
		move_cursor(r * CELL_HEIGHT + ROW_OFFSET + 3, c * CELL_WIDTH);
		printf("|        |");
		move_cursor(r * CELL_HEIGHT + ROW_OFFSET + 4, c * CELL_WIDTH);
		printf("+--------+");
	} else {
		/* small display */

		nb_spaces_before -= 2;
		nb_spaces_after -= 2;
		move_cursor(r * 3 + 2, c * 7 + 2);
		printf("+----+");
		move_cursor(r * 3 + 3, c * 7 + 2);
		if (nb == 0) printf("|    |");
		else printf("|%*d%*s|", nb_spaces_before, nb, nb_spaces_after, "");
		move_cursor(r * 3 + 4, c * 7 + 2);
		printf("+----+");
	}

	set_text_colour(COLOUR_RESET);
}


/******************************************************
 *                 Public functions                   *
 ******************************************************/

void draw_setup(bool enable) {
	static bool enabled = false;
	static struct termios old;
	struct termios new;

	if (enable && !enabled) {
		char *term;
		char *dptr = tc_data;
		/* get termcap entry */
		term = getenv("TERM");
		if (!term || tgetent(tc_entry, term) != 1) {
			if (!term)
				term = "";
			fprintf(stderr, "%s: no termcap\n", term);
			exit(EXIT_FAILURE);
		}
		dptr = tc_data = malloc(strlen(tc_entry)+1);
		if (!opt_mono) {
			if (strcmp(term, "vt52") == 0) {
				colours = fuzix_colours;
			} else if (strncmp(term, "xterm", 5) == 0) {
				colours = ansi_colours;
			}
		}
		/* record important termcap information */
		tc_cl = tgetstr("cl", &dptr);
		tc_cm = tgetstr("cm", &dptr);
		tc_ve = tgetstr("ve", &dptr);
		tc_vi = tgetstr("vi", &dptr);

		/* get the terminal settings for standard input */
		tcgetattr(STDIN_FILENO, &new);
		/* we want to keep the old setting to restore them at the end */
		old = new;
		/* disable canonical mode (buffered i/o) and local echo */
		new.c_lflag &= (~ICANON & ~ECHO);
		new.c_cc[VMIN] = 1;
		/* set the new settings immediately */
		tcsetattr(STDIN_FILENO, TCSANOW, &new);
		/* invisible cursor */
		tputs(tc_vi, 0, tputchar);
		/* clear screen */
		draw_clear();
	} else if (!enable && enabled) {
		/* make cursor visible, reset all modes */
		set_text_colour(COLOUR_RESET);
		tputs(tc_ve, 0, tputchar);
		/* restore the former settings */
		tcsetattr(STDIN_FILENO, TCSANOW, &old);
		free(tc_data);
		tc_data = NULL;
	}
	enabled = enable;
}

/* Wrap putchar for use with tputs because putchar may be a macro */
static int tputchar(int c) {
	putchar(c);
	return 0;
}

void draw_clear(void) {
	tputs(tc_cl, 0, tputchar);
}

void print_score(u32 score) {
	static u32 last_score = 0;
	int swidth = opt_small ? 22 : 32;

	move_cursor(0, opt_small ? 0 : 1);

	set_text_colour(COLOUR_YELLOW);
	printf("Score");
	set_text_colour(COLOUR_RESET);

	if (score - last_score != 0) {
		int len = snprintf(NULL, 0, "  + %d", score - last_score);
		set_text_colour(COLOUR_GREEN);
		printf("  + %d", score - last_score);
		set_text_colour(COLOUR_RESET);
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
			print_cell(board[r][c], r, c);
		}
	}
}

void diff_board(u16 old[SIZE][SIZE], u16 new[SIZE][SIZE]) {
	i8 r, c;
	for (r = 0; r < SIZE; r++) {
		for (c = 0; c < SIZE; c++) {
			if (new[r][c] != old[r][c]) {
				print_cell(new[r][c], r, c);
			}
		}
	}
}

void print_indicators(void) {
	/* back */
	move_cursor(opt_small ? 15 : 23, 0);
	set_text_colour(COLOUR_CYAN);
	printf("B");
	set_text_colour(COLOUR_RESET);
	printf("ack  ");

	/* restart */
	set_text_colour(COLOUR_GREEN);
	printf("R");
	set_text_colour(COLOUR_RESET);
	printf("estart   ");

	if (!opt_small) {
		set_text_colour(COLOUR_MAGENTA);
		printf("WASD, HJKL, Cursors   ");
		set_text_colour(COLOUR_RESET);
	} else {
		printf("%11s", "");
	}

	/* quit */
	set_text_colour(COLOUR_RED);
	printf("Q");
	set_text_colour(COLOUR_RESET);
	printf("uit");
}

void print_win(void) {
	move_cursor(1, opt_small ? 11 : 17);
	set_text_colour(COLOUR_CYAN);
	printf("You win !");
	set_text_colour(COLOUR_RESET);
}

void print_game_over(void) {
	move_cursor(1, opt_small ? 11 : 17);
	set_text_colour(COLOUR_RED);
	printf("GAME OVER");
	set_text_colour(COLOUR_RESET);
}
