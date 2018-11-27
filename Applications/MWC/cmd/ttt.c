/*
 * ttt -- 3-dimensional tic-tac-toe.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define	EMPTY	0
#define	MAN	1
#define	BEAST	5

/*
 * This is a table of all winning
 * combinations.
 * I stole it out of Kilobaud, April
 * 78.
 * You can look there to see how it
 * is ordered.
 */
const char w[] = {
	0, 1, 2, 3,
	4, 5, 6, 7,
	8, 9, 10, 11,
	12, 13, 14, 15,
	0, 4, 8, 12,
	1, 5, 9, 13,
	2, 6, 10, 14,
	3, 7, 11, 15,
	0, 5, 10, 15,
	3, 6, 9, 12,
	16, 17, 18, 19,
	20, 21, 22, 23,
	24, 25, 26, 27,
	28, 29, 30, 31,
	16, 20, 24, 28,
	17, 21, 25, 29,
	18, 22, 26, 30,
	19, 23, 27, 31,
	16, 21, 26, 31,
	19, 22, 25, 28,
	32, 33, 34, 35,
	36, 37, 38, 39,
	40, 41, 42, 43,
	44, 45, 46, 47,
	32, 36, 40, 44,
	33, 37, 41, 45,
	34, 38, 42, 46,
	35, 39, 43, 47,
	32, 37, 42, 47,
	35, 38, 41, 44,
	48, 49, 50, 51,
	52, 53, 54, 55,
	56, 57, 58, 59,
	60, 61, 62, 63,
	48, 52, 56, 60,
	49, 53, 57, 61,
	50, 54, 58, 62,
	51, 55, 59, 63,
	48, 53, 58, 63,
	51, 54, 57, 60,
	0, 16, 32, 48,
	1, 17, 33, 49,
	2, 18, 34, 50,
	3, 19, 35, 51,
	4, 20, 36, 52,
	5, 21, 37, 53,
	6, 22, 38, 54,
	7, 23, 39, 55,
	8, 24, 40, 56,
	9, 25, 41, 57,
	10, 26, 42, 58,
	11, 27, 43, 59,
	13, 29, 45, 61,
	12, 28, 44, 60,
	14, 30, 46, 62,
	15, 31, 47, 63,
	0, 21, 42, 63,
	4, 21, 38, 55,
	8, 25, 42, 59,
	12, 25, 38, 51,
	1, 21, 41, 61,
	13, 25, 37, 49,
	2, 22, 42, 62,
	14, 26, 38, 50,
	3, 22, 41, 60,
	7, 22, 37, 52,
	11, 26, 41, 56,
	15, 26, 37, 48,
	0, 20, 40, 60,
	0, 17, 34, 51,
	3, 18, 33, 48,
	3, 23, 43, 63,
	12, 24, 36, 48,
	12, 29, 46, 63,
	15, 30, 45, 60,
	15, 27, 39, 51
};

char b[64];
const char sep[] = "----------- ----------- ----------- -----------";


int yes(const char *s)
{
	char b[20];

	for (;;) {
		printf("%s? ", s);
		fflush(stdout);
		if (fgets(b, 20, stdin) == NULL)
			exit(0);
		if (b[0] == 'y')
			return (1);
		if (b[0] == 'n')
			return (0);
		printf("Answer `yes' or `no'.\n");
	}
}

void rules(void)
{
	printf("Three dimensional tic-tac-toe is played on a 4x4x4\n");
	printf("board. To win you must get 4 in a row.  Your moves\n");
	printf("are specified as a 3 digit number; the first digit\n");
	printf("is the level, the second the row and the third the\n");
	printf("column. Levels and columns  go  from left to right\n");
	printf("from 0 to 3. Rows go from top to bottom with  0 on\n");
	printf("the top.\n");
}

void psq(int s)
{
	int v;

	v = b[s];
	if (v == MAN)
		printf("UU");
	else if (v == BEAST)
		printf("CC");
	else
		printf("  ");
}

void board(void)
{
	int i, j;

	for (i = 0; i < 4; ++i) {
		if (i != 0)
			puts(sep);
		for (j = 0; j < 64; j += 4) {
			psq(i + j);
			if (j == 12 || j == 28 || j == 44)
				putchar(' ');
			else if (j >= 60)
				putchar('\n');
			else
				putchar('!');
		}
	}
}

void man(void)
{
	int i, j, t;
	char buf[20];

	board();
	for (;;) {
		if (gets(buf) == NULL)
			exit(0);
		i = 16 * (buf[0] - '0') + (buf[1] - '0') + 4 * (buf[2] -
								'0');
		if (i >= 0 && i <= 63 && b[i] == EMPTY)
			break;
		printf("?\n");
	}
	b[i] = MAN;
	for (i = 0; i < 4 * 76; i += 4) {
		t = 0;
		for (j = 0; j < 4; ++j)
			t += b[w[i + j]];
		if (t == 4 * MAN) {
			printf("You win.\n");
			exit(0);
		}
	}
}

int weight(int t)
{
	if (t == MAN)
		return (1);
	if (t == 2 * MAN)
		return (4);
	if (t == BEAST)
		return (1);
	if (t == 2 * BEAST)
		return (2);
	return (0);
}


void beast(void)
{
	int i, j, t;
	int s, bs, bt, v[76];

	for (i = 0; i < 4 * 76; i += 4) {
		t = 0;
		for (j = 0; j < 4; ++j)
			t += b[w[i + j]];
		v[i >> 2] = t;
		if (t == 3 * BEAST)
			break;
	}
	if (i < 4 * 76) {
		for (j = 0; j < 4; ++j)
			if (b[w[i + j]] == EMPTY) {
				b[w[i + j]] = BEAST;
				break;
			}
		board();
		printf("I win.\n");
		exit(0);
	}
	bt = 0;
	for (s = 0; s < 64; ++s) {
		if (b[s] != EMPTY)
			continue;
		t = 0;
		for (i = 0; i < 4 * 76; i += 4) {
			for (j = 0; j < 4; ++j)
				if (w[i + j] == s)
					break;
			if (j != 4) {
				if (v[i >> 2] == 3 * MAN) {
					b[s] = BEAST;
					return;
				}
				t += weight(v[i >> 2]);
			}
		}
		if (t > bt) {
			bt = t;
			bs = s;
		}
	}
	if (bt != 0)
		b[bs] = BEAST;
	else {
		for (s = 0; s < 64; ++s)
			if (b[s] == EMPTY)
				break;
		if (s == 64) {
			printf("Draw.\n");
			exit(0);
		}
		b[s] = BEAST;
	}
}

int main(int argc, char *argv[])
{
	if (yes("Print rules"))
		rules();
	if (yes("Play first"))
		man();
	for (;;) {
		beast();
		man();
	}
}
