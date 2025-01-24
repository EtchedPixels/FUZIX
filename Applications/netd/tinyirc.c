/* 
   NanoIRC 0.1

   Based heavily upon

   TinyIRC 1.1
   Copyright (C) 1991-1996 Nathan I. Laredo

   This program is modifiable/redistributable under the terms
   of the GNU General Public Licence.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   Send your comments and all your spare pocket change to
   laredo@gnu.ai.mit.edu (Nathan Laredo) or to PSC1, BOX 709,
   Lackland AFB, TX, 78236-5128
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>
#include <pwd.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "netdb.h"
#include <termcap.h>

#include "linein.h"

#define COMMANDCHAR	'/'
#define ASCIIHEXCHAR	'@'
#define HEXASCIICHAR	'#'
#define RELEASE		"NanoIRC 0.1"
/* most bytes to try to read from server at one time */
#define IB_SIZE		256

static unsigned short irc_port = 6667;

struct dlist {
	char name[64];
	char mode[64];
	struct dlist *next;
};

#define ischan(x) (*x == '#' || *x == '&' || *x == '+')

static struct dlist *obj = NULL, *olist = NULL, *newobj;
static int my_tcp;
static int sok = 1;
static int column;
static char *tmp, *fromhost;
static char *tok[20];
static char ircname[32];
static char *irclogin;
static char *gecos;
static char ib[IB_SIZE];
static char serverdata[512];
static char ch;
static char lineout[512] = {
"NanoIRC 0.1 based upon TinyIRC\n\
(C) 1991-1996 Nathan Laredo\n\
This is free software with ABSOLUTELY NO WARRANTY.\n\
For details please see the file COPYING."
};

static int cursd = 0;
static int noinput = 0;
static int reconnect = 1;
static time_t idletimer, tmptime;
static struct passwd *userinfo;

static char inbuf[256];
static char input[256];

static struct dlist *additem(char *item, struct dlist *p)
{
	newobj = (struct dlist *) malloc(sizeof(struct dlist));
	if (newobj == NULL) {
		fprintf(stderr, "Out of memory.\n");
		exit(1);
	}
	strlcpy(newobj->name, item, 64);
	newobj->mode[0] = '\0';
	newobj->next = p;
	return newobj;
}

static struct dlist *finditem(register char *item, register struct dlist *p)
{
	while (p != NULL)
		if (strcasecmp(item, p->name) == 0)
			break;
		else
			p = p->next;
	return p;
}

static struct dlist *delitem(register char *item, register struct dlist *p)
{
	register struct dlist *prev = NULL, *start = p;
	while (p != NULL)
		if (strcasecmp(item, p->name) == 0) {
			newobj = p->next;
			if (obj == p)
				obj = NULL;
			free(p);
			if (prev == NULL)
				return newobj;
			else {
				prev->next = newobj;
				return start;
			}
		} else {
			prev = p;
			p = p->next;
		}
	return start;
}

static char encoded[513];

static void hexascii(register const char *s)
{
	uint8_t byte = 0;
	uint8_t k = 0;
	register char *p = encoded;

	while(*s && p < encoded + 400) {
		ch = toupper(*s);
		s++;
		if (ch >= '0' && ch <= '9')
			byte = (byte << 4) | (ch - '0');
		else if (ch >= 'A' && ch <= 'F')
			byte = (byte << 4) | (ch - 'A' + 10);
		else
			continue;
		if (k++ & 1)
			*p++ = byte;
	}
	*p = 0;
}

static void asciihex(register const char *s)
{
	register char *p = encoded;
	static char hex[] = "0123456789ABCDEF";
	
	while(*s && p != encoded + sizeof(encoded) - 1) {
		uint8_t c = *s++;
		*p++ = hex[c >> 4];
		*p++ = hex[c & 15];
	}
	*p = 0;
}

/* FIXME: turn off ndelay for the write */
static int sendline(void)
{
	if (write(my_tcp, lineout, strlen(lineout)) < 1)
		return 0;
	return 1;
}

static int nop(void)
{
	return 1;
}

static int doerror(void)
{
	column = printf("*** ERROR:");
	return 2;
}

static int doinvite(void)
{
	printf("*** %s (%s) invites you to join %s.", tok[0], fromhost, tok[3]);
	return 0;
}

static int dojoin(void)
{
	if (strcmp(tok[0], ircname) == 0) {
		obj = olist = additem(tok[2], olist);
		sprintf(lineout, "MODE :%s\n", obj->name);
		sendline();
		printf("*** Now talking in %s", obj->name);
	} else
		printf("*** %s (%s) joined %s", tok[0], fromhost, tok[2]);
	return 0;
}

static int dokick(void)
{
	printf("*** %s was kicked from %s by %s (%s)", tok[3], tok[2], tok[0], tok[4]);
	if (strcmp(tok[3], ircname) == 0) {
		olist = delitem(tok[2], olist);
		if (obj == NULL)
			obj = olist;
		if (obj != NULL)
			printf("\n\r*** Now talking in %s", obj->name);
	}
	return 0;
}

static int dokill(void)
{
	printf("*** %s killed by %s: ", tok[3], tok[0]);
	if (strcmp(tok[3], ircname) == 0)
		reconnect = 0;	/* don't reconnect if killed */
	return 4;
}

static int domode(void)
{
	char *t = tok[3], op = *tok[3];
	printf("*** %s changed %s to:", tok[0], tok[2]);
	if ((newobj = finditem(tok[2], olist)) != NULL) {
		while ((t = strpbrk(t, "-+psitnml")) != NULL) {
			if (*t == '-' || *t == '+')
				op = *t;
			else if (op == '-')
				for (tmp = strchr(newobj->mode, *t); *tmp != '\0'; tmp++)
					*tmp = *(tmp + 1);
			else
				strncat(newobj->mode, t, 1);
			t++;
		}
	}
	return 3;
}

static int donick(void)
{
	if (strcmp(tok[0], ircname) == 0)
		strlcpy(ircname, tok[2], 32);
	printf("*** %s is now known as %s", tok[0], tok[2]);
	return 0;
}

static int donotice(void)
{
	if (!ischan(tok[2]))
		column = printf("-%s-", tok[0]);
	else
		column = printf("-%s:%s-", tok[0], tok[2]);
	return 3;
}

static int dopart(void)
{
	printf("*** %s (%s) left %s", tok[0], fromhost, tok[2]);
	if (strcmp(tok[0], ircname) == 0) {
		olist = delitem(tok[2], olist);
		if (obj == NULL)
			obj = olist;
		if (obj != NULL)
			printf("\n\r*** Now talking in %s", obj->name);
	}
	return 0;
}

static int dopong(void)
{
	column = printf("*** Got PONG from %s:", tok[0]);
	return 3;
}

static int doprivmsg(void)
{
#ifdef DO_CTCP
	if (strncmp(tok[3], "\01PING", 5) == 0) {	/* lame ctcp ping hack */
		sprintf(lineout, "NOTICE %s :%s\n", tok[0], tok[3]);
		column = printf("*** CTCP PING from %s", tok[0]);
		sendline();
		return 0;
	} else if (strncmp(tok[3], "\01VERSION", 8) == 0) {	/* lame ctcp */
		sprintf(lineout, "NOTICE %s :\01VERSION " RELEASE " :*ix\01\n", tok[0]);
		column = printf("*** CTCP VERSION from %s", tok[0]);
		sendline();
		return 0;
	}
#endif
	if (!ischan(tok[2])) {
		column = printf("*%s*", tok[0]);
	} else if (obj != NULL && strcasecmp(obj->name, tok[2]))
		column = printf("<%s:%s>", tok[0], tok[2]);
	else
		column = printf("<%s>", tok[0]);
	return 3;
}

static int doquit(void)
{
	printf("*** %s (%s) Quit (%s)", tok[0], fromhost, tok[2]);
	return 0;
}

static int dosquit(void)
{
	return 1;
}

static int dotime(void)
{
	return 1;
}

static int dotopic(void)
{
	printf("*** %s set %s topic to \"%s\"", tok[0], tok[2], tok[3]);
	return 0;
}

int donumeric(int num)
{
	switch (num) {
	case 352:
		column = printf("%-14s %-10s %-3s %s@%s :", tok[3], tok[7], tok[8], tok[4], tok[5]);
		return 9;
	case 311:
		column = printf("*** %s is %s@%s", tok[3], tok[4], tok[5]);
		return 6;
	case 324:
		if ((newobj = finditem(tok[3], olist)) != NULL)
			strlcpy(newobj->mode, tok[4], 64);
		break;
	case 329:
		tmptime = atoi(tok[4]);
		strcpy(lineout, ctime(&tmptime));
		tmp = strchr(lineout, '\n');
		if (tmp != NULL)
			*tmp = '\0';
		column = printf("*** %s formed %s", tok[3], lineout);
		return 0;
	case 333:
		tmptime = atoi(tok[5]);
		strcpy(lineout, ctime(&tmptime));
		tmp = strchr(lineout, '\n');
		if (tmp != NULL)
			*tmp = '\0';
		column = printf("*** %s topic set by %s on %s", tok[3], tok[4], lineout);
		return 0;
	case 317:
		tmptime = atoi(tok[5]);
		strcpy(lineout, ctime(&tmptime));
		tmp = strchr(lineout, '\n');
		if (tmp != NULL)
			*tmp = '\0';
		column = printf("*** %s idle %s second(s), on since %s", tok[3], tok[4], lineout);
		return 0;
	case 432:
	case 433:
		printf("*** You've chosen an invalid nick.  Choose again.");
		tmp = ircname;
		tty_restore();
		printf("New Nick? ");
		fflush(stdout);
		fgets(ircname, 32, stdin);
		tmp = strchr(ircname, '\n');
		if (tmp)
			*tmp = '\0';
		tty_resume();
		sprintf(lineout, "NICK :%s\n", ircname);
		sendline();
		return 0;
	default:
		break;
	}
	column = 0;//printf("%s", tok[1]);
	return 3;
}

#define LISTSIZE 49
static char *clist[LISTSIZE] = { "ADMIN", "AWAY", "CLOSE", "CONNECT", "DIE", "DNS", "ERROR", "HASH",
	"HELP", "INFO", "INVITE", "ISON", "JOIN", "KICK", "KILL", "LINKS", "LIST",
	"LUSERS", "MODE", "MOTD", "MSG", "NAMES", "NICK", "NOTE", "NOTICE", "OPER",
	"PART", "PASS", "PING", "PONG", "PRIVMSG", "QUIT", "REHASH", "RESTART",
	"SERVER", "SQUIT", "STATS", "SUMMON", "TIME", "TOPIC", "TRACE", "USER",
	"USERHOST", "USERS", "VERSION", "WALLOPS", "WHO", "WHOIS", "WHOWAS"
};

#define DO_JOIN 12
#define DO_MSG 20
#define DO_PART 26
#define DO_PRIVMSG 30
#define DO_QUIT 31
static int numargs[LISTSIZE] = {
	15, 1, 15, 3, 15, 15, 15, 1, 15, 15, 15, 15, 15, 3, 2, 15, 15, 15,
	15, 15, 2, 1, 1, 2, 2, 15, 15, 1, 1, 1, 2, 1, 15, 15, 15, 2, 15,
	15, 15, 2, 15, 4, 15, 15, 15, 1, 15, 15, 15
};

static int (*docommand[LISTSIZE]) (void) = {
nop, nop, nop, nop, nop, nop, doerror, nop, nop, nop, doinvite,
	    nop, dojoin, dokick, dokill, nop, nop, nop, domode, nop, nop, nop, donick, nop, donotice, nop, dopart, nop, nop, dopong, doprivmsg, doquit, nop, nop, nop, dosquit, nop, nop, dotime, dotopic, nop, nop, nop, nop, nop, nop, nop, nop, nop};

static int wordwrapout(register char *p, register int count)
{
	while (p != NULL) {
		if ((tmp = strchr(p, ' ')) != NULL)
			*(tmp++) = '\0';
		if (strlen(p) < tty_width - count) {
			if (count == 0)
				count += printf("%s", p);
			else
				count += printf(" %s", p);
		} else
			count = printf("\n\r   %s", p);
		p = tmp;
	}
	return count;
}

static int parsedata(void)
{
	register int i;
	int found = 0;

	if (serverdata[0] == 'P') {
		sprintf(lineout, "PONG :%s\n", ircname);
		return sendline();
	}
	tok[i = 0] = serverdata;
	tok[i]++;
	while (tok[i] != NULL && i < 15)
		if (*tok[i] == ':')
			break;
		else {
			if ((tmp = strchr(tok[i], ' ')) != NULL) {
				tok[++i] = &tmp[1];
				*tmp = '\0';
			} else
				tok[++i] = NULL;
		}
	if (tok[i] != NULL && *tok[i] == ':')
		tok[i]++;
	tok[++i] = NULL;
	if ((tmp = strchr(tok[0], '!')) != NULL) {
		fromhost = &tmp[1];
		*tmp = '\0';
	} else
		fromhost = NULL;
	column = 0;
	if ((i = atoi(tok[1])) != 0)
		i = donumeric(i);
	else {
		for (i = 0; i < LISTSIZE && !found; i++)
			found = (strcmp(clist[i], tok[1]) == 0);
		if (found)
			i = (*docommand[i - 1]) ();
		else
			i = nop();
	}
	if (i) {
		if (*tok[i] == ASCIIHEXCHAR && tok[i + 1] == NULL) {
			hexascii(&tok[i][1]);
			wordwrapout(encoded, column);
		} else {
			while (tok[i])
				column = wordwrapout(tok[i++], column);
		}
	}
	putchar('\n');
	if (strncmp(tok[1], "Closing", 7) == 0)
		return (reconnect = 0);
	return 1;
}

static int serverinput(void)
{
	register int i;
	int count;
	int hidden = 0;
	
	while ((count = read(my_tcp, ib, IB_SIZE)) >= 1) {
		for (i = 0; i < count; i++) {
			if (ib[i] == '\n') {
				serverdata[cursd] = '\0';
				cursd = 0;
				if (!hidden) {
					tty_hide();
					hidden = 1;
				}
				if (!parsedata())
					return 0;
			} else if (ib[i] != '\r') {
				if (cursd < 512)
					serverdata[cursd++] = ib[i];
			}
		}
	}
	if (count == 0)
		count = -1;	/* EOF */
	else if (errno == EWOULDBLOCK || errno == EAGAIN)
		count = 0;
	if (hidden)
		tty_show();
	return count;
}

static void parseinput(void)
{
	int i, j, outcol = 0, c, found = 0;
	if (*inbuf == '\0')
		return;
	strcpy(input, inbuf);
	tok[i = 0] = input;
	while (tok[i] != NULL && i < 5)
		if ((tmp = strchr(tok[i], ' ')) != NULL) {
			tok[++i] = &tmp[1];
			*tmp = '\0';
		} else
			tok[++i] = NULL;
	tok[++i] = NULL;
	if (*tok[0] == COMMANDCHAR) {
		tok[0]++;
		for (i = 0; i < strlen(tok[0]) && isalpha(tok[0][i]); i++)
			tok[0][i] = toupper(tok[0][i]);
		for (i = 0; i < LISTSIZE && !found; i++)
			found = (strncmp(clist[i], tok[0], strlen(tok[0])) == 0);
		i--;
		if (!found) {
			printf("*** Invalid command\n");
			return;
		}
		if (i == DO_JOIN) {
			if ((newobj = finditem(tok[1], olist)) != NULL) {
				obj = newobj;
				printf("*** Now talking in %s\n", obj->name);
				return;
			} else if (!ischan(tok[1])) {
				obj = olist = additem(tok[1], olist);
				printf("*** Now talking to %s\n", obj->name);
				return;
			}
		}
		if (i == DO_PART && !ischan(tok[1]))
			if ((newobj = finditem(tok[1], olist)) != NULL) {
				olist = delitem(tok[1], olist);
				if (obj == NULL)
					obj = olist;
				printf("*** No longer talking to %s", tok[1]);
				if (obj != NULL)
					printf(", now %s", obj->name);
				putchar('\n');
				return;
			}
		if (i == DO_MSG)
			i = DO_PRIVMSG;
		if (i == DO_PRIVMSG && (tok[1] == NULL || tok[2] == NULL)) {
			printf("*** Unable to parse message\n");
			return;
		}
		strcpy(lineout, clist[i]);
		if (i == DO_QUIT)
			reconnect = 0;
		if (i == DO_QUIT && tok[1] == NULL)
			strcat(lineout, " :" RELEASE);
		j = 0;
#if 0
		if (i != DO_PRIVMSG || tok[1] == NULL)
			outcol = printf("= %s", lineout);
		else if (ischan(tok[1]))
			outcol = printf(">%s>", tok[1]);
		else {
			outcol = printf("-> *%s*", tok[1]);
		}
#else
		outcol = 0;
#endif
		while (tok[++j]) {
			c = strlen(lineout);
			sprintf(&lineout[c], "%s%s", ((j == numargs[i] && tok[j + 1] != NULL) ? " :" : " "), tok[j]);
#if 0			
			if (j > 1 || i != DO_PRIVMSG)
				outcol = wordwrapout(tok[j], outcol);
#endif				
		}
		strcat(lineout, "\n");
	} else {
		if (obj == NULL) {
			printf("*** Nowhere to send\n");
			return;
		}
		if (*tok[0] == ASCIIHEXCHAR) {
			asciihex(&inbuf[1]);
			strcpy(&inbuf[1], encoded);
		} else if (*tok[0] == HEXASCIICHAR) {
			/* display decoded line */
			hexascii(&inbuf[1]);
			outcol = wordwrapout(encoded, outcol);
			return;
		}
		sprintf(lineout, "PRIVMSG %s :%s\n", obj->name, inbuf);
/*		outcol = printf("> %s", tok[j = 0]);
		while (tok[++j])
			outcol = wordwrapout(tok[j], outcol); */
	}
	sendline();
	idletimer = time(NULL);
}

static int userinput(void)
{
	register int err = tty_event();
	if (err < 0)
		return err;
	if (err == 2) {
		parseinput();
	}
	return err;
}

static void cleanup(int sig)
{
	tty_restore();
	fflush(stdout);
#ifndef __hpux
//	psignal(sig, "tinyirc");
#endif
	if (sig != SIGTSTP)
		exit(128 + sig);
	raise(SIGSTOP);
}

static void stopin(int sig)
{
	signal(SIGTTIN, stopin);
	noinput = 1;
}

static void redraw(int sig)
{
	signal(SIGCONT, redraw);
	signal(SIGTSTP, cleanup);
	noinput = 0;
}

static int makeconnect(char *hostname)
{
	struct sockaddr_in sa;
	register struct hostent *hp;
	register int s, t;
	if ((hp = gethostbyname(hostname)) == NULL)
		return -1;
	for (t = 0, s = -1; s < 0 && hp->h_addr_list[t] != NULL; t++) {
		memset(&sa, 0, sizeof(sa));
		bcopy(hp->h_addr_list[t], (char *) &sa.sin_addr, hp->h_length);
		sa.sin_family = hp->h_addrtype;
		sa.sin_port = htons((unsigned short) irc_port);
		s = socket(hp->h_addrtype, SOCK_STREAM, 0);
		if (s > 0) {
			if (connect(s, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
				close(s);
				s = -1;
			} else {
				fcntl(s, F_SETFL, O_NDELAY);
				my_tcp = s;
				sprintf(lineout, "USER %s * * :%s\n", irclogin, gecos);
				sendline();
				sprintf(lineout, "NICK :%s\n", ircname);
				sendline();
				for (obj = olist; obj != NULL; obj = olist->next) {
					sprintf(lineout, "JOIN %s\n", obj->name);
					sendline();
				}	/* error checking will be done later */
			}
		}
	}
	return s;
}

static void parsehost(char *p)
{
	char *tmp = strchr(p, ':');
	if (tmp) {
		*tmp++ = 0;
		irc_port = (unsigned short) atoi(tmp);
	}
}

int main(int argc, char *argv[])
{
	char *hostname;

	puts(lineout);

	if (argc < 2) {
		fprintf(stderr, "%s: server:port [name]\n", argv[0]);
		exit(1);
	}
	hostname = argv[1];
	parsehost(hostname);

	userinfo = getpwuid(getuid());
	if (userinfo == NULL) {
		fprintf(stderr, "User not found\n");
		exit(1);
	}
	if (argv[2])
		strlcpy(ircname, argv[2], sizeof(ircname));
	else
		strlcpy(ircname, userinfo->pw_name, sizeof(ircname));
	irclogin = userinfo->pw_name;
	gecos = userinfo->pw_gecos;
	if (*gecos == '\0')
		gecos = "unknown";

	printf("*** trying port %d of %s\n\n", irc_port, hostname);
	if (makeconnect(hostname) < 0) {
		fprintf(stderr, "*** %s refused connection, aborting\n", hostname);
		exit(0);
	}
	tty_begin();
	
	idletimer = time(NULL);

	redraw(0);
	signal(SIGINT, cleanup);
	signal(SIGHUP, cleanup);
	signal(SIGTERM, cleanup);
	signal(SIGSEGV, cleanup);
	signal(SIGTTIN, stopin);

	tty_set_buffer(inbuf, 0, sizeof(inbuf));

	while (sok >= 0) {
		if (userinput() == -1)
			break;
		sok = serverinput();
		if (sok < 0 && reconnect) {
			close(my_tcp);	/* dead socket */
			printf("*** trying port %d of %s\n\n", irc_port, hostname);
			if (makeconnect(hostname) < 0) {
				fprintf(stderr, "*** %s refused connection\n", hostname);
				break;
			}
		}
		fflush(stdout);
	}
	tty_restore();
	exit(0);
}

/* EOF */
