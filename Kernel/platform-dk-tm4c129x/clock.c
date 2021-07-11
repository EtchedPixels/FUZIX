/*
 *	Configure the memory and system timing. Hard coded for 120MHz
 */

#include <kernel.h>
#include "tm4c129x.h"

#define SYSCON_MOSCCTL	0x400FE07C
#define		MOSCCTL_OSCRNG		0x10
#define		MOSCCTL_PWRDN		0x08
#define		MOSCCTL_NOXTAL		0x04
#define SYSCON_RSCLKCFG	0x400FE0B0
#define		RSCLKCFG_MEMTIMU	0x80000000
#define		RSCLKCFG_NEWFREQ	0x40000000
#define		RSCLKCFG_MOSCSRC	(3 << 20)
#define		RSCLKCFG_PIOSCSRC	(0 << 20)
#define		RSCLKCFG_MPLLSRC	(3 << 24)
#define		RSCLKCFG_PPLLSRC	(0 << 24)
#define		RSCLKCFG_USEPLL		0x10000000
#define		RSCLKCFG_PSYDIV		0x3FF
#define SYSCON_ALTCLKCFG 0x400FE138
#define		ALTCLKCFG_PIOSC		0
#define		ALTCLKCFG_RTCOSC	0x03
#define		ALTCLKCFG_LFIOSC	0x04
#define	SYSCON_MEMTIM0	0x400FE0C0
#define		MEMTIM0_RESERVED	0xFC10FC10
                        /* EBCE, FBCE 1/2 system clock period, no waits */
#define 	MEM_SLOW	(1 << 21) | (1 << 5)
                        /* 3.5 system clock periods, 5 wait states */
#define		MEM_120MHZ	(6 << 22) | (6 << 9) | (5 << 16) | (5 << 0)
#define SYSCON_PLLFREQ0 0X400FE160
#define 	PLLFREQ0_PLLPWR		0x00800000
#define		PLLFREQ0_RESERVED	0xFF700000
#define SYSCON_PLLFREQ1 0X400FE164
#define		PLLFREQ1_RESERVED	0xFFFFE0E0
#define SYSCON_PLLSTAT	0x400FE168
#define 	PLLSTAT_LOCK		0x00000001

#define PLL0		95	/* INT 96 FRAC 0 */
#define PLL1		4	/* N 5 Q 1 */

/*
 *	See 3.5.5.3 (page 258)
 *
 *	We power up with slow timings on the precision oscillator at 25MHz or so
 */
int sysclock_init(void)
{
    uint32_t r;
    unsigned int loops = 0;

    /* 2. Set up the Main Oscillator by clearing NOXTAL */
    r = inl(SYSCON_MOSCCTL);
    r &= ~(MOSCCTL_PWRDN | MOSCCTL_NOXTAL);
    r |= MOSCCTL_OSCRNG;
    outl(SYSCON_MOSCCTL, r);

    /* Do we need to wait for MOSCPUPRIS in RIS ? */

    /* Slow timing  as will be on the PIOSC */
    r = inl(SYSCON_MEMTIM0);
    r &= ~MEMTIM0_RESERVED;
    r |= MEM_SLOW;
    outl(SYSCON_MEMTIM0, r);

    /* 4. Set the OSCSCRC field */
    r = inl(SYSCON_RSCLKCFG);
    r |= (RSCLKCFG_MOSCSRC | RSCLKCFG_MPLLSRC);
    outl(SYSCON_RSCLKCFG, r);

    /* 6. Load the PLLFREQ registers */
    r = inl(SYSCON_PLLFREQ0);
    r &= PLLFREQ1_RESERVED;
    r |= PLL1;
    outl(SYSCON_PLLFREQ1, r);
    
    r = inl(SYSCON_PLLFREQ0);
    r &= (PLLFREQ0_PLLPWR|PLLFREQ0_RESERVED);
    r |= PLL0;
    outl(SYSCON_PLLFREQ0, r);

    /* 7. Write the new memory timing */
    r = inl(SYSCON_MEMTIM0);
    r &= ~MEMTIM0_RESERVED;
    r |= MEM_120MHZ;
    outl(SYSCON_MEMTIM0, r);

    /* Start the PLL loading - power it up if needed, poke it to relock if
       not */
    if (inl(SYSCON_PLLFREQ0) & PLLFREQ0_PLLPWR)
        outmod32(SYSCON_RSCLKCFG, RSCLKCFG_NEWFREQ, RSCLKCFG_NEWFREQ);
    else
        outmod32(SYSCON_PLLFREQ0, PLLFREQ0_PLLPWR, PLLFREQ0_PLLPWR);

    /* 8. Wait for the PLL to lock */
    while(loops < 50000) {
        r = inl(SYSCON_PLLSTAT);
        if (r & PLLSTAT_LOCK) {
            /* Write the RSCLKCFG PSYSDIV, USEPLL and MEMTIMU */
            /* 9. Switch to MOSC, 120MHz here we come */
            r = inl(SYSCON_RSCLKCFG);
            r |= 3 /* Divide by 4 on PLL */ | RSCLKCFG_MOSCSRC |
                RSCLKCFG_MPLLSRC | RSCLKCFG_USEPLL |
                RSCLKCFG_MEMTIMU;
            outl(SYSCON_RSCLKCFG, r);
            /* And set the alt clock to the precision internal oscillator */
            outl(SYSCON_ALTCLKCFG, ALTCLKCFG_PIOSC);
            return 1;
        }
    }
    /* What to do - not clear, our serial setup will be wonky as a result so
       we can't sanely report it */
    return 0;
}
