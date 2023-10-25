/*
 *  (C) Copyright 2018 Alan Cox
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

unsigned int port;
uint8_t port_opt;

#if defined(__8080__) || defined(__8085__)

/* You can't currently run 8080 binaries on the Z80 CPU systems because of
   all the compiler/signal support needed in Kernel. If that changes we need
   to revisit this lot */

extern uint8_t cpu_identify(void);

static uint8_t cpu_vendor;
static uint8_t cpu_id;
static const char *vendor_name[2] = { "Intel", "AMD" };
static const char *cpu_name[3] = { "8080", "9080A", "8085" };
static const int8_t cpu_step = -1;
static const int8_t cpu_MHz = 0;
static const uint8_t cpu_cache = 0;
static const char cpu_fpu[1]="no";
static char *cpu_bugs = "";
static char *cpu_flags = "";
static const uint8_t cpu_vsize = 16;
static uint8_t cpu_psize = 16;
static const char *cpu_pm = NULL;

static void cpu_ident(void)
{
    cpu_id = cpu_identify();
    switch(cpu_id) {
    case 0:
        cpu_vendor = 0;
        break;
    case 1:
        cpu_vendor = 1;
        break;
    case 2:
        cpu_vendor = 0;
        break;
    }
}


/* TODO: ez80 */
#elif defined(__z80__) || defined(__z180__)

#define CPU_Z80_Z80		0
#define CPU_Z80_Z180		1
#define CPU_Z80_Z280		2
#define CPU_Z80_EZ80		3
#define CPU_Z80_U880		4
#define CPU_Z80_EMULATOR	5
#define CPU_Z80_CLONE		6
#define CPU_Z80_T80		7

static const int cpu_vsize = 16;
static const int cpu_psize = 16;
static const int cpu_step = -1;
static uint8_t cpu_vendor;
uint8_t cpu_id;
static int cpu_cache = 0;
static const char cpu_fpu[] = "no";
static const char *cpu_bugs = "";
static const char *cpu_pm = "halt";
int8_t z80_nmos = -1;
static int cpu_MHz;
static const char *cpu_flags = NULL;
uint8_t outibust;

static char *vendor_name[] = {
    "Unknown",
    "Zilog",
    "Clone",
    "Zilog/Hitachi",
    "USSR",
    "Daniel Wallner"
};

static char *cpu_name[] = {
    "Z80",
    "Z180/HD64180",
    "Z280/R800",
    "eZ80",
    "KP18588BM1/T34MB1/U880",
    "Emulator",
    "Clone",
    "T80"
};

/* TODO: Z80N (Spectrum Next T80 variant) */
static void cpu_ident(void)
{
    if (badlibz80_asm()) {
        fprintf(stderr, "Your emulator appears to have a faulty libz80, please upgrade it.\n");
        exit(1);
    }
    ident_asm();
    switch(cpu_id) {
    case CPU_Z80_Z280:	/* FIXME R800.. */
        cpu_cache = 256;
        /* FIXME: check divuw with bit 15 of divisor set for div bug */
        cpu_bugs = "exaf inout";	/* Also maybe some others */
        cpu_vendor = 1;
        /* And on the R800 check for mul bugs */
        break;
    case CPU_Z80_Z180:
        cpu_vendor = 3;
        cpu_pm = "halt iostop sleep systemstop";	/* Not always all of */
        break;
    case CPU_Z80_Z80:
        /* Looks like a real Z80, might be a T80. We don't run the T80 test
           on anything else because stuff with broken R emulation might
           get stuck looping */
        if (badt80_asm()) {
            cpu_vendor = 5;
            cpu_id = CPU_Z80_T80;
            cpu_bugs = "ldflag";
            z80_nmos = 0;
            break;
        }
        cpu_bugs = "otdr";
        cpu_vendor = 1;
        break;
    case CPU_Z80_U880:
        cpu_vendor = 4;
        break;
    case CPU_Z80_EMULATOR:
        break;
    case CPU_Z80_CLONE:
        cpu_id = CPU_Z80_Z80;
        cpu_vendor = 2;
        break;
    }
    if (cpu_id == CPU_Z80_Z80 && outibust) {
        cpu_vendor = 4;
        cpu_id = CPU_Z80_U880;
        cpu_bugs = "iff";
    } else if (z80_nmos == 1)
        cpu_bugs = "iff";
}

#elif defined(__CC65__)

extern uint8_t cpu_identify(void);
extern uint8_t cpu_bcdtest(void);
extern uint8_t cpu_rortest(void);
extern uint8_t cpu_bitop(void);
extern uint8_t cpu_ce02(void);
extern uint8_t cpu_huc(void);
extern uint8_t cpu_740(void);

static uint8_t cpu_vendor;
static uint8_t cpu_id;
static const char *vendor_name[] = { "Unknown", "WDC", "Rockwell/WDC", "Ricoh", "Commodore", "Hudson Soft", "Renesas" };
static const char *cpu_name[] = { "6502", "65C02", "65C816", "2A03", "65CE02", "HuC6820", "740" };
static const int8_t cpu_step = -1;
static const int8_t cpu_MHz = 0;
static const uint8_t cpu_cache = 0;
static const char cpu_fpu[]="no";
static char *cpu_bugs = "";
static char *cpu_flags = "";
static const uint8_t cpu_vsize = 16;
static uint8_t cpu_psize = 16;
static const char *cpu_pm = NULL;

static void cpu_ident(void)
{
    /* NMOS, CMOS or 65C816 ? */
    cpu_id = cpu_identify();
    switch(cpu_id) {
    case 0:
        if (cpu_rortest() == 0x02)
            cpu_bugs = "brk ror jmpff invread rmw";
        else
            cpu_bugs = "brk jmpff invread rmw";
        /* Check if BCD mode works - 0A v 10 */
        /* 2A03: Just in case anyone ever ports Fuzix to a NES */
        if (cpu_bcdtest() != 0x10) {
            cpu_bugs = "brk jmpff invread rmw nobcd";
            cpu_vendor = 3;
            cpu_id = 3;
        }
        cpu_flags = "nmos";
        break;
    case 1:
        /* Check for a 740 first. This is a weird hybrid 6502 and bits of
           65C02 plus oddments processor. It's not 65C02 compatible but is
           6502 compatible */
        if (cpu_740()) {
            cpu_flags = "bcd 740";
            cpu_vendor = 6;
            cpu_id = 6;
        } else if (cpu_huc()) {
            /* HuC 6820 aka PC Engine */
            cpu_flags = "bcd cmos huc";
            cpu_vendor = 5;
        } else if (cpu_bitop()) {
            if (cpu_ce02()) {
                /* MOS 65CE02 */
                cpu_flags = "bcd cmos bitop z";
                cpu_vendor = 4;
                cpu_id = 4;
            } else {
                /* Rockwell or some WDC 65C02 */
                cpu_flags = "bcd cmos bitop";
                cpu_vendor = 2;
                cpu_id = 6;
            }
        } else {
            /* Non Rockwell/WDC, or WDC 65SC02 */
            cpu_flags = "bcd cmos";
        }
        break;
    case 2:
        /* WDC 65C802/816 */
        /* CMD actually published a document for this but I've never seen any
           GS65C816 hardware ? */
        cpu_vendor = 1;
        cpu_flags = "bcd cmos bitop ai16";
        cpu_psize = 24;
        break;
    }
}

#elif defined(__CC68__)

extern uint8_t identify_cpu(void);

static uint8_t cpu_vendor;
static uint8_t cpu_id;
static const char *vendor_name[] = { "Motorola", "Hitachi" };
static const char *cpu_name[] = { "6800", "6801", "68HC11", "6303" };
static const int8_t cpu_step = -1;
static const int8_t cpu_MHz = 0;
static const uint8_t cpu_cache = 0;
static const char cpu_fpu[]="no";
static char *cpu_bugs = "";
static char *cpu_flags = "";
static const uint8_t cpu_vsize = 16;
static uint8_t cpu_psize = 16;
static const char *cpu_pm = NULL;


static void cpu_ident(void)
{
    /* For the moment: need to add 6800/1/303/HC11 tests */
    cpu_id = identify_cpu();
    if (cpu_id == 3)
        cpu_vendor = 1;
}

#elif defined(__SDCC_r2k)
static uint8_t cpu_vendor;
static uint8_t cpu_id;
static const char *vendor_name[] = { "Rabbit" };
/* TODO: R3K etc idents too */
static const char *cpu_name[] = { "2000", "2000A", "2000B", "2000C" };
static const int8_t cpu_step = -1;
static const int8_t cpu_MHz = 0;
static const uint8_t cpu_cache = 0;
static const char cpu_fpu[]="no";
static char *cpu_bugs = "";
static char *cpu_flags = "";
static const uint8_t cpu_vsize = 16;
static uint8_t cpu_psize = 20;
static const char *cpu_pm = NULL;

static void cpu_ident(void)
{
    /* TODO */

}
#elif defined(__65c816__)
/* Strictly speaking this could be a Ricoh 5A22 but we are hardly likely
   to ever run on a SNES ! */
static uint8_t cpu_vendor;
static uint8_t cpu_id;
static const char *vendor_name[] = { "Western Design Center" };
static const char *cpu_name[] = {"65c816" };
static const int8_t cpu_step = -1;
static const int8_t cpu_MHz = 0;
static const uint8_t cpu_cache = 0;
static const char cpu_fpu[]="no";
static char *cpu_bugs = "";
static char *cpu_flags = "";
static const uint8_t cpu_vsize = 16;
static uint8_t cpu_psize = 24;
static const char *cpu_pm = NULL;

static void cpu_ident(void)
{
    /* TODO */

}
#else
#error "unsupported CPU"
#endif

static void do_identify(void)
{
    cpu_ident();
    printf("%-16s: 0\n", "processor");
    printf("%-16s: %s\n", "vendor_id", vendor_name[cpu_vendor]);
    printf("%-16s: %s\n", "model_name", cpu_name[cpu_id]);
    if (cpu_step != -1)
        printf("%-16s: %d\n", "stepping", cpu_step);
    if (cpu_MHz != 0)
        printf("%-16s: %d\n", "cpu MHz", cpu_MHz);
    if (cpu_cache != 0)
        printf("%-16s: %d bytes\n", "cache size", cpu_cache);
    printf("%-16s: %s\n", "fpu", cpu_fpu);
    if (cpu_flags != NULL)
        printf("%-16s: %s\n", "flags", cpu_flags);
    if (cpu_bugs != NULL)
        printf("%-16s: %s\n", "bugs", cpu_bugs);
    /* TODO bogomips */
    printf("%-16s: %d bits physical, %d bits virtual\n", "address sizes",
        cpu_psize, cpu_vsize);
    if (cpu_pm)
        printf("%-16s: %s\n", "power management", cpu_pm);
}

static void usage(void)
{
    fputs("cpuinfo: [-p n]\n", stderr);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int opt;
    
    while((opt = getopt(argc, argv, "p:")) != -1) {
        switch(opt) {
        case 'p':
            errno = 0;
            port = strtoul(optarg, NULL, 0);
            if (errno)
                usage();
            port_opt = 1;
            break;
        default:
            usage();
        }
    }
    if (optind != argc)
        usage();

    do_identify();
    return 0;
}
            
            