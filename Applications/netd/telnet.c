/*
 *	Sort of a telnet client, although actually it's more of a MUD client
 *	than that.
 *
 *	Rewritten from dwtelnet - Drivewire - raw Telnet
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "netdb.h"


#define TESC 0x19
#define ESC 0x1b


char ibuf[256];
int opos = 0;

struct termios prior, new;
struct termios lprior, lnew;
int fddw;
int para[4] = { 0, 0, 0, 0 };

int no = 0;
char colors[8] = {
	'0', '2', '4', '6',
	'1', '3', '5', '7'
};

int linemode = 0;
int lecho = -1;
int hex = 0;
int del = 0;
int crnl = -1;
int fc = 7;
int bc = 0;
int inten = 0;
int ansivert = 0;


struct sockaddr_in addr;
struct sockaddr_in laddr;

int tonum(char c)
{
	return c - '0';
}


/*
 *	Batch output so that we generate less system calls
 */
static char outbuf[256];
static char *outptr = outbuf;

void outflush(void)
{
	if (outptr != outbuf) {
		unsigned int len = outptr - outbuf;
		if (write(1, outbuf, len) != len)
			perror("write");
		outptr = outbuf;
	}
}

void qbyte(char c)
{
	*outptr++ = c;
	if (outptr == &outbuf[256])
		outflush();
}

void qstr(const char *p)
{
	while(*p)
		qbyte(*p++);
}

void qhexdigit(char c)
{
	qbyte("0123456789ABCDEF"[c & 0x0F]);
}

void qhex(char c)
{
	qbyte('<');
	qhexdigit(c >> 4);
	qhexdigit(c);
	qbyte('>');
}

void out52(char c)
{
	qbyte('\x1b');
	qbyte(c);
}

void out52n(char c, char n)
{
	out52(c);
	qbyte(n);
}

void out52nn(char c, char n, char m)
{
	out52(c);
	qbyte(n);
	qbyte(m);
}

char cconv(char c)
{
	return colors[c];
}

/* check for matching fore/background colors w/ intensity bit
   we can't do intensity - so text disappears */
void ckint(void)
{
	if ((fc == bc) && inten) {
		fc = (fc + 1) & 0x7;
		out52n('b', fc);
	}
}

/* Send a char to the console, processing for ANSI/TELNET chars */
void charout(uint8_t c)
{
	static char mode = 0;
	int i;

	switch (mode) {
		/* normal mode */
	case 0:
		if (c == 255) {
			mode = 3;
			return;
		}
		if (c != ESC)
			qbyte(c);
		else {
			mode = 1;
		}
		return;
		/* ESC detected */
	case 1:
		if (c == '[') {
			mode = 2;
			return;
		}
		return;
		/* Multi-byte detected */
	case 2:
		if (isdigit(c)) {
			para[no] = para[no] * 10 + tonum(c);
			return;
		}
		if (c == ';') {
			no++;
			return;
		}
		no++;
		goto final;
		/* Telnet IAC detected */
	case 3:
		switch (c) {
		case 255:
			qbyte(c);
			mode = 0;
			return;

		case 251:
			mode = 5;
			return;
		case 252:
			mode = 6;
			return;
		case 253:
		case 254:
			qstr("<IAC>");
			qhex(c);
			mode = 4;
			return;
		}
		mode = 0;
		return;
		/* send Telnet's WILLNOTs to server */
	case 4:
		if (hex) {
			qhex(c);
			qbyte('\n');
		}
		write(fddw, "\xff\xfc", 2);
		write(fddw, &c, 1);
		mode = 0;
		return;
		/* received a WILL */
	case 5:
		if (c == 1) {
			lecho = 0;
			//write(fddw,"\xff\xfd",2);
			//write(fddw, &c, 1 );
			mode = 0;
			return;
		}
		write(fddw, "\xff\xfe", 2);
		write(fddw, &c, 1);
		mode = 0;
		return;
		/* received a WONT */
	case 6:
		if (hex) {
			qstr("opt");
			qhex(c);
			qbyte('\n');
		}
		if (c == 1)
			lecho = -1;
		else {
			write(fddw, "\xff\xfe", 2);
			write(fddw, &c, 1);
		}
		mode = 0;
		return;

	}
	/* and the trailing command */
      final:
	//      printf("DEBUG: %d %d %d %c\n", para[0], para[1], para[2], c);
	switch (c) {
	case 'J':
		if (para[0] == 2) {
			out52('H');
			out52('J');
			break;
		}
		if (para[1] == 0) {
			out52('J');
			break;
		}
	case 'H':
	case 'f':
		if (para[0] == 0 && para[1] == 0) {
			out52('H');
			break;
		}
		if (para[0] == 0)
			para[0] = 1;
		if (para[1] == 0)
			para[1] = 1;
		out52nn('Y', para[0] + ' ', para[1] + ' ');
		break;
	case 'K':
		if (para[0] == 0) {
			out52('K');
			break;
		}
	case 'm':
		for (i = 0; i < no; i++) {
			if (para[i] >= 30 && para[i] <= 37) {
				fc = cconv(para[i] - 30);
				out52n('b', fc);
				ckint();
			}
			if (para[i] >= 40 && para[i] <= 47) {
				bc = cconv(para[i] - 40);
				out52n('c', bc);
				ckint();
			}
			if (para[i] == 0) {
				out52n('b', '4');
				out52n('c', '0');
				inten = 0;
			}
			if (para[i] == 1) {
				inten = 1;
				ckint();
			}
		}
		break;
	case 'C':
	case 'D':
	case 'A':
	case 'B':
		if (para[0] == 0)
			para[0] = 1;
		for (i = 0; i < para[0]; i++)
			out52(c);
		break;
	case 'n':
		if (para[0] == 6) {
			write(fddw, "\x1b[1;1R", 6);
			out52('H');
			out52('J');
		}
		break;
		//      default:

	}
	para[0] = 0;
	para[1] = 0;
	para[2] = 0;
	para[3] = 0;
	no = 0;
	mode = 0;
}

/* print string to console under ANSI/TELNET */
/* TODO: a mode where we just pass through */
int mywrite(char * ptr, int len)
{
	int i = len;
	if (ansivert) {
		while (i--)
			charout(*ptr++);
	} else {
		while (i--)
			qbyte(*ptr++);
	}
	return len;
}

void quit(int sig)
{
	if (sig == 0)
		outflush();
	tcsetattr(0, TCSANOW, &lprior);
	close(fddw);
	exit(0);
}

/* Add string to input buffer */
void addstr(char *s)
{
	for (; (ibuf[opos++] = *s) != 0; s++)
		if (lecho)
			qbyte(*s);
	opos--;
}

void printd(char *s)
{
	qstr(s);
	qbyte('\n');
}

void printhelp(void)
{
	printd("l linemode toggle");
	printd("e echo toggle");
	printd("q quit");
	printd("h IAC TELNET hex debugging");
	printd("n NL / CRNL");
	printd("^A send ^A to remote");
}

/* Read from standard input, nonblocking */
int myread(void)
{
	static int icount = 0;
	static int ipos = 0;
	static char kbuf[256];
	static int mode = 0;
	static int cmode = 0;
	int l;

	/* Make sure anything received is visible */
	outflush();

	switch (mode) {
	case 0:		// waiting for input buffer to fill middle 
		l = read(0, kbuf, 256);
		if (!l)
			return 0;
		if (l < 0) {
			if (errno == EAGAIN) {
				return 0;
			} else {
				quit(0);
			}
		}
		ipos = 0;
		icount = l;
		mode = 1;
	case 1:		// processing middle buffer to input buffer
		for (; ipos < icount; ipos++) {
			char c = kbuf[ipos];
			if (cmode) {
				switch (c) {
					/*
					   case '1':
					   addstr( "tcp connect " );
					   break;
					   case '2':
					   addstr( "nostalgiamud.l-w.ca 4000");
					   break;
					   case '3':
					   addstr( "aardmud.org 23" );
					   break;
					   case '4':
					   addstr( "vert.synchro.net 23" );
					   break;
					   case '5':
					   addstr( "madworld.bounceme.net 6400");
					   break;
					 */
				case 'l':
					linemode = ~linemode;
					break;
				case 'e':
					lecho = ~lecho;
					break;
				case 'q':
					quit(0);
				case 'h':
					hex = ~hex;
					break;
				case 'd':
					del = ~del;
					break;
				case 'n':
					crnl = ~crnl;
					break;
				case 1:
					ibuf[opos++] = 1;
					break;
				case '?':
					printhelp();
					break;
				}
				cmode = 0;
				continue;
			}
			if (c == 1) {
				cmode = 1;
				continue;
			}
			if (linemode) {
				if (c == 8) {
					if (opos) {
						opos--;
						write(1, "\b \b", 3);
						continue;
					}
					continue;
				}
				write(1, &c, 1);
				if (c == '\n') {
					int t = opos;
					opos = 0;
					ibuf[t] = c;
					ipos++;
					return ++t;
				}
				ibuf[opos++] = c;
				continue;
			}
			if (lecho)
				qbyte(c);
			if ((c == 8) && del) {
				ibuf[opos++] = 127;
				continue;
			}
			if ((c == 0x0a) && crnl) {
				ibuf[opos++] = 0x0d;
				ibuf[opos++] = 0x0a;
				continue;
			}
			ibuf[opos++] = c;
			continue;
		}
		mode = 0;
		if (!linemode) {
			int t = opos;
			opos = 0;
			return t;
		}
		return 0;
	default:
		exit(1);
	}
}

int my_open(int argc, char *argv[])
{
	int port = 23;		/* default port */
	struct hostent *h;

	if (argc == 3)
		port = atol(argv[2]);

	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;

	h = gethostbyname(argv[1]);
	if (!h) {
		write(2, "cannot resolve hostname\n", 24);
		exit(1);
	}
	memcpy(&addr.sin_addr.s_addr, h->h_addr_list[0], 4);

	fddw = socket(AF_INET, SOCK_STREAM, 0);
	if (fddw < 0) {
		perror("socket");
		exit(1);
	}

	if (connect(fddw, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("connect");
		exit(1);
	}
	fcntl(fddw, F_SETFL, O_NDELAY);
	return 0;
}

int main(int argc, char *argv[])
{
	int len;

	if (argv[1] && strcmp(argv[1], "-a") == 0) {
		ansivert = 1;
		argv++;
		argc--;
	}

	if (argc < 2) {
		write(2, "telnet: server [port]\n", 22);
		exit(1);
	}

	tcgetattr(0, &lprior);
	memcpy(&lnew, &lprior, sizeof(lnew));

	signal(SIGINT, quit);
	signal(SIGQUIT, quit);
	signal(SIGPIPE, SIG_IGN);

	my_open(argc, argv);

	lnew.c_lflag &= ~ICANON;
	lnew.c_lflag &= ~ECHO;
	/* Use 1/10th delay for now - until we get select */
	lnew.c_cc[VMIN] = 0;
	lnew.c_cc[VTIME] = 1;
	tcsetattr(0, TCSANOW, &lnew);

	qstr("Use Cntl-A ? for help.\n");

	while (1) {
		char *pos = ibuf;
		uint8_t ct = 0;

		/* Keyboard to socket */
		len = myread();
		while (len > 0) {
			int ret = write(fddw, pos, len);
			if (ret < 0) {
				if (errno == EAGAIN) {
					ret = 0;
				} else {
					perror("write");
					quit(0);
				}
			}
			len = len - ret;
			pos += ret;
		}
		/* Socket to console. We do up to 20 loops through this
		   at a go so that big bursts of data feel fast. We don't
		   go forever though as the poor user might be trying to
		   stop the output */
		while(ct++ < 20  && (len = read(fddw, ibuf, 128)) > 0)
		{
			pos = ibuf;
			while (len > 0) {
				int ret = mywrite(pos, len);
				if (ret < 0) {
					if (errno == EWOULDBLOCK) {
						ret = 0;
					} else
						quit(0);
				}
				len = len - ret;
				pos += ret;
			}
		}
		/* EOF: remote closed */
		if (!len) {
			printd("\nConnection closed.");
			quit(0);
		}
		/* Error. EAGAIN is ok - it means we've run out of data,
		   anything else is bad */
		if (len < 0) {
			if (errno != EAGAIN) {
				outflush();
				perror("closed");
				quit(0);
			}
		}
	}
}
