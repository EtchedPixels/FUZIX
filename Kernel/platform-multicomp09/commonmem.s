;
;	Put the udata at the start of common. We have four 16K banks so we
; keep the non .common kernel elements below C000 and then keep bank 3 as a
; true common bank
;
        .module commonmem

        ; exported symbols
        .globl _ub
        .globl _udata
        .globl kstack_top
        .globl istack_top
        .globl istack_switched_sp
	.globl kcommon_start
	
        .area .udata

;;; first 512 bytes: starts with struct u_block,
;;; with the kernel stack working down from above
_ub:
_udata:
kstack_base:
	zmb 512
kstack_top:

;;; next 256 bytes: 254 byte interrupt stack, then 2 byte saved stack pointer
istack_base:
	zmb 254
istack_top:
istack_switched_sp: .dw 0

;;; This helps _program_vectors know where the kernel common code starts
;;; for copying into userspace.
kcommon_start