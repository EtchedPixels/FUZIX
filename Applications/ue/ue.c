
/*
   ue.c from ae.c. Public Domain 1991 by Anthony Howe.  All rights released.
   version 1.25, Public Domain, (C) 2002 Terry Loveall, 
   compile with 'gcc ue.c -O2 -fomit-frame-pointer -o ue;strip ue;ls -al ue'
   THIS PROGRAM COMES WITH ABSOLUTELY NO WARRANTY OR BINARIES. COMPILE AND USE 
   AT YOUR OWN RISK.

Cursor Control/Command Keys:
----------------------------
left			^S
right			^D
up				^E
down			^X
word left		^A
word right		^F
goto line begin	^[
goto line end	^]
pgdown			^C
pgup			^R
top of file		^T
bottom of file	^B
del char		^G
del prev char	^H
del rest of line^Y
undo			^U
write file		^W
look for string	^L
quit			^Q
ins tab spaces	^I

Cursor positioning:
Tab spacing requires that internal x-y is (0,0) based, but the linux 
terminal is (1,1) based. display() takes care of the conversion.
*/

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "ue.h"

COORD outxy;

int ROWS = MAXROWS;
int COLS = MAXCOLS;
int LINES = 1;
int done;
int row, col;

char prompt[]="Look for: ";
char sstring[MAXCOLS];	// search string, used by look()
char ubuf[UBUF];			// undo buffer
char ks[6];				// keyboard input string, used by getch()

char buf[BUF];
char *ebuf = buf + BUF;
char *etxt = buf;
char *curp = buf;
char *page, *epage;
char *filename;

/* undo record struct */
typedef struct {
	char ch;
	char *pos;
} U_REC;

U_REC* undop = (U_REC*)&ubuf;

struct termios termios, orig;

// edit key function prototypes
void left(void);
void down(void);
void up(void);
void right(void);
void wleft(void);
void pgdown(void);
void pgup(void);
void wright(void);
void lnbegin(void);
void lnend(void);
void top(void);
void bottom(void);
void delete(void);
void bksp(void);
void delrol(void);
void file(void);
void look(void);
void undo(void);
void quit(void);
void nop(void);

// '& 0X1F' means control key
char key[20] = { 
'S' & 0X1F, 'X' & 0X1F, 'E' & 0X1F, 'D' & 0X1F,
'A' & 0X1F, 'C' & 0X1F, 'R' & 0X1F, 'F' & 0X1F,
'[' & 0X1F, ']' & 0X1F, 'T' & 0X1F, 'B' & 0X1F,
'G' & 0X1F, 'H' & 0X1F, 'Y' & 0X1F, 'U' & 0X1F,
'W' & 0X1F, 'L' & 0X1F, 'Q' & 0X1F, '\0'
};

// one to one correspondence to key array, above
void (*func[])(void) = {
	left, down, up, right, 
	wleft, pgdown, pgup, wright,
	lnbegin, lnend, top, bottom, 
	delete, bksp, delrol, undo,
	file, look, quit, nop
};

// generic console I/O

void cleanup(void)
{
	tcsetattr(0, TCSANOW, &orig);
}

void
GetSetTerm(int set)
{
	struct termios *termiop = &orig;
	if(!set) {
		tcgetattr(0,&orig);
		memcpy(&termios, &orig, sizeof(termios));
		termios.c_lflag    &= (~ICANON & ~ECHO & ~ISIG);
		termios.c_iflag    &= (~IXON);
		termios.c_cc[VMIN]  = 1;
		termios.c_cc[VTIME] = 0;
		termiop = &termios;
		atexit(cleanup);
	}
	tcsetattr(0, TCSANOW, termiop);
}

char
getch()
{
	read(0, (void*)&ks, 1);
	if(*ks == 0x1b)read(0, (void*)&ks[1], 5);
	return (*ks);
}

void
put1(char c)
{
	write(1, (void*)&c, 1);
	outxy.X++;
}

void
emitch(int c)
{
	if(c == '\t'){
		do{ put1(' '); }while(outxy.X & TABM);
	}
	else
		put1(c);
	if(c == '\n'){outxy.X=0;outxy.Y++;};
}

// end of I/O

char *prevline(char *p)
{
	while (buf < --p && *p != '\n');
	return (buf < p ? ++p : buf);
}

char *nextline(char *p)
{
	while (p < etxt && *p++ != '\n');
	return (p < etxt ? p : etxt);
}

char *adjust(char *p, int column)
{
	int i = 0;
	while (p < etxt && *p != '\n' && i < column) {
		i += *p++ == '\t' ? TABSZ-(i&TABM) : 1;
	}
	return (p);
}

void
left(void)
{
	if (buf < curp)
		--curp;
} 

void
down(void)
{
	curp = adjust(nextline(curp), col);
}

void
up(void)
{
	curp = adjust(prevline(prevline(curp)-1), col);
}

void
right(void)
{
	if (curp < etxt)
		++curp;
}

void
wleft(void)
{
	while (isspace(*(curp-1)) && buf < curp)
		--curp;
	while (!isspace(*(curp-1)) && buf < curp)
		--curp;
}

void
pgdown(void)
{
	page = curp = prevline(epage-1);
	while (0 < row--)
		down();
	epage = etxt;
}

void
pgup(void)
{
	int i = ROWS;
	while (0 < --i) {
		page = prevline(page-1); 
		up();
	}
}

void
wright(void)
{
	while (!isspace(*curp) && curp < etxt)
		++curp;
	while (isspace(*curp) && curp < etxt)
		++curp;
}

void
lnbegin(void)
{
	curp = prevline(curp);
}

void
lnend(void)
{
	curp = nextline(curp);
	left();
}

void
top(void)
{
	curp = buf;
}

void
bottom(void)
{
	epage = curp = etxt;
}

void
cmove(char *src, char *dest, int cnt)
{
	if(src > dest){
		while(cnt--) *dest++ = *src++;
	}
	if(src < dest){
		src += cnt;
		dest += cnt;
		while(cnt--) *--dest = *--src;
	}
	etxt += dest-src;
}

void
delete(void)
{
	if(curp < etxt){
		if(*curp == '\n') 
			LINES--;
		if((char*)&undop[1] < ubuf+UBUF){
			undop->ch = *curp | 0x80;	// negative means delete
			undop->pos = curp;
			undop++;
		}
		cmove(curp+1, curp, etxt-curp);
	}
}

void
bksp(void)
{
	if(buf < curp){
		left();
		delete();
	}
}

void
delrol(void)
{
	int l=LINES;
	do{ delete();} while(curp < etxt && l == LINES);
}

void
undo(void)
{
	if((char *)undop > ubuf){
		undop--;
		curp = undop->pos;
		if(undop->ch & 0x80){		// negative means was delete
			cmove(curp, curp+1, etxt-curp);		// so insert
			*curp = undop->ch & ~0x80;
			if(*curp == '\n') LINES++;
		}
		else{	// was insert so delete
			if(*curp == '\n') LINES--;
			cmove(curp+1, curp, etxt-curp);
		}
	}
}

void
file(void)
{
	int i;
	write(i = creat(filename, MODE), buf, (int)(etxt-buf));
	close(i);
}

void look(void)
{
	char c;
	int i;

	i = strlen(sstring);
	gotoxy(1,1);
	clrtoeol();
	write(1, (void*)&prompt,10);
	write(1, (void*)&sstring,i);
	do {
		c = getch();
		if(c == '\b'){
			if(!i) continue;
			i--;emitch(c);emitch(' ');
		}
		else {
			if(i == MAXCOLS) continue;
			sstring[i++] = c;
		}
		if(c != 0x1b) emitch(c);
	} while(c != 0x1b && c != '\n' && c != 0x0c );
	sstring[--i] = 0;

	if(c != 0x1b){
		do{
			right();
		}
		while(curp < etxt && 
		      strncmp(curp,sstring,i));
	}
}

void
quit(void)
{
	done = 1;
}

void
nop(void)
{
}

void
display(void)
{
	int i=0, j=0;
	if (curp < page)
		page = prevline(curp);
	if (epage <= curp) {
		page = curp; 
		i = ROWS;
		while (1 < i--)
			page = prevline(page-1);
	}
	epage = page;
	gotoxy(1,1);
	while (1) {
		if (curp == epage) {
			row = i;
			col = j;
		}
		if (i >= ROWS || LINES <= i || etxt <= epage)
			break;
		if (*epage == '\n' || COLS <= j) {
			++i;
			j = 0;
			clrtoeol();
		}
		if (*epage != '\r') {
			emitch(*epage);
			j += *epage == '\t' ? TABSZ-(j&TABM) : *epage == '\n' ? 0 : 1;
		}
		++epage;
	}
	clrtoeos();
	gotoxy(col+1, row+1);
}

int main(int argc, char **argv)
{
	int i;
	char ch, *p;
#ifdef VTSIZE
	int16_t vtsize;
#endif
#ifdef TIOCGWINSZ
	struct winsize w;
#endif

	if (argc < 2)
		return (2);
	GetSetTerm(0);
#ifdef VTSIZE
	vtsize = ioctl(0, VTSIZE, 0);
	if (vtsize != -1) {
		ROWS = vtsize >> 8;
		COLS = vtsize & 0xFF;
	}
#endif
#ifdef TIOCGWINSZ
	if (ioctl(0, TIOCGWINSZ, &w) != -1) {
		if (w.ws_row != 0)
			ROWS = w.ws_row;
		if (w.ws_col != 0)
			COLS = w.ws_col;
	}
#endif
	tty_init();

	if (0 < (i = open(filename = *++argv, 0))) {
		etxt += read(i, buf, BUF);
		if (etxt < buf)
			etxt = buf;
		else{
			p = etxt;
			while(p > buf)
				if(*--p == '\n') LINES++;
		}
		close(i);
	}
	while (!done) {
		display();
		ch = getch(); 
		i = 0; 
		while (key[i] != ch && key[i] != '\0')
			++i;
		(*func[i])();
		if(key[i] == '\0'){
			if (etxt < ebuf) {
				cmove(curp, curp+1, etxt-curp);
				*curp = ch == '\r' ? '\n' : ch;
				if(*curp++ == '\n') LINES++;
				if((char*)&undop[1] < ubuf+UBUF){
//					undop->ch = curp[-1];
					undop->pos = curp-1;	// positive means insert
					undop++;
				}
			}
		}
	}
	gotoxy(1,ROWS+1);
	GetSetTerm(1);
	return (0);
}
