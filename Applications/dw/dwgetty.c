/*
  A Very simple getty for starting a telnet session over Drivewire 4.

*/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <utmp.h>
/* for getty/login */
#include <pwd.h>
#include <signal.h>
#include <grp.h>
#include <paths.h>
#include <sys/wait.h>
#include <errno.h>


int port;
int tries=3;
char *port_txt;
char *listenMsg="tcp listen ";
char *join="tcp join ";
char conn;
struct termios new;

static void spawn_login(struct passwd *, const char *, const char *);
static pid_t getty(const char *, const char *, int);

static struct utmp ut;

char buff[128];

static const unsigned char d_echo[3]={ 255, 253, 1 };
static const unsigned char d_noecho[3]={ 255, 254, 1 };
static const unsigned char d_willecho[3]={ 255, 251, 1 };
static const unsigned char d_wontecho[3]={ 255, 252, 1 };

void doEcho(int f)
{
	write(f, d_echo, 3);
}


void dontEcho(int f)
{
	write(f, d_noecho, 3);
}

void willEcho(int f)
{
	write(f, d_willecho, 3);
}

void wontEcho(int f)
{
	write(f, d_wontecho, 3);
}

void getrply( int f )
{
	unsigned char e[3];
	read( f, e, 3 );
}


#define crlf()   write(1, "\r\n", 2)

void sigalarm(int sig)
{
	return;
}

void putstr(char *str)
{
	write(1, str, strlen(str));
}

int showfile(char *fname)
{
	int fd, len;
	char buf[80];

	fd = open(fname, O_RDONLY);
	if (fd >= 0) {
		do {
			len = read(fd, buf, 80);
			write(1, buf, len);
		} while (len > 0);
		close(fd);
		return 1;
	}
	return 0;
}

static char *env[10];
static int envn;

static void envset(const char *a, const char *b)
{
	int al = strlen(a);
	static char hptr[5];
	/* May unalign the memory pool but we don't care by this point */
	char *tp = sbrk(al + strlen(b) + 2);
	if (tp == (char *) -1) {
		putstr("out of memory.\n");
		return;
	}
	strcpy(tp, a);
	tp[al] = '=';
	strcpy(tp + al + 1, b);
	env[envn++] = tp;
}

static char *argp[] = { "sh", NULL };

static void spawn_login(struct passwd *pwd, const char *tty, const char *id)
{
	char *p, buf[50];
	/* utmp */
	ut.ut_type = USER_PROCESS;
	ut.ut_pid = getpid();
	strncpy(ut.ut_line, tty+5, UT_LINESIZE);
	strncpy(ut.ut_id, id, 2);
	time(&ut.ut_time);
	strncpy(ut.ut_user, pwd->pw_name, UT_NAMESIZE);
	pututline(&ut);
	endutent();

	/* We don't care if initgroups fails - it only grants extra rights */
	initgroups(pwd->pw_name, pwd->pw_gid);

	/* But we do care if these fail! */
	if (setgid(pwd->pw_gid) == -1 ||
		setuid(pwd->pw_uid) == -1)
			_exit(255);
	signal(SIGINT, SIG_DFL);

	/* setup user environment variables */

	envset("LOGNAME", pwd->pw_name);
	envset("HOME", pwd->pw_dir);
	envset("SHELL", pwd->pw_shell);

	/* home directory */

	if (chdir(pwd->pw_dir))
		putstr("login: unable to change to home directory, using /\n");

	/* show the motd file */

	if (!showfile("/etc/motd"))
		crlf();

	/* and spawn the shell */

	strcpy(buf, "-");
	if ((p = strrchr(pwd->pw_shell, '/')) != NULL)
		strcat(buf, ++p);
	else
		strcat(buf, pwd->pw_shell);

	argp[0] = buf;
	argp[1] = NULL;

	execve(pwd->pw_shell, (void *) argp, (void *) env);
	putstr("login: can't execute shell\n");
	exit(1);
}

/* 
   getpass.c implimented for TELNET
*/

#define EOF	(-1)

static int __getchar(int fd)
{
	static char ch;
	return (read(fd, &ch, 1) == 1) ? ch : EOF;
}

static char *_gets(int fd, char *buf, int len)
{
	int ch, i = 0;

	while (i < len) {
		if ((ch = __getchar(fd)) == EOF && i == 0)
			return NULL;
		if (ch == '\n' || ch == '\r')
			break;
		buf[i++] = ch;
	}
	buf[i] = 0;
	return buf;
}

char *getpass(char *prompt)
{
	static char result[128];

	/* display the prompt */
	write(1, prompt, strlen(prompt));

	willEcho(1);
	getrply(0);
	/* read the input */
	if (_gets(0, result, sizeof(result) - 1) == NULL)
	result[0] = 0;
	wontEcho(1);
	/* The newline isn't echoed as we have echo off, so we need to
	   output it as the end of the task */
	write(1, "\n", 1);
	return result;
}



/*
 *	Internal implementation of "getty" and "login"
 */
static pid_t getty(const char *ttyname, const char *id, int fdtty)
{
	struct passwd *pwd;
	const char *pr;
	char *p, buf[50], salt[3];
	char hn[64];
	gethostname(hn, sizeof(hn));

	for (;;) {
		close(0);
		close(1);
		close(2);
		setpgrp();
		setpgid(0,0);

		/* here we are inside child's context of execution */
		envset("PATH", "/bin:/usr/bin");
		envset("CTTY", ttyname);

		/* make stdin, stdout and stderr point to fdtty */
		
		dup(fdtty);
		dup(fdtty);
		dup(fdtty);
		close(fdtty);

		/* display the /etc/issue file, if exists */
		showfile("/etc/issue");
		if (*hn) {
			putstr(hn);
			putstr(" ");
		}
		/* loop until a valid user name is entered
		 * and a shell is spawned */

		for (;;) {
			putstr("login: ");
			while (read(0, buf, 20) < 0);	/* EINTR might happens because of the alarm() call below */
			
			if ((p = strchr(buf, '\n')) != NULL)
				*p = '\0';	/* strip newline */

			pwd = getpwnam(buf);

			if (pwd) {
				if (pwd->pw_passwd[0] != '\0') {
					p = getpass("Password: ");
					salt[0] = pwd->pw_passwd[0];
					salt[1] = pwd->pw_passwd[1];
					salt[2] = '\0';
					pr = crypt(p, salt);
				} else {
					pr = "";
				}
				if (strcmp(pr, pwd->pw_passwd) == 0)
					spawn_login(pwd, ttyname, id);
			}
			if( !--tries ){
				putstr("\nToo Many Attempts\n\n");
				exit(1);
			}
			putstr("\nLogin incorrect\n\n");
			signal(SIGALRM, sigalarm);
			alarm(2);
			pause();
		}
	}
}



void pute( char *mess )
{
	write( 2, mess, strlen(mess) );
}


// dwgetty tty       port
// dwgetty /dev/tty3 6809
int main( int argc, char *argv[])
{
	int f;
	int i;
	int len;
	char c[2];



	if( argc<3){
		pute("usage: dwgetty tty port\n" );
		exit(-1);
	}

	port=atoi( argv[2] );
	port_txt=argv[2];

	f=open( argv[1], O_RDWR );
	if( ! f ){ 
		pute("Cannot open device file\n" );
		exit(-1);
	}

	tcgetattr( f, &new );
	new.c_iflag |= ICRNL;
	new.c_iflag &= ~IGNCR;
	// new.c_oflag |= OPOST | ONLCR;
	//	new.c_oflag &= ~ONLCR;
	new.c_lflag &= ~ECHO;
	tcsetattr( f, TCSANOW, &new );

	write(f,listenMsg,strlen(listenMsg) );
	write(f,port_txt,strlen(port_txt) );
	write(f,"\r",1);

	for( i=0; i<4; i++){
		len=read(f,buff,128);
	}
	conn=buff[0];

	write(f, join, strlen(join) );
	write(f, &conn, 1 );
	write(f, "\r", 1 );
	len=read(f,buff,128);

       	new.c_iflag |= IGNCR;
	tcsetattr( f, TCSANOW,  &new );
	c[0]='0';
	c[1]=argv[1][strlen(argv[1])-1];
	getty( argv[1], c, f );
}


