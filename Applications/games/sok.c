/*
 *	A tiny Sokoban clone
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <termcap.h>
#include "sok.h"

static struct winsize win;
static int conf_fd = -1;
static int level_fd;

static struct level map;
static struct level base_map;

static uint16_t level;

#define MAX_UNDO	64
static uint8_t undolist[MAX_UNDO];
static uint8_t undop;
static uint16_t moves;

static uint8_t left;

#define BLOCK_MOVED	0x80

#define MAP_WALL	'#'
#define MAP_FLOOR	' '
#define MAP_BLOCK	'$'
#define MAP_BLOCK_ON	'*'
#define MAP_TARGET	'.'

/* HIJKL */
static int8_t delta[5][2] = { {0, -1}, {0, 0}, {1, 0}, {-1, 0}, {0, 1} };


static char *t_go;
static char *t_clreol;
static char *t_clreos;

/* str is either a space buffer for clearing or the output queue for
   termios, but never both at once */

#define MAXCOLS	132

static char str[MAXCOLS];
static char *ptr;

#ifdef __linux__
#include <stdio.h>
static char *_uitoa(int x)
{
	static char buf[10];
	sprintf(buf, "%d", x);
	return buf;
}
#endif

static int outbuf(int c)
{
	*ptr++ = c;
	return c;
}

static void tputs_buf(char *p, int n)
{
	ptr = str;
	tputs(p, n, outbuf);
	write(1, str, ptr - str);
}

static void moveto(uint8_t y, uint8_t x)
{
	tputs_buf(tgoto(t_go, x, y), 1);
}

static void draw(uint8_t y, uint8_t x, uint8_t c)
{
	moveto(2 + y, x + left);
	write(1, &c, 1);
}

static void clrtoeol(void)
{
	if (*t_clreol)
		tputs_buf(t_clreol, 1);
	else {
		memset(str, ' ', win.ws_col);
		write(1, str, win.ws_col);
	}
}

static void clear(void)
{
	if (*t_clreos) {
		moveto(0, 0);
		tputs_buf(t_clreos, win.ws_row);
	} else {
		int i = 0;
		while (i++ <= win.ws_row) {
			moveto(i, 0);
			clrtoeol();
		}
	}
}

static struct termios tcsave, tcnew;

void is_done(void)
{
	if (tcsetattr(0, TCSANOW, &tcsave))
		perror("tcsetattr:exit");
}

void quit_game(int sig)
{
	is_done();
	_exit(1);
}


static int ival[3];

static char *tnext(char *p)
{
	return p + strlen(p) + 1;
}

static void tty_init(void)
{
	int fd[2];
	pid_t pid;

	if (pipe(fd) < 0) {
		perror("pipe");
		exit(1);
	}

	pid = fork();
	if (pid == -1) {
		perror("fork");
		exit(1);
	}

	if (pid == 0) {
		close(fd[0]);
		dup2(fd[1], 1);
		execl("/usr/lib/tchelp", "tchelp", "li#co#cm$ce$cd$cl$",
		      NULL);
		_exit(1);
	}
	close(fd[1]);

	if (read(fd[0], ival,sizeof(int)) != sizeof(int)) {
		perror("tcread");
		exit(1);
	}
	if (ival[0] == 0)
		exit(1);

	if (read(fd[0], ival + 1 ,2 * sizeof(int)) != 2 * sizeof(int)) {
		perror("tcread");
		exit(1);
	}
	/* Don't need space for the two integer values reported */
	ival[0] -= 2 * sizeof(int);
	t_go = sbrk((ival[0] + 3) & ~3);
	if (t_go == (void *) -1) {
		perror("sbrk");
		exit(1);
	}
	if (read(fd[0], t_go, ival[0]) != ival[0]) {
		perror("tcread2");
		exit(1);
	}
	close(fd[0]);
	t_clreol = tnext(t_go);
	t_clreos = tnext(t_clreol);
	if (*t_clreos == 0)	/* No clr eos - try for clr/home */
		t_clreos++;	/* cl cap if present */

	if (!*t_go) {
		write(2, "sok: insufficient terminal control.\n", 36);
		exit(1);
	}

	if (tcgetattr(0, &tcsave) == 0) {
		atexit(is_done);
		memcpy(&tcnew, &tcsave, sizeof(struct termios));
		tcnew.c_cc[VMIN] = 1;
		tcnew.c_cc[VTIME] = 0;
		tcnew.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK);
		signal(SIGINT, quit_game);
		/* FIXME: handle stop/cont properly */
		signal(SIGTSTP, SIG_IGN);
		if (tcsetattr(0, TCSADRAIN, &tcnew))
			perror("tcsetattr");
	}
	if (ioctl(0, TIOCGWINSZ, &win)) {
		win.ws_col = 80;
		win.ws_row = 25;
	}
	if (win.ws_col > MAXCOLS)
		win.ws_col = MAXCOLS;
	left = (win.ws_col - MAP_W) / 2;
}

static void drawplayer(void)
{
	if (map.map[map.py][map.px] == MAP_TARGET)
		draw(map.py, map.px, '+');
	else
		draw(map.py, map.px, '@');
}

static void wipe_line(void)
{
	write(1, "                                ", 31);
}

static uint8_t getkey(void)
{
	uint8_t c;
	if (read(0, &c, 1) < 1)
		exit(1);
	return toupper(c);
}

static uint8_t fixup(uint8_t code)
{
	if (code == MAP_BLOCK)
		code = MAP_FLOOR;
	if (code == MAP_BLOCK_ON)
		code = MAP_TARGET;
	return code;
}

static void move(uint8_t dir)
{
	int8_t *dp;
	uint8_t ny, nx, n;

	dp = delta[dir];
	ny = map.py + *dp;
	nx = map.px + dp[1];

	n = map.map[ny][nx];

	if (n == MAP_WALL)
		return;

	if (n == MAP_BLOCK || n == MAP_BLOCK_ON) {
		uint8_t by = ny + *dp;
		uint8_t bx = nx + dp[1];
		uint8_t m = map.map[by][bx];
		if (m == MAP_WALL || m == MAP_BLOCK || m == MAP_BLOCK_ON)
			return;
		/* Adjust count of blocks positioned */
		if (map.map[by][bx] == MAP_TARGET) {
			draw(by, bx, MAP_BLOCK_ON);
			map.done++;
		} else
			draw(by, bx, MAP_BLOCK);

		/* Move on the map */
		map.map[by][bx] = MAP_BLOCK;
		map.map[ny][nx] = fixup(base_map.map[ny][nx]);
		if (map.map[ny][nx] == MAP_TARGET)
			map.done--;
		/* We don't need to redraw dy dx as we will put the player on it */
		dir |= BLOCK_MOVED;	/* For undo */
	}
	draw(map.py, map.px, map.map[map.py][map.px]);
	map.py += *dp;
	map.px += *++dp;
	drawplayer();
	moves++;
	if (undop == MAX_UNDO) {
		memmove(undolist, undolist + 1, MAX_UNDO - 1);
		undop--;
	}
	undolist[undop++] = dir;
}



static uint8_t reverse[] = "L KJH";

static void undo(void)
{
	uint8_t op;
	uint8_t dir;
	uint8_t c;
	int8_t *dp;
	uint8_t pry, prx;

	if (undop == 0)
		return;
	op = undolist[--undop];
	dir = op & 0x7F;
	/* As we are reversing a move we know we won't be pushing any other
	   block */
	if (op & BLOCK_MOVED) {
		/* We need to reverse a block */
		if (map.map[map.py][map.px] == MAP_TARGET)
			map.done++;
		map.map[map.py][map.px] = MAP_BLOCK;
		/* The block after the one in direction goes back to previous
		   (we don't allow a multi-block push) */
		dp = delta[dir];
		pry = map.py + *dp;
		prx = map.px + *++dp;
		c = fixup(base_map.map[pry][prx]);
		map.map[pry][prx] = c;
		draw(pry, prx, c);
		if (c == MAP_TARGET)
			map.done--;
	}
	move(reverse[dir] - 'H');
}

static uint8_t notifier(const char *p)
{
	uint8_t c;
	moveto(0, 0);
	write(1, "Do you want to ", 15);
	write(1, p, strlen(p));
	write(1, " (Y/N)", 6);
	c = getkey();
	moveto(0, 0);
	wipe_line();
	return c == 'Y';
}

static void redraw_all(void)
{
	uint8_t i;
	clear();
	for (i = 0; i < MAP_H; i++) {
		moveto(2 + i, left);
		write(1, map.map[i], MAP_W);
	}
	drawplayer();
}


static void start_level(void)
{
	moves = 0;
	undop = 0;
	memcpy(&map, &base_map.map, sizeof(map));
	redraw_all();
}

static void key(void)
{
	uint8_t c;
	moveto(0,0);
	switch (c = getkey()) {
	case 'Q':
		if (notifier("quit"))
			exit(0);
		break;
	case 'R' & 31:
		redraw_all();
		break;
	case 'R':
		if (notifier("restart"))
			start_level();
		break;
	case 'U':
		undo();
		break;
	case 'H':
	case 'J':
	case 'K':
	case 'L':
		move(c - 'H');
		break;
	}
	/* TODO arrow key handling */
}

static void play_level(void)
{
	start_level();
	while (map.done != map.blocks)
		key();
	moveto(0, 0);
	strcpy(str, "Level ");
	strcpy(str + 6, _uitoa(level));
	strcat(str + 6, " done in ");
	strcat(str + 6, _uitoa(moves));
	strcat(str + 6, " moves.");
	write(1, str, strlen(str));
	getkey();
}

/*
 *	There are various formats that mostly try and compress data
 *	down, notably the rather tight scheme used by Pusher, and the
 *	much more general approach used by XSB and the like. We naturally
 *	do our own thing for now and expect a preprocessed fixed size level
 *	on disk.
 */

static int load_level(uint16_t level)
{
	if (lseek(level_fd, level * MAP_SEEK, SEEK_SET) == -1 ||
	    read(level_fd, &base_map,
		 sizeof(struct level)) != sizeof(struct level))
		return 0;
	return 1;
}

static void play_game(void)
{
	while (load_level(level)) {
		play_level();
		level++;
		/* Save the new level achieved */
		if (conf_fd != -1) {
			lseek(conf_fd, 0L, SEEK_SET);
			write(conf_fd, &level, 2);
		}
	}
}

int main(int argc, char *argv[])
{
	uint16_t n;
	struct passwd *pw;
	const char *path = "/usr/lib/sok/sok.levels";

	if (argc > 2 && strcmp(argv[1], "-l") == 0) {
		level = atoi(argv[2]);
		argc -= 2;
		argv += 2;
	}
	if (argc == 2)
		path = argv[1];
	if (argc > 2) {
		write(2, "sok [-l level] [path to levels].\n", 21);
		exit(1);
	}
	
	level_fd = open(path, O_RDONLY);
	if (level_fd == -1) {
		perror(path);
		exit(1);
	}

	if (level == 0) {
		level = 1;
		pw = getpwuid(getuid());
		if (pw && chdir(pw->pw_dir) == 0) {
			conf_fd = open(".sokoban", O_RDWR | O_CREAT, 0600);
			if (conf_fd != -1) {
				if (read(conf_fd, &n, 2) == 2)
					level = n;
			}
		}
	}
	tty_init();
	/* Level '0' is actually not a level but a title screen */
	load_level(0);
	start_level();
	getkey();
	play_game();
}
