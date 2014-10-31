#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[])
{
    time_t now;

    now = time(NULL);
    fputs(ctime(&now), stdout);

    return 0;
}
