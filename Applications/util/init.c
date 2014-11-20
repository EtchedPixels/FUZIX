/* init.c - simplified init program for UZI180 (mix of init/getty/login)
 *          only handles logins in /dev/tty1
 *          handles user names from /etc/passwd
 */

#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>

extern char **environ;

char *argp[] = { "sh", NULL };

#define crlf   write(1, "\n", 1)

int  login(char *);
void spawn(struct passwd *);
int  showfile(char *);
void putstr(char *);
void sigalarm(unsigned int);

int main(int argc, char *argv[])
{
    int fdtty1, sh_pid, pid;

    signal(SIGINT, SIG_IGN);

    /* remove any stale /etc/mtab file */

    unlink("/etc/mtab");

    /* loop until we can open the first terminal */

    do {
        fdtty1 = open("/dev/tty1", O_RDWR);
    } while (fdtty1 < 0);

    /* make stdin, stdout and stderr point to /dev/tty1 */

    if (fdtty1 != 0) close(0);
    dup(fdtty1);
    close(1);
    dup(fdtty1);
    close(2);
    dup(fdtty1);

    /* start with a clean environment */
    environ = 0;

    putstr("init version 0.8\n");
    
    /* then call the login procedure on it */

    for (;;) {

        sh_pid = login("/dev/tty1");

        /* wait until the user exits the shell */

        do {
            pid = wait(NULL);
        } while (sh_pid != pid);

        /* then loop to call login again */
        
        crlf;
    }
}

int login(char *ttyname)
{
    int  fdtty, pid;
    struct passwd *pwd;
    char *p, buf[50], salt[3];

    fdtty = open(ttyname, O_RDWR);
    if (fdtty < 0) return -1;

    for (;;) {
        pid = fork();
        if (pid == -1) {
            putstr("init: can't fork\n");
        } else {
            if (pid != 0) {
                close(fdtty);
                /* parent's context: return pid of the child process */
                return pid;
            }
            
            /* here we are inside child's context of execution */
            
            putenv("PATH=:/bin:/usr/bin");
            strcpy(buf, "CTTY=");
            strcat(buf, ttyname);
            putenv(strdup(buf));

            /* make stdin, stdout and stderr point to fdtty */

            close(0);
            dup(fdtty);
            close(1);
            dup(fdtty);
            close(2);
            dup(fdtty);

            /* display the /etc/issue file, if exists */
            showfile("/etc/issue");

            /* loop until a valid user name is entered
             * and a shell is spawned */

            for (;;) {
                putstr("login: ");
                while (read(0, buf, 20) < 0);    /* EINTR might happens because of the alarm() call below */

                if ((p = strchr(buf, '\n')) != NULL)
                    *p = '\0';                    /* strip newline */

                pwd = getpwnam(buf);
                
                if (pwd) {
                    if (pwd->pw_passwd[0] != '\0') {
                        p = getpass("Password: ");
                        salt[0] = pwd->pw_passwd[0];
                        salt[1] = pwd->pw_passwd[1];
                        salt[2] = '\0';
                        p = crypt(p, salt);
                    } else {
                        p = "";
                    }
                    if (strcmp(p, pwd->pw_passwd) == 0) spawn(pwd);
                }

                putstr("Login incorrect\n\n");
                signal(SIGALRM, sigalarm);
                alarm(2);
                pause();
            }
        }
    }
}

void spawn(struct passwd *pwd)
{
    char *p, buf[50];

    setgid(pwd->pw_gid);
    setuid(pwd->pw_uid);
    signal(SIGINT, SIG_DFL);

    /* setup user environment variables */

    strcpy(buf, "LOGNAME=");
    strcat(buf, pwd->pw_name);
    putenv(strdup(buf));

    strcpy(buf, "HOME=");
    strcat(buf, pwd->pw_dir);
    putenv(strdup(buf));

    strcpy(buf, "SHELL=");
    strcat(buf, pwd->pw_shell);
    putenv(strdup(buf));

    /*chdir(pwd->pw_dir);*/

    /* show the motd file */

    if (!showfile("/etc/motd")) crlf;

    /* and spawn the shell */

    strcpy(buf, "-");
    if ((p = strrchr(pwd->pw_shell, '/')) != NULL)
        strcat(buf, ++p);
    else
        strcat(buf, pwd->pw_shell);

    argp[0] = buf;
    argp[1] = NULL;

    execve(pwd->pw_shell, argp, environ);
    putstr("login: can't execute shell\n");
    exit(1);
}

void sigalarm(unsigned int sig)
{
    return;
}

int showfile(char *fname)
{
    int  fd, len;
    char buf[80];
    
    fd = open(fname, O_RDONLY);
    if (fd > 0) {
        do {
            len = read(fd, buf, 80);
            write(1, buf, len);
        } while (len > 0);
        close(fd);
        return 1;
    }
    return 0;
}

void putstr(char *str)
{
    write(1, str, strlen(str));
}
