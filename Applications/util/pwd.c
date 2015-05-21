#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, const char *argv[])
{
    char buf[512];
    
    if (getcwd(buf, sizeof(buf)) == NULL) {
        write(2, "pwd: cannot get current directory\n", 34);
        return 1;
    }
    write(1, buf, strlen(buf));
    write(1, "\n", 1);
    return 0;
}
