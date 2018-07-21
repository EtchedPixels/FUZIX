#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void usage(void)
{
    write(2, "date: [-u]\n", 11);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    time_t t;
    struct tm *tm;
    const char *ts, *z;
    int utc = 0;
    
    if (argc > 1 && strcmp(argv[1], "-u") == 0) {
        utc = 1;
        argc--;
    }
    if (argc != 1)
        usage();

    time(&t);
    if (utc)
        tm = gmtime(&t);
    else
        tm = localtime(&t);
    ts = asctime(tm);
    /* Up to the time zone */
    write(1, ts, 20);
    /* Zone */
    z = tzname[tm->tm_isdst];
    write(1, z, strlen(z));
    /* Year */
    write(1, ts + 19, 6);
    return 0;
}
