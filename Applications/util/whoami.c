#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>

int main(int argc, const char *argv[])
{
    register struct passwd *pw;
    register uid_t uid;

    uid = geteuid();
    pw = getpwuid(uid);
    if (pw) {
	write(1, pw->pw_name, strlen(pw->pw_name));
	write(1, "\n", 1);
	return 0;
    }

    return 1;
}
