/* su - become super-user		Author: Patrick van Kleef */
/* Ported to UZI by Hector Peraza */

#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* True if the invoker need not give a password. */
#define privileged()	(getgid() == 0)

static char *shell1 = "/bin/sh";
static char *shell2 = "/usr/bin/sh";
static char *shell3 = "/bin/ssh";

static char USER[20], LOGNAME[25], HOME[100], SHELL[100];

int main(int argc, char *argv[])
{
    register char *name, *password;
    register struct passwd *pwd;
    int  login_shell = 0;
    char *shell, arg0[20];

    if (argc > 1 && strcmp(argv[1], "-") == 0) {
	login_shell = 1;	/* Read .profile */
	argv[1] = argv[0];
	argv++;
	argc--;
    }

    if (argc > 1) {
	if (argv[1][0] == '-') {
	    fprintf(stderr, "Usage: su [-] [user [shell-arguments ...]]\n");
	    exit(1);
	}
	name = argv[1];
	argv[1] = argv[0];
	argv++;
    } else {
	name = "root";
    }

    if ((pwd = getpwnam(name)) == 0) {
	fprintf(stderr, "%s: user %s does not exist\n", argv[0], name);
	exit(1);
    }

    if (!privileged() && 
        strcmp(pwd->pw_passwd, crypt("", pwd->pw_passwd)) != 0) {
	password = getpass("Password: ");
	if (strcmp(pwd->pw_passwd, crypt(password, pwd->pw_passwd))) {
	    fprintf(stderr, "%s: incorrect password\n", argv[0]);
	    exit(2);
	}
    }

    if (login_shell) {
	if ((shell = pwd->pw_shell)[0] == 0) shell = shell1;
    } else {
	if ((shell = getenv("SHELL")) == NULL) shell = shell1;
    }

    if (access(shell, 0) < 0) shell = shell2;
    if (access(shell, 0) < 0) shell = shell3;

    if ((argv[0] = strrchr(shell, '/')) == NULL)
	argv[0] = shell;
    else
	argv[0]++;

    if (login_shell) {
	arg0[0] = '-';
	strncpy(arg0 + 1, argv[0], sizeof(arg0) - 2);
	arg0[sizeof(arg0) - 1] = 0;
	argv[0] = arg0;
	strcpy(USER, "USER=");
	strcpy(USER + 5, name);
	putenv(USER);
	strcpy(LOGNAME, "LOGNAME=");
	strcpy(LOGNAME + 8, name);
	putenv(LOGNAME);
	strcpy(SHELL, "SHELL=");
	strcpy(SHELL + 6, shell);
	putenv(SHELL);
	strcpy(HOME, "HOME=");
	strcpy(HOME + 5, pwd->pw_dir);
	putenv(HOME);
	chdir(pwd->pw_dir);
    }
    setgid(pwd->pw_gid);
    setuid(pwd->pw_uid);
    execve(shell, (const char**)argv, (const char**)environ);

    fprintf(stderr, "%s: no shell\n", argv[0]);
    return (3);
}
