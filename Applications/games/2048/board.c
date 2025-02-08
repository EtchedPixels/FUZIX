#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "board.h"


/******************************************************
 *                 Private functions                  *
 ******************************************************/

static bool _slide_col(u16 board[SIZE][SIZE], i8 c, u32 *score);
/* rotate the board 90 degres clockwise */
static void _rotate_board(u16 board[SIZE][SIZE]);
static bool _empty_cells_left(u16 board[SIZE][SIZE]);
static bool _can_slide_up(u16 board[SIZE][SIZE]);


static bool _slide_col(u16 board[SIZE][SIZE], i8 c, u32 *score) {
	bool has_move = false;
	int last_cell = board[0][c] == 0 ? -1 : 0;
	i8 r;

	for (r = 1; r < SIZE; r++) {
		if (board[r][c] == 0) continue;

		if (last_cell == -1) {
			board[0][c] = board[r][c];
			board[r][c] = 0;
			last_cell = 0;
			has_move = true;
			continue;
		}

		if (board[last_cell][c] == 0) {
			board[last_cell][c] = board[r][c];
			board[r][c] = 0;
			has_move = true;
			continue;
		}

		if (board[last_cell][c] == board[r][c]) {
			board[last_cell][c] *= 2;
			*score += board[last_cell][c];
			board[r][c] = 0;
			has_move = true;
		}
		else if (last_cell + 1 != r) {
			board[last_cell + 1][c] = board[r][c];
			board[r][c] = 0;
			has_move = true;
		}

		last_cell++;
	}

	return has_move;
}

static void _rotate_board(u16 board[SIZE][SIZE]) {
	i8 r, c;
	/* transpose the board */
	for (r = 0; r < SIZE - 1; r++) {
		for (c = r + 1; c < SIZE; c++) {
			u16 tmp = board[r][c];
			board[r][c] = board[c][r];
			board[c][r] = tmp;
		}
	}

	/* vertical symmetry */
	for (r = 0; r < SIZE; r++) {
		for (c = 0; c < SIZE / 2; c++) {
			u16 tmp = board[r][c];
			board[r][c] = board[r][SIZE - 1 - c];
			board[r][SIZE - 1 - c] = tmp;
		}
	}
}

static bool _empty_cells_left(u16 board[SIZE][SIZE]) {
	i8 r, c;

	for (r = 0; r < SIZE; r++) {
		for (c = 0; c < SIZE; c++) {
			if (board[r][c] == 0)
				return true;
		}
	}

	return false;
}

static bool _can_slide_up(u16 board[SIZE][SIZE]) {
	i8 r, c;

	for (r = 1; r < SIZE; r++) {
		for (c = 0; c < SIZE; c++) {
			if (board[r][c] == board[r - 1][c])
				return true;
		}
	}

	return false;
}


/******************************************************
 *                 Public functions                   *
 ******************************************************/

void board_init(u16 board[SIZE][SIZE]) {
	i8 r, c;

	for (r = 0; r < SIZE; r++) {
		for (c = 0; c < SIZE; c++) {
			board[r][c] = 0;
		}
	}

	board_add_piece(board);
	board_add_piece(board);
}

void board_add_piece(u16 board[SIZE][SIZE]) {
	static int valid_cells[SIZE * SIZE][2];
	i8 r, c;
	unsigned i = 0;
	unsigned cell;

	for (r = 0; r < SIZE; r++) {
		for (c = 0; c < SIZE; c++) {
			if (board[r][c] != 0) continue;

			valid_cells[i][0] = r;
			valid_cells[i][1] = c;
			i++;
		}
	}

	cell = rand() % i;
	r = valid_cells[cell][0];
	c = valid_cells[cell][1];
	board[r][c] = (1 + (rand() % 10 == 0)) * 2;
}

bool board_can_move(u16 board[SIZE][SIZE]) {
	bool can_move;

	if (_empty_cells_left(board)) return true;
	if (_can_slide_up(board)) return true;

	can_move = false;

	_rotate_board(board);
	can_move = _can_slide_up(board);
	_rotate_board(board);
	_rotate_board(board);
	_rotate_board(board);

	return can_move;
}

bool board_win(u16 board[SIZE][SIZE]) {
	i8 r, c;
	for (r = 0; r < SIZE; r++) {
		for (c = 0; c < SIZE; c++) {
			if (board[r][c] == 2048) {
				return true;
			}
		}
	}

	return false;
}

bool board_move_up(u16 board[SIZE][SIZE], u32 *score) {
	bool has_move = false;
	i8 c;

	for (c = 0; c < SIZE; c++) {
		has_move |= _slide_col(board, c, score);
	}

	return has_move;
}

bool board_move_down(u16 board[SIZE][SIZE], u32 *score) {
	bool has_move;

	_rotate_board(board);
	_rotate_board(board);
	has_move = board_move_up(board, score);
	_rotate_board(board);
	_rotate_board(board);

	return has_move;
}

bool board_move_left(u16 board[SIZE][SIZE], u32 *score) {
	bool has_move;

	_rotate_board(board);
	has_move = board_move_up(board, score);
	_rotate_board(board);
	_rotate_board(board);
	_rotate_board(board);

	return has_move;
}

bool board_move_right(u16 board[SIZE][SIZE], u32 *score) {
	bool has_move;

	_rotate_board(board);
	_rotate_board(board);
	_rotate_board(board);
	has_move = board_move_up(board, score);
	_rotate_board(board);

	return has_move;
}
