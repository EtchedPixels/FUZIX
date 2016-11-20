#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char *argv[])
{
    static uint16_t rwe[8];
    int r;
    
    r = _select(0, rwe);
    if (r != 0) {
        perror("test 1 r");
        exit(1);
    }
    /* Report what stdin shows */
    rwe[0] = 1;
    rwe[2] = 1;
    rwe[4] = 1;
    rwe[6] = 100;
    r = _select(1, rwe);
    if (r != 0) {
        perror("test 2 r");
        exit(1);
    }
    if (rwe[0] & 1)
        write(1, "R", 1);
    if (rwe[2] & 1)
        write(1, "W", 1);
    if (rwe[4] & 1)
        write(1, "E", 1);
    
    exit(0);
}