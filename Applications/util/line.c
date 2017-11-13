#include <stdio.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char *argv[])
{
    char buf[512];
    char *e;
    int l;
    while((l = read(0, buf, 512)) > 0) {
        e = memchr(buf, '\n', l);
        if (e) {
            l = e + 1 - buf;
            write(1, buf, l);
            return 0;
        }
        write(1, buf, l);
    }
    /* No newline was found */
    write(1, "\n", l);
    return 1;
}

            