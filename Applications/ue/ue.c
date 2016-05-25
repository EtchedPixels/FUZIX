
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
#include <termios.h>

#define BUF 65536*4
#define MODE 0666
#define TABSZ 4
#define TABM TABSZ-1

#define MAXLINES 25
#define MAXCOLS 96

int COLS = MAXCOLS;
int LINES = 1;
int done;
int row, col;

char str[MAXCOLS];		// used by gotoxy and clrtoeol
char prompt[]="Look for: ";
char sstring[MAXCOLS];	// search string, used by look()
char ubuf[BUF];			// undo buffer
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
	int pos;
} U_REC;

U_REC* undop = (U_REC*)&ubuf;

typedef struct {
        int X;
        int Y;
} COORD;

COORD outxy;

struct termios termios, orig;

// edit key function prototypes
void left();
void down();
void up();
void right();
void wleft();
void pgdown();
void pgup();
void wright();
void lnbegin();
void lnend();
void top();
void bottom();
void delete();
void bksp();
void delrol();
void file();
void look();
void undo();
void quit();
void nop();

// '& 0X1F' means control key
char key[20] = { 
'S' & 0X1F, 'X' & 0X1F, 'E' & 0X1F, 'D' & 0X1F,
'A' & 0X1F, 'C' & 0X1F, 'R' & 0X1F, 'F' & 0X1F,
'[' & 0X1F, ']' & 0X1F, 'T' & 0X1F, 'B' & 0X1F,
'G' & 0X1F, 'H' & 0X1F, 'Y' & 0X1F, 'U' & 0X1F,
'W' & 0X1F, 'L' & 0X1F, 'Q' & 0X1F, '\0'
};

// one to one correspondence to key array, above
void (*func[])() = {
	left, down, up, right, 
	wleft, pgdown, pgup, wright,
	lnbegin, lnend, top, bottom, 
	delete, bksp, delrol, undo,
	file, look, quit, nop
};

// generic console I/O

void
GetSetTerm(int set)
{
	struct termios *termiop = &orig;
	if(!set) {
		tcgetattr(0,&orig);
		termios = orig;
		termios.c_lflag    &= (~ICANON & ~ECHO & ~ISIG);
		termios.c_iflag    &= (~IXON);
		termios.c_cc[VMIN]  = 1;
		termios.c_cc[VTIME] = 0;
		termiop = &termios;
	}
	tcsetattr(0, TCSANOW, termiop);
}

void
gotoxy(int x, int y){
	sprintf(str,"%c[%03d;%03dH",0x1b,y,x);
	outxy.Y=y;outxy.X=x;
	write(1, (void*)&str, 10);
}

char
getch()
{
	read(0, (void*)&ks, 1);
	if(*ks == 0x1b)read(0, (void*)&ks[1], 5);
	return (*ks);
}

void
put1(int c)
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

void
clrtoeol()
{
	int i=0;
	while(i < COLS-outxy.X)
		str[i++]=' ';
	write(1, (void*)&str, COLS-outxy.X > 0 ? COLS-outxy.X : 0);
	gotoxy(outxy.X,outxy.Y);
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
left()
{
	if (buf < curp)
		--curp;
} 

void
down()
{
	curp = adjust(nextline(curp), col);
}

void
up()
{
	curp = adjust(prevline(prevline(curp)-1), col);
}

void
right()
{
	if (curp < etxt)
		++curp;
}

void
wleft()
{
	while (isspace(*(curp-1)) && buf < curp)
		--curp;
	while (!isspace(*(curp-1)) && buf < curp)
		--curp;
}

void
pgdown()
{
	page = curp = prevline(epage-1);
	while (0 < row--)
		down();
	epage = etxt;
}

void
pgup()
{
	int i = MAXLINES;
	while (0 < --i) {
		page = prevline(page-1); 
		up();
	}
}

void
wright()
{
	while (!isspace(*curp) && curp < etxt)
		++curp;
	while (isspace(*curp) && curp < etxt)
		++curp;
}

void
lnbegin()
{
	curp = prevline(curp);
}

void
lnend()
{
	curp = nextline(curp);
	left();
}

void
top()
{
	curp = buf;
}

void
bottom()
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
delete()
{
	if(curp < etxt){
		if(*curp == '\n') 
			LINES--;
		if((char*)&undop[1] < ubuf+BUF){
			undop->ch = *curp;
			undop->pos = -(int)curp;	// negative means delete
			undop++;
		}
		cmove(curp+1, curp, etxt-curp);
	}
}

void
bksp()
{
	if(buf < curp){
		left();
		delete();
	}
}

void
delrol()
{
	int l=LINES;
	do{ delete();} while(curp < etxt && l == LINES);
}

void
undo()
{
	if((void*)undop > (void*)ubuf){
		undop--;
		curp = (char*)undop->pos;
		if(undop->pos < 0){	// negative means was delete
			(int)curp = -(int)curp;		// so insert
			cmove(curp, curp+1, etxt-curp);
			*curp = undop->ch;
			if(*curp == '\n') LINES++;
		}
		else{	// was insert so delete
			if(*curp == '\n') LINES--;
			cmove(curp+1, curp, etxt-curp);
		}
	}
}

void
file()
{
	int i;
	write(i = creat(filename, MODE), buf, (int)(etxt-buf));
	close(i);
}

void look()
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
quit()
{
	done = 1;
}

void
nop()
{
}

void
display()
{
	int i=0, j=0;
	if (curp < page)
		page = prevline(curp);
	if (epage <= curp) {
		page = curp; 
		i = MAXLINES;
		while (1 < i--)
			page = prevline(page-1);
	}
	epage = page;
	gotoxy(0,1);
	while (1) {
		if (curp == epage) {
			row = i;
			col = j;
		}
		if (i >= MAXLINES || LINES <= i || etxt <= epage)
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
	i = outxy.Y;
	while(i++ <= MAXLINES){
		clrtoeol();
		gotoxy(1,i);
	}
	gotoxy(col+1, row+1);
}

int main(int argc, char **argv)
{
	int i;
	char ch, *p;
	if (argc < 2)
		return (2);
	GetSetTerm(0);
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
				if((char*)&undop[1] < ubuf+BUF){
//					undop->ch = curp[-1];
					undop->pos = (int)curp-1;	// positive means insert
					undop++;
				}
			}
		}
	}
	gotoxy(1,MAXLINES+1);
	GetSetTerm(1);
	return (0);
}
