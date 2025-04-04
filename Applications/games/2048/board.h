#ifndef BOARD_H
#define BOARD_H

#include "common.h"

void board_init(u16 board[SIZE][SIZE]);

void board_add_piece(u16 board[SIZE][SIZE]);
bool board_can_move(u16 board[SIZE][SIZE]);
bool board_win(u16 board[SIZE][SIZE]);

bool board_move_up(u16 board[SIZE][SIZE], u32 *score);
bool board_move_down(u16 board[SIZE][SIZE], u32 *score);
bool board_move_left(u16 board[SIZE][SIZE], u32 *score);
bool board_move_right(u16 board[SIZE][SIZE], u32 *score);

#endif
