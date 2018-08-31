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

unsigned int port;
uint8_t port_opt;

#if defined(__SDCC_z80) || defined(__SDCC_z180)

#define CPU_Z80_Z80		0
#define CPU_Z80_Z180		1
#define CPU_Z80_Z280		2
#define CPU_Z80_EZ80		3
#define CPU_Z80_U880		4
#define CPU_Z80_BM1		5
#define CPU_Z80_EMULATOR	6
#define CPU_Z80_CLONE		7

/* Must come first and be split off (SDCC bug 2771) */

static void ident_asm(void)
{
    __asm
        xor a
        ld e,e			; Seen as .LIL on eZ80
        ld hl, #0
        inc a			; part of long load on eZ80 only
        ld a, #CPU_Z80_EZ80
        jr z, set_id
        ld a,#0x40
        .byte 0xCB,0x37		; from the Z280 data book
        jp p, z280_detected
        xor a
        dec a
        daa
        cp #0xF9
        ld a,#CPU_Z80_Z180
        jr z, set_id
        ld a,(_port_opt)
        or a
        jr z, no_port_idwork
        di
        ld bc,(_port)
        in e,(c)
        ld a,#1
        .db 0xED
        .db 0x71
        /* out (c),0|255 */
        in a,(c)
        out (c),e
        ei
        cp #1
        jr z, emulator_detected
        inc a
        ld (_z80_nmos),a
        ld bc,(_port)
        di
        in e,(c)
        and a
        ld hl,#0x00FF
        outi
        out (c),e
        ei
        ld a, #1
        jr nc, no_port_idwork
        ld (_outibust), a
no_port_idwork:
        ld a,(#0xffff)
        bit 7,(hl)
        push af
        pop bc
        ld a,c
        and #0x28
        jr nz, emulator_detected
        ld a,(#0xfffe)
        bit 7,(hl)
        push af
        pop bc
        bit 5,c
        jr z, emulator_detected
        bit 3,c
        ld a, #CPU_Z80_BM1
        jr z, set_id
        ld bc,#0xffff
        push bc
        pop af
        xor a
        scf
        nop
        push af
        pop bc
        ld a,c
        and #0x28
        cp #0x28
        ld a, #CPU_Z80_Z80
        jr z, set_id
        ld a, #CPU_Z80_CLONE
        jr set_id
emulator_detected:
        ld a,#CPU_Z80_EMULATOR
        jr set_id
        /* TODO: separate Z280 from R800, test the many Z280 bugs */
z280_detected:
        ld a,#CPU_Z80_Z280
set_id:
        ld (_cpu_id),a
    __endasm;
}

static const int cpu_vsize = 16;
static const int cpu_psize = 16;
static const int cpu_step = -1;
static uint8_t cpu_vendor, cpu_id;
static int cpu_cache = 0;
static const char cpu_fpu[] = "no";
static const char *cpu_bugs = "";
static const char *cpu_pm = "halt";
static int8_t z80_nmos = -1;
static int cpu_MHz;
static const char *cpu_flags = NULL;
static uint8_t outibust;

static char *vendor_name[] = {
    "Unknown",
    "Zilog",
    "Clone",
    "Zilog/Hitachi",
    "USSR"
};

static char *cpu_name[] = {
    "Z80",
    "Z180/HD64180",
    "Z280/R800",
    "eZ80",
    "U880",
    "KP18588BM1/T34MB1",
    "Emulator"
};

static void cpu_ident(void)
{
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
        cpu_vendor = 1;
        break;
    case CPU_Z80_U880:
    case CPU_Z80_BM1:
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
    }
    if (z80_nmos == 1)
        cpu_bugs = "iff";
}

#elif defined(__CC65__)

extern uint8_t cpu_identify(void);
extern uint8_t cpu_bcdtest(void);
extern uint8_t cpu_rortest(void);

static uint8_t cpu_vendor;
static uint8_t cpu_id;
static const char *vendor_name[] = { "Unknown", "WDC", "Rockwell/WDC" };
static const char *cpu_name[3] = { "6502", "65C02", "65C816" };
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
        if (cpu_bcdtest() != 0x10)
            cpu_bugs = "brk jmpff invread rmw nobcd";
        cpu_flags = "nmos";
        break;
    case 1:
        /* Q : How to safely check for rockwell bit ops ? */
        /* Could also check here for HuC6820, Renesas 740 I guess ? */
        cpu_flags = "bcd cmos";
        break;
    case 2:
        cpu_vendor = 1;
        cpu_flags = "bcd cmos ai16";
        cpu_psize = 24;
        break;
    }
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
            
            