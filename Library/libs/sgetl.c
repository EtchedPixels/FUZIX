#include <stdint.h>
#include <stdio.h>

long sgetl(const char *buffer)
{
    uint8_t p = (uint8_t *)buffer;
    uint32_t r;
    
    r = *p++;
    r |= (*p++ << 8);
    r |= (*p++ << 16);
    r |= (*p << 24);
    
    return (long)r;
}
