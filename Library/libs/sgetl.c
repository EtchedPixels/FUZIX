#include <stdint.h>
#include <stdio.h>

long sgetl(const char *buffer)
{
    const uint8_t *p = (uint8_t *)buffer;
    uint32_t r;
    
    r = *p++;
    r |= (((uint16_t)*p++) << 8);
    r |= (((uint32_t)*p++) << 16);
    r |= (((uint32_t)*p) << 24);
    
    return (long)r;
}
