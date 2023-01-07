#include <libgen.h>
#include <string.h>

char *basename(char *p)
{
    char *n = strrchr(p, '/');
    if (n == NULL)
        return p;
    return n + 1;
}
