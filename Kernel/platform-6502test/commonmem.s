;
;	Put the udata at the start of common. We have four 16K banks so we
; keep the non .common kernel elements below C000 and then keep bank 3 as a
; true common bank
;
        ; exported symbols
        .export _ub
        .export _udata
        .export kstack_top
        .export istack_top
        .export istack_switched_sp

        .segment "COMMONMEM"

;
;	In 6502 land these are the C stacks, we will need to handle the
;	hardware stack separately, and also to save sp,sp+1 etc on irqs
;
_ub:    ; first 512 bytes: starts with struct u_block, with the kernel stack working down from above
_udata:
kstack_base:
	.res 512,0
kstack_top:

        ; next 256 bytes: 254 byte interrupt stack, then 2 byte saved stack pointer
istack_base:
	.res 254,0
istack_top:
istack_switched_sp: .word 0
