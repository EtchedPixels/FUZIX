/*
 *	Support code for the AMD9511 in the kernel. This is fairly minimal
 *	as we don't use the FPU ourselves
 *
 *	TODO: Figure out how to wire the status handling so the app picks up the
 *	correct status flags/exceptions on a context switch case - may need stubs helpers
 */

#include <kernel.h>
#include <kdata.h>
#include <fpu.h>
#include <printf.h>

#if defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_ez80_z80)

__sfr __at AMD_CTL fpctl;
__sfr __at AMD_DATA fpdata;

static void do_fpu_save(struct fpu_context *fp) __fastcall
{
__asm
fpwait_save:
        in a,(AMD_CTL)
        rla
        jr c, fpwait_save
        ld bc,#0x1000+AMD_DATA
        inir
__endasm;    
}

static void do_fpu_restore(struct fpu_context *fp) __fastcall
{
__asm
fpwait_restore:
        in a,(AMD_CTL)
        rla
        jr c, fpwait_restore
        ld de,#15
        add hl,de
        ld bc,#0x1000+AMD_DATA
        otdr
__endasm;    
}

void fpu_save(void)
{
    if (fpu_present)
        do_fpu_save(&udata.u_fpu);
}

void fpu_restore(void)
{
    if (fpu_present)
        do_fpu_restore(&udata.u_fpu);
}

uint_fast8_t fpu_detect(void)
{
    uint16_t ct = 1024;
    uint8_t r;
    /* FPU will be idle */
    if (fpctl & 0x80)
        return 0;        
    fpctl = 0x1A;	/* Push PI */
    while(--ct && (fpctl & 0x80));
    if (ct == 0)
        return 0;
    /* We got a response, or something like one */
    if (fpdata != 0x02)
        return 0;
    if (fpdata != 0xC9)
        return 0;
    fpu_present = 1;
    return 1;
}

#else /* Generic */

void fpu_save(void) __fastcall
{
    uint8_t *ptr = fpu->stack;
    if (!fpu_present)
        return;
    while(in(AMD_CTL & 0x80));
    for (i = 0; i <16; i++)
        *ptr++ = in(AMD_DATA);
}

void fpu_restore(void) __fastcall
{
    uint8_t *ptr = fpu->stack + 16;
    if (!fpu_present)
        return;
    while(in(AMD_CTL & 0x80));
    for (i = 0; i <16; i++)
        out(AMD_DATA, *--ptr);
}

uint_fast8_t fpu_detect(void)
{
    uint16_t ct = 0;
    /* FPU will be idle */
    if (fpctl & 0x80)
        return 0;        
    out(AMD_CTL, 0x8A);	/* Push PI */
    while(ct++ < 1024 && (in(AMD_DATA) & 0x80));
    if (ct == 0)
        return 0;
    /* We got a response, or something like one */
    if (in(AMD_DATA) != 0x02)
        return 0;
    if (in(AMD_DATA) != 0xC9)
        return 0;
    fpu_present = 1;
    return 1;
}

#endif
