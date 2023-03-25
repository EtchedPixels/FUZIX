/***************************************************
 UZI (Unix Z80 Implementation) Utilities:  ssh.c
  Simple Shell.  Copyright (C) 1998, Harold F. Bower

 15 Mar 1998 - Added Path searching from Minix.  HFB
 26 Sep 1999 - Added kill command                HP
 29 Sep 1999 - Added pwd and sync commands       HP
 04 Oct 1999 - Added umask command               HP
 27 MAy 2001 - Added simple support for env vars HP
 20 May 2015 - Stripped out stdio usage		 AC
****************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <sys/wait.h>

extern char **environ;      /* Location of Envp from executable Header */

#define MAX_ARGS  16

char buf[128];
char eline[45];		    /* Line for Search command */

char *cmd;
char *arg[MAX_ARGS];
char *inrd;
char *outrd;
char *exrd;
int bg;

int infd;

static void writes(int fd, const char *p)
{
    write(fd, p, strlen(p));
}

static void nl(int fd)
{
    write(fd, "\n", 1);
}

static void writeo(int i)
{
    static char buf[3];
    buf[0] = ((i >> 6) & 7) + '0';
    buf[1] = ((i >> 3) & 7) + '0';
    buf[2] = (i & 7) + '0';
    write(1, buf, 3);
}

static void writenum(int fd, unsigned int n)
{
    char buf[6];
    char *bp = buf+6;
    int c = 0;

    do {
        *--bp = (n % 10) + '0';
        n /= 10;
        c++;
    } while(n);
    write(fd, bp, c);
}

static char *argptr;
static char *argout;
static char argstate;

static char *nextarg(void)
{
    char *p;
    char **t;
    uint8_t subst = 0;
redo:
    p = argout;
    t = NULL;
    while(isspace(*argptr))
        argptr++;
    if (*argptr == '\0')
        return NULL;
    if (memcmp(argptr, "2>", 2) == 0) {
        t = &exrd;
        argptr += 2;
    } else if (*argptr == '<') {
        t = &inrd;
        argptr++;
    } else if (*argptr == '>') {
        t = &outrd;
        argptr++;
    }
    if (*argptr == '$')
        subst = 1;
    while (*argptr != '\0') {
        if (!argstate) {
            if (isspace(*argptr))
                break;
            if (*argptr == '\'' || *argptr == '"')
                argstate = *argptr;
            else
                *argout++ = *argptr;
        } else {
            if (*argptr == argstate)
                argstate = 0;
            else
                *argout++ = *argptr;
        }
        argptr++;
    }
    if (*argptr)
        argptr++;
    *argout++ = 0;
    if (subst) {
        /* Ought to wrap this and handle $? $$ $* and $1 $2 ... */
        char *n = getenv(p + 1);
        if (n)
            return n;
        else
            return "";
    }
    /* Redirections etc are pulled out of line */
    if (t) {
        *t = p;
        goto redo;
    }
    return p;
}

static char *getarg(char *in)
{
    argstate = 0;
    argptr = in;
    argout = in;
    inrd = NULL;
    outrd = NULL;
    exrd = NULL;
    bg = 0;
    return nextarg();
}

static void openf(const char *p, int tfd, int mode)
{
    int fd;

    if (p == NULL)
        return;

    fd = open(p, mode, 0666);
    if (fd == -1) {
        perror(p);
        exit(1);
    }
    dup2(fd, tfd);
    close(fd);
}

static pid_t lastfg = -1;

static pid_t pidwait(pid_t want, int f)
{
    int status;
    pid_t p = waitpid(0, &status, f);
    if (p == -1)
        return -1;

    if (p != want) {
        write(1, "[", 1);
        writenum(1, p);
        write(1, "] ", 2);
        if (WIFEXITED(status)) {
            write(1, "Exit ", 5);
            writenum(1, WEXITSTATUS(status));
            nl(1);
            return p;
        }
    }
    /* Silently for fg process */
    if (WIFEXITED(status)) {
        /* Set $? some day ? */
        return p;
    }
    if (WIFSIGNALED(status)) {
        writes(1, strsignal(WTERMSIG(status)));
        if (WCOREDUMP(status))
            write(1, "(core dumped)", 13);
        nl(1);
        return p;
    }
    if (WIFSTOPPED(status)) {
        write(1, "Stopped\n", 7);
        if (want == 0)
            lastfg = want;
    }
    return p;
}        
    
int main(int argc, char *argval[])
{
    char  *path, *tp, *sp;     /* Pointers for Path Searching */
    int   login_sh, pid = -1, sig, stat, asis, i;
    const char *cprompt;
    char  *home;
    const char  *argv[MAX_ARGS+2];

    login_sh = 0;
    if (argval[0][0] == '-') login_sh = 1;

    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    if (login_sh) {
	if (!getenv("PATH"))
	    putenv("PATH=/bin");
	home = getenv("HOME");
	if (!home) putenv("HOME=/");
	chdir(getenv("HOME"));
	infd = open(".sshrc", O_RDONLY);
	if (infd < 0)
	    infd = 0;
    }

    cprompt = (getuid() == 0) ? "ssh# " : "ssh$ ";

    for (;;) {
        char **argp = arg;
        
        
        
        for (i = 0; i < MAX_ARGS; i++)
            *argp++ = NULL;
        argp = arg;
        do {
            pidwait(0, WNOHANG);
            /* Also check for mail ?? */
            if (infd == 0)
                write(1, cprompt, 5);
            if ((i = read(infd, buf, 127)) <= 0) {
                if (infd == 0)
                    return 0;
                else {
                    close(infd);
                    infd = 0;
                }
            }
            if (buf[i - 1] == '\n')
                buf[i - 1] = '\0';   /* Strip newline from read() */
	    cmd = getarg(buf);
	} while (cmd == NULL);

	for (i = 0; i < MAX_ARGS; i++) {
	    *argp = nextarg();
	    if (*argp == NULL)
	        break;
            argp++;
        }
        argp--;
        if (i && strcmp(*argp, "&") == 0) {
            bg = 1;
            *argp = NULL;
        }

        /* Check for User-Requested Exit back to Login Prompt */
        if (strcmp(cmd, "exit") == 0)
            return 0;                      /* Quit if requested */

        /* Check for User request to change Current Working Directory */
        else if (strcmp(cmd, "cd") == 0) {
            stat = chdir(arg[0] ? arg[0] : getenv("HOME"));
            if (stat)
                perror("cd");
        }
        else if (strcmp(cmd, "sync") == 0) {
            sync();
        }
        
        else if (strcmp(cmd, "umask") == 0) {
            if (arg[0][0] == (char) 0) {
                i = umask(0);
                umask(i);
                writeo(i);
            } else {
                i = 0;
                tp = arg[0];
                while (*tp >= '0' && *tp <= '7')
                    i = (i << 3) + *tp++ - '0';
                if (*tp || (i & ~0777))
                    write(2, "umask: bad mask value\n", 22);
                else
                    umask(i);
            }
        }
        else if (strcmp(cmd, "fg") == 0) {
            if (lastfg == -1 || kill(lastfg, SIGCONT) == -1)
                write(2, "fg: can't continue\n", 19);
            else
                while(pidwait(pid, 0) != pid);
        }
        else if (strcmp(cmd, "bg") == 0) {
            if (lastfg == -1 || kill(lastfg, SIGCONT) == -1)
                write(2, "bg: can't continue\n", 19);
            else
                lastfg = -1;
        }
        /* Check for User request to kill a process */
        else if (strcmp(cmd, "kill") == 0) {
            if (arg[0][0] == '-') {
                sig = atoi(&arg[0][1]);
                pid = atoi(arg[1]);
            } else {
                sig = SIGINT;
                pid = atoi(arg[0]);
            }
            if (pid == 0 || pid == 1) {
                write(2, "kill: can't kill process ", 25);
                writenum(2, pid);
                nl(2);
            } else {
                stat = kill(pid, sig);
                if (stat)
                    perror("kill");
            }
        }
#ifdef SSH_EXPENSIVE
        else if (strcmp(cmd, "pwd") == 0) {
	    if (getcwd(buf, 128)){
	        char *ptr=&buf[strlen(buf)];
	        *ptr++='\n';
		*ptr=0;
	        writes(1, buf);
	    }
            else
                write(1, "pwd: cannot get current directory\n",34);
        }
#endif

        /* Check for environment variable assignment */
        else if ((tp = strchr(cmd, '=')) != NULL) {
	  *tp='\0';
	  if( *(tp+1) == '\0' ) unsetenv( cmd );
	  else setenv(cmd,tp+1,1);
        }

        /* No built-in Command, Try to find Executable Command File */
        else {
            argv[0] = cmd;                  /* Build Argv Pointer Array */
            for (i = 0; i < MAX_ARGS; ++i)
               argv[i+1] = arg[i];
            argv[i+1] = NULL;

            if ((pid = fork()) == -1) {     /* Try to spawn new Process */
                write(2, "ssh: can't fork\n", 16);
            } else {
                if (pid == 0) {             /* Child is in context */
                    signal(SIGINT, SIG_DFL);
                    signal(SIGQUIT, SIG_DFL);

                    openf(inrd, 0, O_RDONLY);
                    openf(outrd, 1, O_WRONLY|O_CREAT);
                    /* Must be done last */
                    openf(exrd, 2, O_WRONLY|O_CREAT);

                    /* Path search adapted from Univ of Washington's Minix */
                    path = getenv("PATH");  /* Get base of path string, or NULL */
                    eline[0] = '\0';
                    sp = strchr(cmd, '/') ? "" : path;
                    asis = *sp == '\0';
                    while (asis || *sp != '\0') {
                        asis = 0;
                        tp = eline;
                        for (; *sp != '\0'; tp++)
                            if ((*tp = *sp++) == ':') {
                                asis = *sp == '\0';
                                break;
                            }
                        if (tp != eline)
                            *tp++ = '/';
                        for (i = 0; (*tp++ = cmd[i++]) != '\0'; )
                            ;
                        execve(eline, (char**) argv, (char**) environ);
                    }
                    write(2, "ssh: ", 5);
                    writes(2, cmd);
                    write(2, "?\n", 2);      /* Say we can't exec */
                    exit(1);
                }
                /* Wait for the child */
                lastfg = pid;
                if (!bg) {
                    while(pidwait(pid, 0) != pid);
                }
            }
        }
    }
}
