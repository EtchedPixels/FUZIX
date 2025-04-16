#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <tinyide.h>
#include <devtty.h>

/*
 *	Might be worth unifying with the other versions but this one
 *	differs a little
 */
COMMON_MEMORY

void zxcf_read_data(uint8_t *p) __naked
{
    __asm
            pop bc
            pop de
            pop hl
            push hl
            push de
            push bc
            ld a, (_td_raw)
            ld bc, #0x00BF	                    ; setup port number
                                                    ; and count
            or a
            push af
            call nz, map_buffers
            ld a,#0x05
            out (0xfe),a
            ; The ZXCF design is sucky as we have a 16bit port decode
            ld a,#0x40
            
next4:
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            dec a
            jr nz, next4
            ld a,#0x02
            out (0xfe),a
            ld a,#0x40
            
next4_2:
            ini
            inc b
            ini
            inc b
            ini
            inc b
            ini
            inc b
            dec a
            jr nz, next4_2
            ld a,(_vtborder)
            out (0xfe),a
            pop af
            ret z
            jp map_kernel_restore               ; else map kernel then return
    __endasm;
}

void zxcf_write_data(void) __naked
{
    __asm
            pop bc
            pop de
            pop hl
            push hl
            push de
            push bc
            ld a, (_td_raw)
            ld bc, #0x00BF	                    ; setup port number
                                                    ; and count
            or a
            push af
            call z, map_buffers
            ld a,#0x05
            out (0xfe),a
            ; The ZXCF design is sucky as we have a 16bit port decode
            ld a,#0x40
            
w_next4:
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            dec a
            jr nz, w_next4
            ld a,#0x02
            out (0xfe),a
            ld a,#0x40
            
w_next4_2:
            outi
            inc b
            outi
            inc b
            outi
            inc b
            outi
            inc b
            dec a
            jr nz, w_next4_2
            ld a,(_vtborder)
            out (0xfe),a
            pop af
            ret z
            jp map_kernel_restore               ; else map kernel then return
    __endasm;
}
