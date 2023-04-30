/*
 *	System probe logic. It's probably easiest to understand what is
 *	going on if you read the PortMap file
 */


__sfr __at 0xA0 pluto1;
__sfr __at 0xA0 pluto2;

__sfr __at 0xB0 avc_addr
__sfr __at 0xB1 avc_data;

__sfr __at 0xB1 ivc_data;
__sfr __at 0xB2 ivc_handshake;

__sfr __at 0xC0 mv256_probe;
__sfr __at 0xC2 mv256_probe2;

static uint8_t next_vector;

static uint8_t probe_pluto(void)
{
    if (pluto1 != 0xFF && pluto2 != 0xFF)
        return 1;
    return 0;
}

static uint8_t probe_mv256(void)
{
    if (mv256_probe == 0xFF)
        return 0;
    /* But is it a second FDC ? */
    /* This is either the sector register or the control 2 register. Sector
       for an FDC is 8bit, mode2 is 4 bit with the high bits 0000 */
    mv256_probe2 = 0x1F;
    if (mv256_probe2 != 0x0F)
        return 1;
    /* Something is there but it seems to be a floppy controller */
    return 0;
}

static uint8_t probe_rtc(void) __naked
{
    /* Read each byte twice to avoid hitting the 'updating' value and check
       the tens of things bits are meaningful time values */
    __asm
        in a, (0x23)
        and 0x0F
        cp 6
        jr c, rtc_1
        in a, (0x23)
        and 0x0F
        cp 6
        jr nc, no_rtc
    rtc_1:
        in a, (0x25)
        and 0x0F
        cp 6
        jr c, rtc_2
        in a, (0x25)
        and 0x0F
        cp 6
        jr nc, no_rtc
    rtc_2:
        in a, (0x27)
        and 0x0F
        cp 6
        jr c, rtc_3
        in a, (0x27)
        and 0x0F
        cp 3
        jr nc, no_rtc
    rtc_3:
        ld l,#1
        ret
no_rtc:
        ld l,#0
        ret
    __endasm;
}

static uint8_t probe_ctc(uint8_t addr) __z88dk_fastcall __naked
{
    __asm
        ld c,l
        ld a,#0x06	; Reset, new constant
        out (c),a
        ld a,#0xFF	; counter is now counting down every 16 clocks
        out (c),a
        ex (sp),hl
        in a,(c)	; first sample
        ex (sp),hl	; 19 clocks
        in b,(c)	; second sample (will be lower)
        ex (sp),hl
        in d,(c)	; third sample (will be lower again)
        ld e,#0x02
        out (c),e	; quiet
        cp d
        jr z, no_ctc
        jr nc, no_ctc
        cp b
        jr z, no_ctc
        jr nc, no_ctc
        ld l,#1
        ret
    no_ctc:
        ld l,#0
        ret
    __endasm;
        
}

/* We assume interrupts are disabled */

static void probe_pio(uint8_t addr) __z88dk_fastcall __naked
{
    /* We do a momentary output mode test, then go back to input
       so that we don't end up bus fighting */
    __asm
        ld c,l
        inc c
        inc c
        ld de,#0x0F4F		; modes
        out (c),d
        dec c
        dec c
        ld a,#0xAA		; data to AA
        out (c),a		; read it back
        in b,(c)
        inc c
        inc c
        out (c),e		; back to input mode
        ld l,#0
        cp b
        ret nz
        inc l
        ret
    __endasm;
}        

__sfr __at 0xB0 avc_crtc;
__sfr __at 0xB2 ivc_ctrl;

static uint8_t probe_avc(void)
{
    /* If port B0 is writeable and changes then it looks like an AVC. The
       other AVC ports clash with the IVC so try to avoid poking them */
    avc_crtc = 5;
    if (avc_crtc != 5)
        return 0;
    avc_crtc = 11;
    if (avc_crtc != 11)
        return 0;
    return 1;
}

static uint8_t probe_ivc(void)
{
    /* We would xpect bit 0 to be low */
    if (ivc_ctrl == 0xFF)
        return 0;
    /* We already check for an AVC */
    return 1;
}

static void install_pio(uint8_t addr)
{
    /* We don't do anything with this yet */
    vector = (vector + 1) & ~1;
    kprintf("Z80 PIO at 0x%X irq %d\n", addr, vector);
    vector += 2;
}

static void probe_install_pio(uint8_t addr)
{
    if (probe_pio(addr))
        install_pio(addr);
}

static uint8_t num_ctc;

static void install_ctc(uint8_t addr)
{
    /* We don't do anything with this yet */
    vector = (vector + 3) & ~3;
    kprintf("Z80 CTC at 0x%X irq %d", addr, vector);
    if (num_ctc++ == 0)
        kputs(" (system timer)");
    kputs("\n");
    vector += 4;
}

static void probe_install_ctc(uint8_t addr)
{
    if (probe_ctc(addr))
        install_ctc(addr);
}

static void install_8250(uint8_t addr)
{
    /* We don't do anything with this yet */
    kprintf("8250 UART at 0x%X\n", addr);
    tty_install(TTY_8250, addr, 0);
}

static void probe_install_6402(uint8_t addr, uint8_t addr2)
{
    if (in(addr2) == 0xFF)
        return;
    /* We don't do anything with this yet */
    kprintf("6402 UART at 0x%X,0x%X\n", addr, addr2);
    tty_install(TTY_6402, addr, addr2);
}

void probe_system(void)
{
    /* First look for an RTC */
    if (probe_rtc()) {
        kprintf("RTC at 0x0020.\n");
        has_rtc_20 = 1;
    }

    if (probe_ctc(0x10))
        has_gemio = 1;

    /* We could check 0x30 for a PIO for the ADC but we don't care */
    if (probe_pio(0x30))
        has_adc = 1;

    /* 0x80 might be an FPU but kernel doesn't care */

    /* 0xA0 might be a pluto */
    if (probe_pluto())
        has_pluto = 1;

    /* 0xC0-D0 might be an MV256 or a second FDC/HDC */
    if (probe_mv256())
        has_mv256 = 1;

    /* Are we a GM813 mainboard ? */
    if (probe_gm813_mmu()) {
        is_gm813 = 1;
        install_8250(0xB8);
        install_pio(0xB4);
        paging_mode = MAP_GM813;
    } else {
        if (probe_map80())
            paging_mode = MAP_MAP80;
    }

    /* Mainboard serial/tape port on a Nascom - should check if nascom .. FIXME */
    if (paging_mode != MAP_GM813) {
        probe_install_6402(0x02,0x01);
        console_set(TTY_NASCOM, 0, 0);
    }

    if (probe_map80_ivc(0xE0)) {
        console_set(TTY_MAP80, 0xE0, 0);
        has_map80_ivc = 1;
    }

    probe_fdc(0xE0);
    if (has_mv256 == 0)
        probe_fdc(0xC0);	/* One day will need to look at Cx as well */
    /* A MAP80 IVC collides with the SASI/SCSI ports */
    if (!has_map80_ivc)
        probe_gm_scsi(0xE0);
    probe_gm_scsi(0xC0);

    if (has_gemio) {
        /* Everything is fitted */
        install_ctc(0x10);
        install_pio(0x14);
        install_pio(0x18);
        install_pio(0x1C);
    } else {
        probe_install_ctc(0x08);
        probe_install_6402(0x11, 0x12);
        probe_install_pio(0x14);
        probe_install_pio(0x18);
        probe_install_pio(0x1C);
        probe_install_ctc(0x0C);
        if (!has_rtc) {
            probe_install_6402(0x21, 0x22);
            probe_install_pio(0x24);
            probe_install_pio(0x28);
            probe_install_pio(0x2C);
        }
    }

    if (probe_avc())
        has_avc = 1;
    else if (probe_ivc()) {
        console_set(TTY_IVC, 0xB0, 0);
        has_ivc = 1;
    }

    if (num_ctc == 0) {
        if (has_avc)
            kprintf("avc: will use vblank as system timer.\n");
        else
            panic("no system timer.\n");
    }
}
