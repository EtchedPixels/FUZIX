#include <stdio.h>
#include <unistd.h>
#include <pwd.h>

int main(int argc, const char *argv[])
{
    register struct passwd *pw;
    register uid_t uid;

    uid = geteuid();
    pw = getpwuid(uid);
    if (pw) {
	printf("%s\n", pw->pw_name);
	return 0;
    }

    return 1;
}
