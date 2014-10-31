/***************************************************
 UZI (Unix Z80 Implementation) Utilities:  ssh.c
  Simple Shell.  Copyright (C) 1998, Harold F. Bower

 15 Mar 1998 - Added Path searching from Minix.  HFB
 26 Sep 1999 - Added kill command                HP
 29 Sep 1999 - Added pwd and sync commands       HP
 04 Oct 1999 - Added umask command               HP
 27 MAy 2001 - Added simple support for env vars HP
****************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

extern char **environ;      /* Location of Envp from executable Header */

#define MAX_ARGS  10

char buf[128];
char eline[45];		    /* Line for Search command */

char cmd[50], arg[MAX_ARGS][50];


main(int argc, char *argval[])
{
    char  *path, *tp, *sp;     /* Pointers for Path Searching */
    int   login_sh, pid, sig, stat, count, asis, i;
    char  cprompt;
    char  *home;
    char  *argv[MAX_ARGS+1];

    login_sh = 0;
    if (argval[0][0] == '-') login_sh = 1;

    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    if (login_sh) {
	home = getenv("HOME");
	if (!home) putenv("HOME=/");
	chdir("/");
    }

    cprompt = (getuid() == 0) ? '#' : '$';

    for (;;) {
        cmd[0] = (char) 0;
        for (i = 0; i < MAX_ARGS; i++) arg[i][0] = (char) 0;
        do {
            printf("ssh%c ", cprompt);
            fgets(buf, 127, stdin);
            if (feof(stdin)) return 0;     /* user typed ^D ? */
            buf[strlen(buf) - 1] = '\0';   /* Strip newline from fgets */
        }
        while (buf[0] == (char) 0);
        count = sscanf(buf, "%s %s %s %s %s %s %s %s %s %s %s",
                            cmd,
                            arg[0], arg[1], arg[2], arg[3], arg[4],
                            arg[5], arg[6], arg[7], arg[8], arg[9]);

        /* Check for User-Requested Exit back to Login Prompt */
        if (strcmp(cmd, "exit") == 0)
            return (0);                      /* Quit if requested */

        /* Check for User request to change Current Working Directory */
        else if (strcmp(cmd, "cd") == 0) {
            stat = chdir(*arg[0] ? arg[0] : getenv("HOME"));
            if (stat)
                perror("cd");
        }

        else if (strcmp(cmd, "pwd") == 0) {
            if (getcwd(buf, 128))
                printf("%s\n", buf);
            else
                printf("pwd: cannot get current directory\n");
        }
        
        else if (strcmp(cmd, "sync") == 0) {
            sync();
        }
        
        else if (strcmp(cmd, "umask") == 0) {
            if (arg[0][0] == (char) 0) {
                i = umask(0);
                umask(i);
                printf("%03o\n", i);
            } else {
                i = 0;
                tp = arg[0];
                while (*tp >= '0' && *tp <= '7')
                    i = (i << 3) + *tp++ - '0';
                if (*tp || (i & ~0777))
                    printf("umask: bad mask value\n");
                else
                    umask(i);
            }
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
                printf("kill: can't kill process %d\n", pid);
            } else {
                stat = kill(pid, sig);
                if (stat)
                    perror("kill");
            }
        }

        /* Check for environmen variable assignment */
        else if ((tp = strchr(cmd, '=')) != NULL) {
            if (*(tp+1) == '\0') *tp = '\0';
            putenv(cmd);
        }

        /* No built-in Command, Try to find Executable Command File */
        else {
            argv[0] = cmd;                  /* Build Argv Pointer Array */
            for (i = 0; i < MAX_ARGS; ++i)
               argv[i+1] = *arg[i] ? arg[i] : (char *) NULL;

            if ((pid = fork()) == -1) {     /* Try to spawn new Process */
                printf("ssh: can't fork\n");
            } else {
                if (pid == 0) {             /* Child is in context */
                    signal(SIGINT, SIG_DFL);
                    signal(SIGQUIT, SIG_DFL);
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
                        execve(eline, argv, environ);
                    }
                    printf("ssh: %s?\n", buf);      /* Say we can't exec */
                    exit(1);
                }                                   /* Parent is in context */
                wait(0);                    /* Parent waits for completion */
                kill(pid, SIGKILL);         /* then kills child process */
            }
        }
    }
}
