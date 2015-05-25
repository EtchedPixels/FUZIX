#include <time.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    time_t now;
    const char *p;

    now = time(NULL);
    p = ctime(&now);
    write(1, p, strlen(p));

    return 0;
}
