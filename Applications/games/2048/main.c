#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>     /* defines: STDIN_FILENO, usleep */
#include <time.h>

#include "common.h"
#include "board.h"
#include "draw.h"


bool opt_mono = false;
bool opt_small = false;


void signal_callback_handler(int s) {
	draw_setup(false);
	exit(EXIT_SUCCESS);
}

void setup(void) {
	/* initialise display */
	draw_setup(true);
	/* init rng */
	srand(time(NULL));
	/* register signal handler for when ctrl-c is pressed */
	signal(SIGINT, signal_callback_handler);
}

static void helptext(const char *arg0) {
	printf("usage: %s [-msh]\n", arg0);
	puts(
"Terminal version of 2048\n"
"Copyright (c) 2023 Frost-Phoenix\n"
"Fuzix adaptation and enhancements by Ciaran Anscomb\n"
"Original 2048 web game by Gabriele Cirulli\n"
"\n"
"   -m   monochrome mode (no Fuzix colour codes)\n"
"   -s   small tiles (fit within 32x16 display)\n"
"   -h   display this help and exit\n"
	);
}

int main(int argc, char **argv) {
	int opt;
	int c;
	u32 score = 0;
	u32 last_score = 0;
	bool run = true;
	static u16 board[SIZE][SIZE];
	static u16 tmp_board[SIZE][SIZE];
	static u16 last_board[SIZE][SIZE];

	while ((opt = getopt(argc, argv, "msh")) != -1) {
		switch (opt) {
		case 'm':
			opt_mono = true;
			break;
		case 's':
			opt_small = true;
			break;
		case 'h':
			helptext(argv[0]);
			exit(EXIT_SUCCESS);
		default:
			printf("Try '%s -h' for more information.\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	setup();

	board_init(board);
	memcpy(last_board, board, sizeof(board));

	print_score(score);
	print_board(board);
	print_indicators();

	while (run) {
		bool has_move = false;

		fflush(stdout);
		c = getchar();

		if (c == -1) {
			puts("\nError! Cannot read keyboard input!");
			break;
		} else if (c == 27 || c == 91) {
			continue;
		} else if (c == 'q' || c == 'Q') {
			break;
		} else if (c == 'r' || c == 'R') {
			score = last_score = 0;
			board_init(board);
			memcpy(last_board, board, sizeof(board));
			print_score(score);
			print_board(board);
			print_indicators();
			continue;
		} else if (c == 'b') {
			/* undo last move */
			memcpy(board, last_board, sizeof(board));
			score = last_score;
			print_score(score);
			print_board(board);
			print_indicators();
			continue;
		} else if (c == 12) {
			draw_clear();
			print_score(score);
			print_board(board);
			print_indicators();
			continue;
		}

		/* save current board tp a tmp buffer */
		memcpy(tmp_board, board, sizeof(board));
		last_score = score;

		switch (c) {
		case 'k': case 'w': case '^':
		case 65:  /* up arrow */
			has_move = board_move_up(board, &score);
			break;
		case 'j': case 's': case '|':
		case 66:  /* down arrow */
			has_move = board_move_down(board, &score);
			break;
		case 'l': case 'd': case 9:
		case 67:  /* right arrow */
			has_move = board_move_right(board, &score);
			break;
		case 'h': case 'a': case 8:
		case 68:  /* left arrow */
			has_move = board_move_left(board, &score);
			break;
		}

		if (has_move) {
			/* save the current board */
			memcpy(last_board, tmp_board, sizeof(board));

			board_add_piece(board);

			print_score(score);
			diff_board(tmp_board, board);
			print_indicators();

			if (!board_can_move(board)) {
				print_game_over();
				print_board(board);
				print_indicators();

				run = false;
				break;
			} else if (board_win(board)) {
				print_win();
				print_board(board);
				print_indicators();

				run = false;
				break;
			}
		}
	}

	/* restore terminal settings */
	draw_setup(false);
	putchar('\n');

	return EXIT_SUCCESS;
}
