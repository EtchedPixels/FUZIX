; 2014-12-16 William R Sowerbutts
; N8VEM Mark IV SBC hardware specific code

        .module mark4
        .z180

        ; exported symbols
        .globl init_early
        .globl init_hardware
        .globl outchar
        .globl outcharhex
        .globl platform_interrupt_all
        .globl _trap_monitor

        .globl map_kernel
        .globl map_process_always
        .globl map_save
        .globl map_restore
        ;.globl map_process_only

        ; imported symbols
        .globl z180_init_hardware
        .globl z180_init_early
        .globl _ramsize
        .globl _procmem
        .globl outhl
        .globl outnewline

        .include "kernel.def"
        .include "../cpu-z180/z180.def"
        .include "../kernel.def"

; -----------------------------------------------------------------------------
; Initialisation code
; -----------------------------------------------------------------------------
        .area _DISCARD

init_early:
        jp z180_init_early

init_hardware:
        ; set system RAM size
        ld hl, #RAM_KB
        ld (_ramsize), hl
        ld hl, #(RAM_KB-64)        ; 64K for kernel
        ld (_procmem), hl

        ; enable ASCI interrupts
        in0 a, (ASCI_STAT0)
        or #0x08                ; enable ASCI0 receive interrupts
        out0 (ASCI_STAT0), a
        in0 a, (ASCI_ASEXT0)
        and #0x7f               ; disable RDRF interrupt inhibit
        out0 (ASCI_ASEXT0), a
        in0 a, (ASCI_STAT1)
        or #0x08                ; enable ASCI1 receive interrupts
        out0 (ASCI_STAT1), a
        in0 a, (ASCI_ASEXT1)
        and #0x7f               ; disable RDRF interrupt inhibit
        out0 (ASCI_ASEXT1), a

        jp z180_init_hardware

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xF000 upwards)
; -----------------------------------------------------------------------------
        .area _COMMONMEM

; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
outchar:
        push bc
        ld b, a
        ; wait for transmitter to be idle
ocloop: in0 a, (ASCI_STAT0)
        bit 1, a        ; and 0x02
        jr z, ocloop    ; loop while busy
        ; now output the char to serial port
        ld a, b
        out0 (ASCI_TDR0), a
        pop bc
        ret

platform_interrupt_all:
        ret

map_kernel: ; map the kernel into the low 60K, leaves common memory unchanged
        push af
.if DEBUGBANK
        ld a, #'K'
        call outchar
.endif
        ld a, #(OS_BANK + FIRST_RAM_BANK)
        out0 (MMU_BBR), a
        pop af
        ret

; this "map_process" business makes no sense on mark4 since we'd switch stacks
; and the RET would thus lose its return address. oh damn. I suppose we could
; pop the stack address into hl, then jp (hl) or whatever. let's try and get by
; without it.
;
; map_process: ; if HL=0 call map_kernel, else map the full 64K in bank pointed to by HL
;             ld a, h
;             or l
;             jr z, map_kernel
;             ld a, (hl)
;             out0 (MMU_BBR), a
;             out0 (MMU_CBR), a
;             ret

; map_process_only: ; as map_process, but does not modify common memory
;             ld a, h
;             or l
;             jr z, map_kernel
;             ld a, (hl)
;             out0 (MMU_BBR), a
;             ret

map_process_always: ; map the process into the low 60K based on current common mem (which is unchanged)
        push af
.if DEBUGBANK
        ld a, #'='
        call outchar
.endif
        ld a, (U_DATA__U_PAGE)
        out0 (MMU_BBR), a
.if DEBUGBANK
        call outcharhex
.endif
        ; MMU_CBR is left unchanged
        pop af
        ret

map_save:   ; save the current process/kernel mapping
        push af
        in0 a, (MMU_BBR)
        ld (map_store), a
        pop af
        ret

map_restore: ; restore the saved process/kernel mapping
        push af
.if DEBUGBANK
        ld a, #'-'
        call outchar
.endif
        ld a, (map_store)
        out0 (MMU_BBR), a
.if DEBUGBANK
        call outcharhex
.endif
        pop af
        ret

_trap_monitor:
        di
        call outnewline
        pop hl
        call outhl
        call outnewline
        pop hl
        call outhl
        call outnewline
        pop hl
        call outhl
        call outnewline
        pop hl
        call outhl
        call outnewline
        pop hl
        call outhl
        call outnewline
        halt
        jr _trap_monitor

map_store:  ; storage for map_save/map_restore
        .db 0
