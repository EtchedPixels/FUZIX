#include <stdio.h>
#include <stdint.h>

#define CONFIG_FONT8X8
#define GENERATOR

#include "../font/font8x8.c"

uint16_t widen(uint8_t n)
{
    unsigned int i;
    uint16_t r = 0;
    /* We don't need performance so do the simple way */
    for (i = 0; i < 8; i++) {
        r <<= 2;
        if (n & 0x80)
            r |= 0x03;
        n <<= 1;
    }
    return r;
}

char *boolstr(uint8_t n)
{
    static char buf[9];
    unsigned int i;
    for (i = 0; i < 8; i++) {
        buf[i] = (n & 0x80) ? '1' : '0';
        n <<= 1;
    }
    buf[8] = 0;
    return buf;
}
        
    
int main(int argc, char *argv[])
{
    const uint8_t *p = fontdata_8x8;
    unsigned int i;

    puts("#include <kernel.h>\n");
    puts("#ifdef CONFIG_FONT_8X8_EXP2\n");
    puts("/* Automatically generated do not edit */\n");
    printf("const uint8_t fontdata_8x8_exp2[%d] = {\n", FONTDATAMAX);
    for (i = 0; i < FONTDATAMAX; i++) {
        uint16_t w = widen(*p);
        if (!(i & 7))
            printf("\t/* Character %u */\n", i >> 3);
        printf("\t0x%02X,0x%02X,\t/* %s */\n", w >> 8, w & 0xFF, boolstr(*p));
        if ((i & 7) == 7)
            putchar('\n');
        p++;
    }
    puts("/* End of font */\n};\n\n#endif");
    return 0;
}
