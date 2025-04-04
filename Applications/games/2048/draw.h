#ifndef DRAW_H
#define DRAW_H

#include "common.h"

enum {
	COLOUR_RESET,
	COLOUR_BLACK,
	COLOUR_RED,
	COLOUR_GREEN,
	COLOUR_YELLOW,
	COLOUR_BLUE,
	COLOUR_MAGENTA,
	COLOUR_CYAN,
	COLOUR_WHITE,
	NUM_COLOURS
};

void draw_setup(bool enable);

void draw_clear(void);
void draw_set_text_color(u8 c);
void draw_move_cursor(int col, int row);

void print_score(u32 score);
void print_board(u16 board[SIZE][SIZE]);
void diff_board(u16 old[SIZE][SIZE], u16 new[SIZE][SIZE]);
void print_indicators(void);
void print_win(void);
void print_game_over(void);

#endif
