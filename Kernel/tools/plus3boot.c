#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
    uint8_t buf[512];
    uint8_t *p = buf;
    uint8_t sum = 0;
    
    if (fread(buf, 512, 1, stdin) != 1) {
        fprintf(stderr, "boot block too short.\n");
        exit(1);
    }
    while(p < buf + 512)
        sum += *p++;
    sum = ~sum;
    sum += 4;
    buf[15] = sum;
    if (fwrite(buf, 512, 1, stdout) != 1) {
        fprintf(stderr, "boot block write error.\n");
        exit(1);
    }
    return 0;
}