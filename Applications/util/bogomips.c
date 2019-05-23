#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#if defined(__m6809__) && defined(__GNUC__)

__attribute__((naked))
void delay(unsigned long r)
{
    asm(
	" ldd 4,s     \n"
	" ldx 2,s     \n"
        "a@ addd #-1  \n"   
	" bne a@      \n"  
	" leax  ,x    \n"
	" beq b@      \n"
	" leax -1,x   \n"   
	" bra a@      \n"   
	"b@ rts       \n"
	);
}
#elif defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_r2k) || defined(__SDCC_ez80_z80)
void delay(unsigned long r)
{
__asm
    pop hl	; return address
    pop de	; low
    pop bc	; high
    
loop:
    dec de
    ld a,d
    or a,e
    jp nz,loop
    ld a,b
    or a,c
    dec bc
    jp nz,loop
    push bc
    push de
    jp (hl)
__endasm;
}
#else
#error "Unsupported platform"
#endif

int main(int argc, char *argv[])
{
    unsigned long loops_per_sec = 1 ;
    clock_t ticks;
    unsigned int cps = sysconf(_SC_CLK_TCK);
    
    printf("Calibrating delay loop... ");
    fflush(stdout);
    
    while (loops_per_sec <<= 1) {
        ticks = clock();
        delay(loops_per_sec);
        ticks = clock() - ticks;
        if (ticks >= cps) {
            unsigned long lps = (loops_per_sec * cps) / ticks;
            printf("ok - %lu.%02lu BogoMips\n",
                lps / 500000UL, (lps / 5000) % 100);
            return 0;
        }
    }
    printf("failed\n");
    return -1;
}
