/* su - become super-user		Author: Patrick van Kleef */
/* Ported to UZI by Hector Peraza */

#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

/* True if the invoker need not give a password. */
#define privileged()	(getuid() == 0)

static char *shell1 = "/bin/sh";
static char *shell2 = "/usr/bin/sh";
static char *shell3 = "/bin/ssh";

static char USER[20], LOGNAME[25], HOME[PATHLEN + 6], SHELL[100];

int main(int argc, char *argv[])
{
    const char *name;
    char *password;
    struct passwd *pwd;
    int  login_shell = 0;
    int  fd;
    char *shell, arg0[20];

    /* Stop people trying funny stuff like running it with handle 2 closed
       and making stderr write to the password file ! */
    fd = open("/dev/null", O_RDONLY);
    if (fd == -1 || fd < 3)
	exit(1);

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
    	/* FIXME: assemble this lot using sbrk ? */
	arg0[0] = '-';
	strncpy(arg0 + 1, argv[0], sizeof(arg0) - 2);
	arg0[sizeof(arg0) - 1] = 0;
	argv[0] = arg0;
	strcpy(USER, "USER=");
	strlcpy(USER + 5, name, sizeof(USER) - 5);
	putenv(USER);
	strcpy(LOGNAME, "LOGNAME=");
	strlcpy(LOGNAME + 8, name, sizeof(LOGNAME) - 8);
	putenv(LOGNAME);
	strcpy(SHELL, "SHELL=");
	strlcpy(SHELL + 6, shell, sizeof(SHELL) - 6);
	putenv(SHELL);
	strcpy(HOME, "HOME=");
	strlcpy(HOME + 5, pwd->pw_dir, sizeof(HOME) - 5);
	putenv(HOME);
	chdir(pwd->pw_dir);
    }
    setgid(pwd->pw_gid);
    setuid(pwd->pw_uid);
    execve(shell, argv, environ);

    fprintf(stderr, "%s: no shell\n", argv[0]);
    return (3);
}
