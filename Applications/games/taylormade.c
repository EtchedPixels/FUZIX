#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <termcap.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>

#include "taylormade.h"

/* Machines we can handle the full largkve text on. We can probably fix this
   on the others by paging the long texts from disk, especially as they
   are uncompressed format */
#if defined(__linux__) || defined(__mc68000__) || defined (__ns32k__) || defined(__ARM_EABI__) || defined(__riscv) || defined(__i86)
#define LOAD_ALL
#define LOAD_SIZE	0x4D00
#endif

/* Work relative to end of video */
#define SCANNER_BIAS	0x5B00
#define DISK_BIAS	27L

#ifndef LOAD_SIZE
#define LOAD_SIZE	0x4D00
#endif

/*
 *	Everything is based upon the actual game sizes
 *
 *	Version 2 games: (all dual message table)
 *	He Man:
 *	91 Objects, 40CF working game size
 *	Kayleth:
 *	123 Objects, 45A0 working game size
 *	Temple of Terror:
 *	191 Objects, 4249 working game size
 *
 *	Version 1 games:
 *	Blizzard Pass:
 *	150 Objects, 40DA working game size
 *
 *	Version 0 games:(not currently loading correctly)
 *	Rebel Planet: (dual message table)
 *	133 Objects, 482E working size
 *
 *	Not yet supported: QuestProbe games.
 */

#define MAXOBJ	200

static uint8_t Flag[128];
static uint8_t InitFlag[6];
static uint8_t Object[MAXOBJ];
static uint8_t Word[5];

/* OOPS buffer for later games */
static uint8_t OopsFlag[128];
static uint8_t OopsObject[MAXOBJ];
/* RAM save buffer for later games */
static uint8_t RamFlag[128];
static uint8_t RamObject[MAXOBJ];

/* Except for Blizzard Pass long text mode we are good with 4900 */
#ifdef LOAD_ALL
static uint8_t Image[0x4000 + LOAD_SIZE];
#else
static uint8_t Image[LOAD_SIZE];
#endif
static int ImageLen;
static int VerbBase;
static unsigned int TokenBase;
static unsigned int MessageBase;
static unsigned int Message2Base;
static unsigned int RoomBase;
static unsigned int ObjectBase;
static unsigned int ExitBase;
static unsigned int ObjLocBase;
static unsigned int StatusBase;
static unsigned int ActionBase;
static unsigned int FlagBase;

static int NumLowObjs;

static int ActionsDone;
static int ActionsExecuted;
static int Redraw;

static uint_fast8_t GameVersion;
static uint_fast8_t NoCarryLimit;
static uint_fast8_t GameNum;

/*
 *	Whilst the game engine for the Scott Adams games was very consistent
 *	the later games have per game engine changes to meet the game needs.
 *
 *	Temple of Terror also comes with bugs that need to be handled to make
 *	it properly playable.
 */
#define G_UNKNOWN	0
#define G_REBEL		1	/* Wait timers */
#define G_KAYLETH	2	/* Wait timers */
#define G_BLIZZARD	3	/* 128K, no carry limit */
#define G_TEMPLE	4	/* Work around bug in actual game */
#define G_HEMAN		5

static int GameFile;

void writes(const char *p)
{
	write(2, p, strlen(p));
}

/* A mini look alike to David Given's libcuss. If useful will probably
   become a library. For now pasted around to experiment */

uint_fast8_t screenx, screeny, screen_height, screen_width;
static char *t_go, *t_clreol, *t_clreos;
static uint8_t conbuf[64];
static uint8_t *conp = conbuf;

extern void con_puts(const char *s);

/* Queue a character to the output buffer */
static int conq(int c)
{
	if (conp == conbuf + sizeof(conbuf)) {
		write(1, conbuf, sizeof(conbuf));
		conp = conbuf;
	}
	*conp++ = (uint8_t) c;
	return 0;
}

/* Make sure the output buffer is written */
void con_flush(void)
{
	write(1, conbuf, conp - conbuf);
	conp = conbuf;
}

static const char hex[] = "0123456789ABCDEF";

/* Put a character to the screen. We handle unprintables and tabs */
void con_putc(uint8_t c)
{
	if (c == '\t') {
		uint8_t n = 8 - (screenx & 7);
		while (n--)
			con_putc(' ');
		return;
	}
	if (c > 127) {
		con_puts("\\x");
		con_putc(hex[c >> 4]);
		con_putc(hex[c & 0x0F]);
		return;
	} else if (c == 127) {
		con_puts("^?");
		return;
	}
	if (c < 32) {
		con_putc('^');
		c += '@';
	}
	conq(c);
	screenx++;
	if (screenx == screen_width) {
		screenx = 0;
		screeny++;
	}
}

/* Write a termcap string out */
static void con_twrite(char *p, int n)
{
#if !defined(__linux__)
	tputs(p, n, conq);
#else
	while (*p)
		conq(*p++);
#endif
}

/* Write a string of symbols including quoting */
void con_puts(const char *s)
{
	uint8_t c;
	while ((c = (uint8_t) * s++) != 0)
		con_putc(c);
}

/* Add a newline */
void con_newline(void)
{
	if (screeny >= screen_height)
		return;
	conq('\n');
	screenx = 0;
	screeny++;
}

/* We need to optimize this but firstly we need to fix our
   tracking logic as we use con_goto internally but don't track
   that verus the true user values */
void con_force_goto(uint_fast8_t y, uint_fast8_t x)
{
	con_twrite(tgoto(t_go, x, y), 2);
	screenx = x;
	screeny = y;
}

void con_goto(uint_fast8_t y, uint_fast8_t x)
{
#if 0
	if (screenx == x && screeny == y)
		return;
	if (screeny == y && x == 0) {
		conq('\r');
		screenx = 0;
		return;
	}
	if (screeny == y - 1 && x == 0) {
		con_newline();
		return;
	}
#endif
	con_force_goto(y, x);
}

/* Clear to end of line */
void con_clear_to_eol(void)
{
	if (screenx == screen_width - 1)
		return;
	if (t_clreol)
		con_twrite(t_clreol, 1);
	else {
		uint_fast8_t i;
		/* Write spaces. This tends to put the cursor where
		   we want it next time too. Might be worth optimizing ? */
		for (i = screenx; i < screen_width; i++)
			con_putc(' ');
	}
}

/* Clear to the bottom of the display */

void con_clear_to_bottom(void)
{
	/* Most terminals have a clear to end of screen */
	if (t_clreos)
		con_twrite(t_clreos, screen_height);
	/* If not then clear each line, which may in turn emit
	   a lot of spaces in desperation */
	else {
		uint_fast8_t i;
		for (i = 0; i < screen_height; i++) {
			con_goto(i, 0);
			con_clear_to_eol();
		}
	}
	con_force_goto(0, 0);
}

void con_clear(void)
{
	con_goto(0, 0);
	con_clear_to_bottom();
}

int con_scroll(int n)
{
	if (n == 0)
		return 0;
	/* For now we don't do backscrolls: FIXME */
	if (n < 0)
		return 1;
	/* Scrolling down we can do */
	con_force_goto(screen_height - 1, 0);
	while (n--)
		conq('\n');
	con_force_goto(screeny, screenx);
	return 0;
}

/* TODO: cursor key handling */
int con_getch(void)
{
	uint8_t c;
	con_flush();
	if (read(0, &c, 1) != 1)
		return -1;
	return c;
}

int con_size(uint8_t c)
{
	if (c == '\t')
		return 8 - (screenx & 7);
	/* We will leave unicode out 8) */
	if (c > 127)
		return 4;
	if (c < 32 || c == 127)
		return 2;
	return 1;
}

static int do_read(int fd, void *p, int len)
{
	int l;
	if ((l = read(fd, p, len)) != len) {
		if (l < 0)
			perror("read");
		else
			write(2, "short read from tchelp.\n", 25);
		return -1;
	}
	return 0;
}

static char *tnext(char *p)
{
	return p + strlen(p) + 1;
}


int tty_init(void)
{
	int fd[2];
	pid_t pid;
	int ival[3];
	int status;

	if (pipe(fd) < 0) {
		perror("pipe");
		return -1;
	}

	pid = fork();
	if (pid == -1) {
		perror("fork");
		return -1;
	}

	if (pid == 0) {
		close(fd[0]);
		dup2(fd[1], 1);
		execl("/usr/lib/tchelp", "tchelp", "li#co#cm$ce$cd$cl$",
		      NULL);
		perror("tchelp");
		_exit(1);
	}
	close(fd[1]);
	waitpid(pid, &status, 0);

	do_read(fd[0], ival, sizeof(int));
	if (ival[0] == 0)
		return -1;
	do_read(fd[0], ival + 1, 2 * sizeof(int));

	ival[0] -= 2 * sizeof(int);
	t_go = sbrk((ival[0] + 3) & ~3);

	if (t_go == (void *) -1) {
		perror("sbrk");
		return -1;
	}

	if (do_read(fd[0], t_go, ival[0]))
		return -1;

	close(fd[0]);
	t_clreol = tnext(t_go);
	t_clreos = tnext(t_clreol);
	if (*t_clreos == 0)	/* No clr eos - try for clr/home */
		t_clreos++;	/* cl cap if present */
	if (*t_go == 0) {
		write(2, "Insufficient terminal features.\n", 32);
		return -1;
	}
	/* TODO - screen sizes */
	screen_height = ival[1];
	screen_width = ival[2];
	/* need to try WINSZ and VT ioctls */
	return 0;
}

static struct termios con_termios, old_termios;

void con_exit(void)
{
	tcsetattr(0, TCSANOW, &old_termios);
}

int con_init(void)
{
	int n;
	static struct winsize w;
	if (tty_init())
		return -1;
	if (tcgetattr(0, &con_termios) == -1)
		return -1;
	memcpy(&old_termios, &con_termios, sizeof(struct termios));
	atexit(con_exit);
	con_termios.c_lflag &= ~(ICANON | ECHO | ISIG);
	con_termios.c_iflag &= ~(IXON);
	con_termios.c_cc[VMIN] = 1;
	con_termios.c_cc[VTIME] = 0;
	if (tcsetattr(0, TCSANOW, &con_termios) == -1)
		return -1;
#ifdef VTSIZE
	n = ioctl(0, VTSIZE, 0);
	if (n != -1) {
		screen_width = n & 0xFF;
		screen_height = (n >> 8) & 0xFF;
	}
#endif
	if (ioctl(0, TIOCGWINSZ, &w) == 0) {
		screen_width = w.ws_col;
		screen_height = w.ws_row;
	}
	return 0;
}

static char wbuf[81];
static int wbp = 0;
static int xpos = 0;
static int upper;
static uint8_t ly, lx;

static char ubuf[1024];
static char *uptr = ubuf;

static void add_upper(char c)
{
	if (uptr < ubuf + 1024)
		*uptr++ = c;
}

static void add_upper_string(char *p)
{
	while (*p)
		add_upper(*p++);
}

static void RefreshUpper(void)
{
	char *p = ubuf;
	ly = screeny;
	lx = screenx;

	con_goto(0, 0);
	while (p < uptr) {
		if (*p == '\n') {
			con_clear_to_eol();
			con_newline();
		} else
			con_putc(*p);
		p++;
	}
	con_goto(ly, lx);
	xpos = lx;
}

void DisplayInit(void)
{
	if (con_init())
		exit(1);
	con_clear();
	con_goto(screen_height - 1, 0);
}

void DisplayEnd(void)
{
	con_newline();
	con_flush();
}

static void flush_word(void)
{
	wbuf[wbp] = 0;
//	fprintf(stderr, "FW U%d X%d W%d %s\n", upper, xpos, wbp, wbuf);
	if (upper)
		add_upper_string(wbuf);
	else
		con_puts(wbuf);
	xpos += wbp;
	wbp = 0;
}

static void char_out(char c)
{
	if (c != ' ' && c != '\n') {
		if (wbp < 80)
			wbuf[wbp++] = c;
		return;
	}
	if (xpos + wbp >= screen_width) {
		xpos = 0;
		if (upper) {
			add_upper('\n');
		} else {
			con_scroll(1);
			con_goto(screen_height - 1, 0);
		}
	}
	if (c == '\n') {
		flush_word();
		if (upper) {
			add_upper('\n');
		} else {
			con_scroll(1);
			con_goto(screen_height - 1, 0);
		}
		xpos = 0;
		return;
	}
	flush_word();
	if (upper)
		add_upper(' ');
	else
		con_putc(' ');
	xpos++;
}

void LineInput(char *linebuf, int len)
{
	int c;
	char *p = linebuf;

	RefreshUpper();

	do {
		c = con_getch();
		if (c == 8 || c == 127) {
			if (p > linebuf) {
				con_goto(screen_height - 1, screenx - 1);
				con_putc(' ');
				con_goto(screen_height - 1, screenx - 1);
				p--;
			}
			continue;
		}
		if (c > 31 && c < 127) {
			if (p < linebuf + len && xpos < screen_width - 1) {
				*p++ = c;
				con_putc(c);
			}
			continue;
		}
	}
	while (c != 13 && c != 10);
	*p = 0;
	con_scroll(1);
	con_goto(screen_height - 1, 0);
}

uint_fast8_t WaitCharacter(void)
{
	RefreshUpper();
	return con_getch();
}

void PrintCharacter(uint_fast8_t c)
{
	char_out(c);
}

void TopWindow(void)
{
	if (upper == 1)
		return;

	flush_word();
	ly = screeny;
	lx = screenx;
	upper = 1;
	uptr = ubuf;
	xpos = 0;
}

void BottomWindow(void)
{
	unsigned i;

	if (upper == 0)
		return;

	char_out('\n');
	for (i = 0; i < screen_width; i++)
		add_upper('=');

	upper = 0;
	con_goto(ly, lx);
	xpos = lx;
}

static char LastChar = 0;
static uint_fast8_t Upper = 0;
static uint_fast8_t PendSpace = 0;

static void OutWrite(char c)
{
	if (isalpha(c) && Upper) {
		c = toupper(c);
		Upper = 0;
	}
	PrintCharacter(c);
}

static void OutFlush(void)
{
	if (LastChar)
		OutWrite(LastChar);
	if (PendSpace)
		OutWrite(' ');
	PendSpace = 0;
	LastChar = 0;
}

static void OutReset(void)
{
	OutFlush();
}

static void OutCaps(void)
{
	if (LastChar) {
		OutWrite(LastChar);
		LastChar = 0;
	}
	Upper = 1;
}

static void OutChar(char c)
{
	if (c == ']')
		c = '\n';
	if (c == ' ') {
		PendSpace = 1;
		return;
	}
	if (LastChar) {
		OutWrite(LastChar);
		LastChar = 0;
	}
	if (PendSpace) {
		OutWrite(' ');
		PendSpace = 0;
	}
	if (c == '.')
		Upper = 1;
	LastChar = c;
}

static void OutReplace(char c)
{
	LastChar = c;
}

static void OutKillSpace(void)
{
	PendSpace = 0;
}

static void OutString(char *p)
{
	while (*p)
		OutChar(*p++);
}

#define DESTROYED	252
#define CARRIED		(Flag[2])
#define WORN		(Flag[3])
#define LOCATION	(Flag[0])
#define NUMOBJECTS	(Flag[6])

static unsigned char *TokenText(unsigned char n)
{
	unsigned char *p = Image + TokenBase;

	p = Image + TokenBase;

	while (n > 0) {
		while ((*p & 0x80) == 0)
			p++;
		n--;
		p++;
	}
	return p;
}

static void PrintToken(unsigned char n)
{
	unsigned char *p = TokenText(n);
	unsigned char c;
	do {
		c = *p++;
		OutChar(c & 0x7F);
	} while (!(c & 0x80));
}

static void PrintText1(unsigned char *p, int n)
{
	while (n > 0) {
		while (*p != 0x7E && *p != 0x5E)
			p++;
		n--;
		p++;
	}
	while (*p != 0x7E && *p != 0x5E)
		PrintToken(*p++);
	if (*p == 0x5E)
		PendSpace = 1;
}

/*
 *	Version 0 is different
 */

static void PrintText0(unsigned char *p, int n)
{
	unsigned char *t = NULL;
	unsigned char c;
	while (1) {
		if (t == NULL)
			t = TokenText(*p++);
		c = *t & 0x7F;
		if (c == 0x5E || c == 0x7E) {
			if (n == 0) {
				if (c == 0x5E)
					PendSpace = 1;
				return;
			}
			n--;
		} else if (n == 0)
			OutChar(c);
		if (*t++ & 0x80)
			t = NULL;
	}
}

static void PrintText(unsigned char *p, int n)
{
	if (GameVersion == 0)	/* In stream end markers */
		PrintText0(p, n);
	else			/* Out of stream end markers (faster) */
		PrintText1(p, n);
}

static void Message(unsigned char m)
{
	PrintText(Image + MessageBase, m);
}

static void Message2(unsigned int m)
{
	PrintText(Image + Message2Base, m);
}

static void PrintObject(unsigned char obj)
{
	unsigned char *p = Image + ObjectBase;
	PrintText(p, obj);
}

static void PrintRoom(unsigned char room)
{
	unsigned char *p = Image + RoomBase;
	/* Rebel Planet inherited the old game engine behaviour of putting the
	   "You are" in automatically. This made no sense with the tokenisation
	   text compression so later games dropped it */
	if (GameNum == G_REBEL && LOCATION)
		OutString("You are ");
#ifdef LOAD_ALL
	if (GameNum == G_BLIZZARD && room < 102)
		PrintText(Image + LOAD_SIZE, room);
	else
#endif
		PrintText(p, room);
}

/* Avoid sucking in stdio */

char *u8toa(uint8_t i)
{
	static char buf[4];
	char *p = buf + sizeof(buf);
	uint8_t c;

	*--p = '\0';
	do {
		c = i % 10;
		i /= 10;
		*--p = '0' + c;
	} while(i);
	return p;
}

static void PrintNumber(unsigned char n)
{
	char *p = u8toa(n);
	while (*p)
		OutChar(*p++);
}


static int CarryItem(void)
{
	if (NoCarryLimit)
		return 1;
	if (Flag[5] == Flag[4])
		return 0;
	return 1;
}

static void Put(unsigned char obj, unsigned char loc)
{
	int mod = 0;
	/* Will need refresh logics somewhere, maybe here ? */
	if (Object[obj] == LOCATION || loc == LOCATION)
		Redraw = 1;
	if (loc == CARRIED)
		mod++;
	if (Object[obj] == CARRIED)
		mod--;
	Object[obj] = loc;
	if (Flag[5] == 0 && mod == -1)
		return;
	if (Flag[5] == 255 && mod == 1)
		return;
	Flag[5] += mod;
}

static int Present(unsigned char obj)
{
	unsigned char v = Object[obj];
	if (v == LOCATION || v == WORN || v == CARRIED)
		return 1;
	return 0;
}

static int Chance(int n)
{
	unsigned int v = (rand() >> 12) ^ time(NULL);
	v %= 100;
	if (v > n)
		return 0;
	return 1;
}

static void NewGame(void)
{
	/* Reload key info from disk */
	Redraw = 1;
	memset(Flag, 0, 128);
	memcpy(Flag + 1, InitFlag, 6);
	lseek(GameFile, ObjLocBase, SEEK_SET);
	read(GameFile, Object, NUMOBJECTS);
}

void RamLoad(void)
{
	memcpy(Flag, RamFlag, 128);
	memcpy(Object, RamObject, MAXOBJ);
	Message(19);
}

static void RamSave(int game)
{
	memcpy(RamFlag, Flag, 128);
	memcpy(RamObject, Object, MAXOBJ);
	if (game)
		Message(19);
}

static void Oops(void)
{
	memcpy(Flag, OopsFlag, 128);
	memcpy(Object, OopsObject, MAXOBJ);
}

static void Checkpoint(void)
{
	memcpy(OopsFlag, Flag, 128);
	memcpy(OopsObject, Object, MAXOBJ);
}

static void LoadGame(void)
{
	char c;
	char name[32];
	int fd;
	OutCaps();
	Message(26);
	OutFlush();

	do {
		c = WaitCharacter();
		if (c == 'n' || c == 'N') {
			OutChar('N');
			OutChar('\n');
			return;
		}
		if (c == 'y' || c == 'Y') {
			OutChar('Y');
			OutChar('\n');

			OutString("File name: ");
			LineInput(name, 32);

			fd = open(name, O_RDONLY);
			if (fd == -1)
				OutString("Unable to open file.\n");
			else if (read(fd, Flag, 128) != 128 ||
				 read(fd, Object, MAXOBJ) != MAXOBJ) {
				OutString("Unable to load game.\n");
				NewGame();
			}
			close(fd);
			Redraw = 1;
			return;
		}
	}
	while (1);
}

static void QuitGame(void)
{
	char c;
	OutCaps();
	Message(18);
	OutChar(' ');
	OutFlush();
	do {
		c = WaitCharacter();
		if (c == 'n' || c == 'N') {
			OutChar('N');
			OutChar('\n');
			OutFlush();
			DisplayEnd();
			exit(0);
		}
		if (c == 'y' || c == 'Y') {
			OutChar('Y');
			OutChar('\n');
			NewGame();
			return;
		}
	}
	while (1);
}

static void Inventory(void)
{
	int i;
	int f = 0;
	OutCaps();
	Message(16);		/* ".. are carrying: " */
	for (i = 0; i < NUMOBJECTS; i++) {
		if (Object[i] == CARRIED || Object[i] == WORN) {
			f = 1;
			PrintObject(i);
			if (Object[i] == WORN)
				Message(30);
		}
	}
	if (f == 0)
		Message(17);	/* "nothing at all" */
	else {
		if (GameVersion == 0) {
			OutKillSpace();
			OutChar('.');
		} else {
			OutReplace('.');
		}
	}
}

static void AnyKey(void)
{
	Message(20);
	OutFlush();
	WaitCharacter();
}

static void SaveGame(void)
{
	int fd;
	char name[33];
	OutString("File name: ");
	LineInput(name, 32);
	fd = open(name, O_WRONLY | O_TRUNC | O_CREAT, 0600);
	if (fd == -1) {
		OutString("Save failed.\n");
		return;
	}
	if (write(fd, Flag, 128) != 128
	    || write(fd, Object, MAXOBJ) != MAXOBJ)
		OutString("Save failed.\n");
	close(fd);
}

static void DropAll(void)
{
	int i;
	for (i = 0; i < NUMOBJECTS; i++) {
		if (Object[i] == CARRIED || Object[i] == WORN)
			Put(i, LOCATION);
	}
	Flag[5] = 0;
}

static void GetObject(unsigned char obj)
{
	if (Object[obj] == CARRIED || Object[obj] == WORN) {
		Message(21);
		return;
	}
	if (Object[obj] != LOCATION) {
		Message(22);
		return;
	}
	if (CarryItem() == 0) {
		Message(15);
		return;
	}
	Put(obj, CARRIED);
}

static int DropObject(unsigned char obj)
{
	/* FIXME: check if this is how the real game behaves */
	if (Object[obj] == WORN) {
		Message(29);
		return 0;
	}
	if (Object[obj] != CARRIED) {
		Message(23);
		return 0;
	}
	Put(obj, LOCATION);
	return 1;
}

static void Look(void)
{
	int i;
	int f = 0;
	unsigned char locw = 0x80 | LOCATION;
	unsigned char *p;

	Redraw = 0;
	OutReset();
	OutCaps();
	TopWindow();

	if (Flag[1]) {
		Message(25);
		BottomWindow();
		return;
	}
	PrintRoom(LOCATION);
        OutChar(' ');
	for (i = 0; i < NumLowObjs; i++) {
		if (Object[i] == LOCATION)
			PrintObject(i);
	}

	p = Image + ExitBase;

	while (*p != locw)
		p++;
	p++;
	while (*p < 0x80) {
		if (f == 0)
			Message(13);
		f = 1;
		OutCaps();
		Message(*p);
		p += 2;
	}
	if (f == 1) {
		OutReplace('.');
		OutChar('\n');
	}
	f = 0;

	for (; i < NUMOBJECTS; i++) {
		if (Object[i] == LOCATION) {
			if (f == 0) {
				Message(0);
				if (GameVersion == 0)
					OutReplace(0);
			}
			f = 1;
			PrintObject(i);
		}
	}
	if (f == 1)
		OutReplace('.');
	OutChar('\n');
	BottomWindow();
}


static void Goto(unsigned char loc)
{
	Flag[0] = loc;
	Redraw = 1;
}

static void Delay(unsigned char n)
{
	sleep(n);
}

static void Wear(unsigned char obj)
{
	if (Object[obj] == WORN) {
		Message(29);
		return;
	}
	if (Object[obj] != CARRIED) {
		Message(23);
		return;
	}
	Put(obj, WORN);
}

static void Remove(unsigned char obj)
{
	if (Object[obj] != WORN) {
		Message(28);
		return;
	}
	if (CarryItem() == 0) {
		Message(15);
		return;
	}
	Put(obj, CARRIED);
}

static void TakeAll(uint_fast8_t start)
{
    uint_fast8_t found = 0;
    uint_fast8_t i;
    
    if (Flag[1]) {
    	Message(25);
        return;
    }
    for (i = start; i < NUMOBJECTS; i++) {
        if (Object[i] == LOCATION) {
            if (found)
                OutChar('\n');
            found = 1;
            PrintObject(i);
            OutReplace(0);
            OutString("......");
            if (CarryItem() == 0) {
                Message(15);
                return;
            }
            OutKillSpace();
            OutString("Taken");
            OutFlush();
            Put(i, CARRIED);
        }
    }
    if (!found) {
        Message(31);
    }
}

static void Means(unsigned char vb, unsigned char no)
{
	Word[0] = vb;
	Word[1] = no;
}

static void ExecuteLineCode(unsigned char *p)
{
	unsigned char arg1, arg2;
	int n;
	
	do {
		unsigned char op = *p;

		if (op & 0x80)
			break;
		p++;
		arg1 = *p++;
		if (op > 20)
			arg2 = *p++;
		switch (op) {
		case 1:
			if (LOCATION == arg1)
				continue;
			break;

		case 2:
			if (LOCATION != arg1)
				continue;
			break;
		case 3:
			if (LOCATION > arg1)
				continue;
			break;
		case 4:
			if (LOCATION < arg1)
				continue;
			break;
		case 5:
			if (Present(arg1))
				continue;
			break;
		case 6:
			if (Object[arg1] == LOCATION)
				continue;
			break;
		case 7:
			if (!Present(arg1))
				continue;
			break;
		case 8:
			if (Object[arg1] != LOCATION)
				continue;
			break;
		case 9:
			/*FIXME : or worn ?? */
			if (Object[arg1] == CARRIED)
				continue;
			if (Object[arg1] == WORN)
				continue;
			break;
		case 10:
			/*FIXME : or worn ?? */
			if (Object[arg1] != CARRIED)
				continue;
			if (Object[arg1] != WORN)
				continue;
			break;
		case 11:
			if (Object[arg1] == WORN)
				continue;
			break;
		case 12:
			if (Object[arg1] != WORN)
				continue;
			break;
		case 13:
			if (Object[arg1] != DESTROYED)
				continue;
			break;
		case 14:
			if (Object[arg1] == DESTROYED)
				continue;
			break;
		case 15:
			/* Work around for Temple of Terror */
			if (GameNum == G_TEMPLE) {
				if (arg1 == 28 && Flag[63] == 0 &&
				    Word[0] == 20 && Word[1] == 162)
					Flag[28] = 0;
			}
			if (Flag[arg1] == 0)
				continue;
			break;
		case 16:
			if (Flag[arg1] != 0)
				continue;
			break;
		case 17:
			if (Word[2] == arg1)
				continue;
			break;
		case 18:
			if (Word[3] == arg1)
				continue;
			break;
		case 19:
			if (Word[4] == arg1)
				continue;
			break;
		case 20:
			if (Chance(arg1))
				continue;
			break;
		case 21:
			if (Flag[arg1] < arg2)
				continue;
			break;
		case 22:
			if (Flag[arg1] > arg2)
				continue;
			break;
		case 23:
			/* Work around for Temple of Terror */
			if (arg1 == 12 && arg2 == 4)
				arg1 = 68;
			if (Flag[arg1] == arg2)
				continue;
			break;
		case 24:
			if (Flag[arg1] != arg2)
				continue;
			break;
		case 25:
			if (Object[arg1] == arg2)
				continue;
			break;
		default:
			writes("Unknown condition.\n");
			break;
		}
		return;
	} while (1);

	ActionsExecuted = 1;

	do {
		unsigned char op = *p;
		if (!(op & 0x80))
			break;
		p++;
		if (op & 0x40)
			ActionsDone = 1;
		op &= 0x3F;

		if (op > 8)
			arg1 = *p++;
		if (op > 21)
			arg2 = *p++;
		switch (op) {
		case 1:
			LoadGame();
			break;
		case 2:
			QuitGame();
			break;
		case 3:
			Inventory();
			break;
		case 4:
			AnyKey();
			break;
		case 5:
			SaveGame();
			break;
		case 6:
			/* Some games this prints things others it does not.
			   We don't atm deal with that detail */
			DropAll();
			break;
		case 7:
			Look();
			break;
		case 8:
			/* Guess */
			Message(8);
			break;
		case 9:
			/* TODO: I believe Rebel Planet also has the old
			   behaviour on GET but it's not clear it matters */
			GetObject(arg1);
			break;
		case 10:
			/* On the older games GET/DROP completed the action */
			if (DropObject(arg1) && GameNum == G_REBEL)
				ActionsDone = 1;
			break;
		case 11:
			/* He Man has custom code here */
			if (GameNum == G_HEMAN && arg1 == 83)
				Checkpoint();
			Goto(arg1);
			break;
		case 12:
			/* Blizzard pass era */
			if (GameVersion == 1)
				Goto(Object[arg1]);
			else
				Message2(arg1);
			break;
		case 13:
			Flag[arg1] = 255;
			break;
		case 14:
			Flag[arg1] = 0;
			break;
		case 15:
			Message(arg1);
			break;
		case 16:
			Put(arg1, LOCATION);
			break;
		case 17:
			Put(arg1, DESTROYED);
			break;
		case 18:
			PrintNumber(Flag[arg1]);
			break;
		case 19:
			Delay(arg1);
			break;
		case 20:
			Wear(arg1);
			break;
		case 21:
			Remove(arg1);
			break;
		case 22:
			/* More workaround bits */
			if (GameNum == G_TEMPLE) {
				if (arg1 == 28 && arg2 == 2)
					Flag[63] = !!(Object[48] == LOCATION);
			}
			Flag[arg1] = arg2;
			break;
		case 23:
			/* More workaround bits */
			if (GameNum == G_TEMPLE) {
				if (arg1 == 12 && arg2 == 1)
					arg1 = 60;
			}
			n = Flag[arg1] + arg2;
			if (n > 255)
				n = 255;
			Flag[arg1] = n;
			break;
		case 24:
			n = Flag[arg1] - arg2;
			if (n < 0)
				n = 0;
			Flag[arg1] = n;
			break;
		case 25:
			Put(arg1, arg2);
			break;
		case 26:
			n = Object[arg1];
			Put(arg1, Object[arg2]);
			Put(arg2, n);
			break;
		case 27:
			n = Flag[arg1];
			Flag[arg1] = Flag[arg2];
			Flag[arg2] = n;
			break;
		case 28:
			Means(arg1, arg2);
			break;
		case 29:
			Put(arg1, Object[arg2]);
			break;
		case 30:
			/* Beep */
			break;
		case 31:
			/* Take all operator for early games. Doesn't take
			   flannel only real objects */
			/* ?? NumLowObjs ?? */
			if (GameNum == G_KAYLETH)
				TakeAll(78);
			if (GameNum == G_HEMAN)
				TakeAll(45);
			Redraw = 1;
			break;
		case 32:
			RamSave(1);
			break;
		case 33:
			RamLoad();
			break;
		case 34:
			break;
		case 35:
			Oops();
			break;
		default:
			writes("Unknown command.\n");
			break;
		}
	}
	while (1);
}

static unsigned char *NextLine(unsigned char *p)
{
	unsigned char op;
	while (!((op = *p) & 0x80)) {
		p += 2;
		if (op > 20)
			p++;
	}
	while (((op = *p) & 0x80)) {
		op &= 0x3F;
		p++;
		if (op > 8)
			p++;
		if (op > 21)
			p++;
	}
	return p;
}

static void RunStatusTable(void)
{
	unsigned char *p = Image + StatusBase;

	ActionsDone = 0;
	ActionsExecuted = 0;

	while (*p != 0x7F) {
		ExecuteLineCode(p);
		if (ActionsDone)
			return;
		p = NextLine(p);
	}
}

static void RunCommandTable(void)
{
	unsigned char *p = Image + ActionBase;

	ActionsDone = 0;
	ActionsExecuted = 0;

	while (*p != 0x7F) {
		if ((*p == 126 || *p == Word[0]) &&
		    (p[1] == 126 || p[1] == Word[1])) {
			ExecuteLineCode(p + 2);
			if (ActionsDone)
				return;
		}
		p = NextLine(p + 2);
	}
}

static int AutoExit(unsigned char v)
{
	unsigned char *p = Image + ExitBase;
	unsigned char want = LOCATION | 0x80;
	while (*p != want) {
		if (*p == 0xFE)
			return 0;
		p++;
	}
	p++;
	while (*p < 0x80) {
		if (*p == v) {
			Goto(p[1]);
			return 1;
		}
		p += 2;
	}
	return 0;
}

static void RunOneInput(void)
{
	if (Word[0] == 0 && Word[1] == 0) {
		OutCaps();
		Message(11);
		return;
	}
	if (Word[0] < 11) {
		if (AutoExit(Word[0])) {
			RunStatusTable();
			if (Redraw)
				Look();
			return;
		}
	}
	OutCaps();
	RunCommandTable();

	if (ActionsExecuted == 0) {
		if (Word[0] < 11)
			Message(24);
		else
			Message(12);
		return;
	}
	if (Redraw)
		Look();
	RunStatusTable();
	if (Redraw)
		Look();
}

static int ParseWord(char *p)
{
	unsigned char *words = Image + VerbBase;
	int i;

	while (*words != 126) {
		if (memcmp(words, p, 4) == 0)
			return words[4];
		words += 5;
	}
	return 0;
}

static unsigned GetWord(char **ptr, char *buf)
{
	char *p = *ptr;
	unsigned n;

	while(*p && isspace(*p))
		p++;
	if (*p == 0) {
		*ptr = p;
		return 0;
	}
	n = 0;
	memset(buf, ' ', 4);
	while(*p && !isspace(*p)) {
		if (n < 4)
			*buf++ = toupper(*p);
		p++;
	}
	*ptr = p;
	return 1;
}
	
static void SimpleParser(void)
{
	int nw;
	int i;
	int wn = 0;
	char wb[5][4];
	char buf[128];
	char *p;

	OutChar('\n');
	if (GameVersion > 0) {
		OutCaps();
		Message(14);
	} else
		OutString("> ");
	OutFlush();
	do {
		LineInput(buf, 127);
		p = buf;
		for (nw = 0; nw < 5; nw++) {
			if (GetWord(&p, wb[nw]) == 0)
				break;
		}
	} while (nw == 0);

	for (i = 0; i < nw; i++) {
		Word[wn] = ParseWord(wb[i]);
		if (Word[wn])
			wn++;
	}
	while(wn < 5)
		Word[wn++] = 0;
	/* Blizzard Pass has magic fixups it seems */
	if (GameNum == G_BLIZZARD) {
		switch(Word[0]) {
		case 249:
			Word[0] = 43;
			break;
		case 175:
			Word[0] = 44;
			break;
		case 134:
			Word[0] = 49;
			break;
		}
	}
}

static int FindCode(char *x, unsigned base)
{
	unsigned char *p = Image + base;
	unsigned len = strlen(x);
	while (p < Image + ImageLen - len) {
		if (memcmp(p, x, len) == 0)
			return p - Image;
		p++;
	}
	return -1;
}

int main(int argc, char *argv[])
{
	int shift;
	off_t len;

	if (argv[1] == NULL) {
		writes("taylormade <file>.\n");
		exit(1);
	}

	GameFile = open(argv[1], O_RDONLY);
	if (GameFile == -1) {
		perror(argv[1]);
		exit(1);
	}
	/* Guess initially at He-man style */
	GameVersion = 2;

	len = lseek(GameFile, 0, SEEK_END);
	if (len == 49179) {
		/* 48K game */
	} else if (len == 131103) {
		/* 128K game so Blizzard Pass */
	} else {
		writes("Not a valid .sna.\n");
		exit(1);
	}
	lseek(GameFile, 0x2B00 + DISK_BIAS, SEEK_SET);
	/* Load the image starting after the screen end */
	if (read(GameFile, Image, LOAD_SIZE) != LOAD_SIZE) {
		writes("Image read failed.\n");
		exit(1);
	}
	ImageLen = LOAD_SIZE;

	/* The message analyser will look for version 0 games */
	VerbBase = FindCode("NORT\001N", 0);
	if (VerbBase == -1) {
		writes("No verb table!\n");
		exit(1);
	}

	/* We hard code these unlike the original interpreter. There are
	   only a few games and it removes all the bulky scanning code. */
	switch(VerbBase) {
	case 0x0A59:
		/* Has wait timers we don't handle */
		GameNum = G_REBEL;
		NumLowObjs = 70;
		TokenBase = 0x60DA;
		GameVersion = 0;
		RoomBase = 0x3b15;
		ObjectBase = 0x5821;
		StatusBase = 0x219d;
		ActionBase = 0x28e0;
		ExitBase = 0x18a3;
		FlagBase = 0x4f9;
		ObjLocBase = 0x731e;
		MessageBase = 0x4305;
		Message2Base = 0x5419;
		/* Load the image offset by 18A3 bytes from 5B00 */
		shift = 0x18A3;
		break;
	case 0x3C20:
		/* Should work */
		GameNum = G_BLIZZARD;
		NumLowObjs = 69;
		GameVersion = 1;
		NoCarryLimit = 1;
		TokenBase = 0x56BC;
		RoomBase = 0x1ace;
		ObjectBase = 0x31e6;
		StatusBase = 0x3bd9;
		ActionBase = 0x3f54;
		ExitBase = 0x5067;
		FlagBase = 0x552;
		ObjLocBase = 0xb6a8;
		MessageBase = 0x2326;
		Message2Base = 0x0;
		/* Rooms are the first block in v1 games so load from there */
		shift = 0x1ACE;
		break;
	case 0x1D3A:
		/* Needs more testing */
		GameNum = G_HEMAN;
		NumLowObjs = 48;
		TokenBase = 0x31E0;
		RoomBase = 0x4036;
		ObjectBase = 0x3ba2;
		StatusBase = 0x2abf;
		ActionBase = 0x1cd8;
		ExitBase = 0x292f;
		FlagBase = 0x346;
		ObjLocBase = 0x83cb;
		MessageBase = 0x4b65;
		Message2Base = 0x34ee;
		/* V2 games start with the actions */
		shift = 0x1CD8;
		break;
	case 0x209D:
		/* Needs more testing */
		GameNum = G_TEMPLE;
		NumLowObjs = 51;
		TokenBase = 0x35D4;
        	RoomBase = 0x38b3;
		ObjectBase = 0x414f;
		StatusBase = 0x2d04;
		ActionBase = 0x1da4;
		ExitBase = 0x2b5c;
		FlagBase = 0x6c0;
		ObjLocBase = 0x8594;
		MessageBase = 0x4b7d;
		Message2Base = 0x5ad0;
		/* V2 games start with the actionss */
		shift = 0x1DA4;
		break;
	case 0x2661:
		/* Currently doesn't work */
		GameNum = G_KAYLETH;
		GameVersion = 0;
		NumLowObjs = 54;
		TokenBase = 0x6323;
		RoomBase = 0x53b3;
		ObjectBase = 0x5d70;
		StatusBase = 0x344d;
		ActionBase = 0x1f48;
		ExitBase = 0x32e0;
		FlagBase = 0x363;
		ObjLocBase = 0x8d5f;
		MessageBase = 0x3f99;
		Message2Base = 0x5b9d;
		/* V2 games start with the actions */
		shift = 0x1F40;
		break;
	default:
		writes("Unknown game image\n");
		exit(1);
	}

	/* So it's relative to the 1B00 start of main memory */
	VerbBase += 0x1000;

	lseek(GameFile, DISK_BIAS + 0x1B00 + FlagBase, SEEK_SET);
	read(GameFile, InitFlag, 6);

	/* Turn the offset into an address relative to the image */
	shift += 0x1B00;

	/* Seek to the byte we are looking for */
	if (lseek(GameFile, shift + DISK_BIAS, SEEK_SET) == -1 ||
	    read(GameFile, Image, LOAD_SIZE) != LOAD_SIZE) {
		writes("Failed to load game data.\n");
		exit(1);
	}

#ifdef LOAD_ALL
	if (GameNum == G_BLIZZARD) {
		if (lseek(GameFile, 0x18000 + DISK_BIAS, SEEK_SET) == -1 ||
			read(GameFile, Image + LOAD_SIZE, 0x4000) != 0x4000) {
			writes("Failed to load text block.\n");
			exit(1);
		}
	}
#endif	

	/* Turn this back into the relative shift */
	shift -= 0x1B00;

	/* This is a disk offset not a memory image one */
	ObjLocBase -= 0x4000;		/* Image starts at 0x4000 */
	ObjLocBase += DISK_BIAS;	/* This is used to load from disk */

	/* Main block: all the tables are shifted down by the amount we
	   threw away having worked out where to load from */
	TokenBase -= shift;
	ExitBase -= shift;
	StatusBase -= shift;
	MessageBase -= shift;
	Message2Base -= shift;
	RoomBase -= shift;
	ObjectBase -= shift;
	VerbBase -= shift;
	ActionBase -= shift;

	/* ObjLoc Flag and Message bases we don't adjust as we work those
	   relative to disc */
	NewGame();
	DisplayInit();
	RamSave(0);
	Look();
	RunStatusTable();
	if (Redraw)
		Look();
	while (1) {
		Checkpoint();
		SimpleParser();
		RunOneInput();
	}
}
