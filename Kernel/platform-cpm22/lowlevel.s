;
;	Low level support code. This will become the system customisation
;	block in the end.
;
		.module lowlevel

		.globl outchar
		.globl _plt_monitor
		.globl _plt_reboot

		.globl init_early
		.globl init_hardware

		.globl map_kernel
		.globl map_kernel_di
		.globl map_kernel_restore
		.globl map_proc
		.globl map_proc_a
		.globl map_proc_always
		.globl map_proc_always_di
		.globl map_save_kernel
		.globl map_restore

		.globl _program_vectors
		.globl plt_interrupt_all

		.globl _int_disabled

		.globl _cpm_conout
		.globl biosbase
		.globl _udata

		.globl interrupt_handler
		.globl unix_syscall_entry
		.globl nmi_handler
		.globl null_handler

		.globl _init_hardware_c

		.include "kernel.def"
		.include "../kernel-z80.def"

;
; Buffers (we use asm to set this up as we need them in a special segment
; so we can grow to fit the space free)
;
		.globl _bufpool
		.area _BUFFERS

_bufpool:
		.ds BUFSIZE * NBUFS


		.area _CODE

init_early:
		im 1
		ld a,#0x08
		out (20),a
		ret

		.area _COMMONMEM

;
;	Wrap conout for our console output
;
outchar:
		push af
		push bc
		push de
		push hl
		ld c,a
		call _cpm_conout
		pop hl
		pop de
		pop bc
		pop af
		ret

_plt_monitor:
		jp _sysmod_monitor
_plt_reboot:
		jp _sysmod_reboot

init_hardware:
		; Set up the BIOS calling interface
		ld a,(2)
		ld (biosbase + 1),a
		call _sysmod_init
		; Set up the system
		call program_vectors_k
		ld a,#1
		out (27),a
		jp _init_hardware_c

map_proc_a:
		ld (map),a
		jp _sysmod_set_map

map_proc:
		ld a,h
		or l
		jr z, map_kernel
		ld a,(hl)
		jp _sysmod_set_map

map_proc_always:
map_proc_always_di:
		push af
		ld a,(_udata + U_DATA__U_PAGE);
		call map_proc_a
		pop af
		ret
map_kernel:
map_kernel_di:
map_kernel_restore:
		push af
		xor a
		call map_proc_a
		pop af
plt_interrupt_all:
		ret

map_save_kernel:
		push af
		ld a,(map)
		ld (saved_map),a
		xor a
		call _sysmod_set_map
		pop af
		ret

map_restore:
		push af
		ld a,(saved_map)
		ld (map), a
		call _sysmod_set_map
		pop af
		ret

_program_vectors:
		; we are called, with interrupts disabled, by both newproc() and crt0
		; will exit with interrupts off
		di ; just to be sure
		pop de ; temporarily store return address
		pop hl ; function argument -- base page number
		push hl ; put stack back as it was
		push de

		call map_proc

program_vectors_k:
		; now install the interrupt vector
		ld hl, #interrupt_handler
		call _sysmod_irq_handler

		; now install the interrupt vector at 0xFEFF
		ld a,#0xC3		; JP
		; set restart vector for UZI system calls
		ld (0x0030), a   ;  (rst 30h is unix function call vector)
		ld hl, #unix_syscall_entry
		ld (0x0031), hl

		; Set vector for jump to NULL
		ld (0x0000), a   
		ld hl, #null_handler  ;   to Our Trap Handler
		ld (0x0001), hl

		ld hl, #nmi_handler
		call _sysmod_nmi_handler

		jp map_kernel

		.area _COMMONDATA

_int_disabled:
		.byte 1
map:
		.byte 0
saved_map:
		.byte 0

;
;	Functions and data that will be provided by the platform
;	module in the end
;

		.globl _sysmod_init
		.globl _sysmod_irq_handler
		.globl _sysmod_nmi_handler
		.globl _sysmod_set_map
		.globl _sysmod_info
		.globl _sysmod_conost
		.globl _sysmod_auxist
		.globl _sysmod_auxost
		.globl _sysmod_idle
		.globl _sysmod_rtc_secs
		.globl _sysmod_monitor
		.globl _sysmod_reboot
		.globl _sysmod_conconf
		.globl _sysmod_auxconf
		.globl _sysmod_joystick

		.area _SYSMOD
_sysmod_base:
_sysmod_init:
		jp sysmod_init
_sysmod_info:
		jp sysmod_info
_sysmod_set_map:
		jp sysmod_set_map
_sysmod_irq_handler:
		jp sysmod_irq_handler
_sysmod_nmi_handler:
		jp sysmod_nmi_handler
_sysmod_conost:
		jp sysmod_conost
_sysmod_auxist:
		jp sysmod_auxist
_sysmod_auxost:
		jp sysmod_auxost
_sysmod_idle:
		jp sysmod_idle
_sysmod_rtc_secs:
		jp sysmod_rtc_secs
_sysmod_monitor:
		jp sysmod_monitor
_sysmod_reboot:
		jp sysmod_reboot
_sysmod_conconf:
		jp sysmod_conconf
_sysmod_auxconf:
		jp sysmod_auxconf
_sysmod_joystick:
		jp sysmod_joystick

sysmod_init:
		im 1
		ld a,#1
		out (27),a	; timer on
		ret

sysmod_irq_handler:
		ld a,#0xC3
		ld (0x38),a
		ld (0x39),hl
sysmod_idle:
		ret
sysmod_nmi_handler:
		ld a,#0xC3
		ld (0x66),a
		ld (0x67),hl
		ret
sysmod_set_map:
		out (21),a		; set the page to A, 0 = kernel
		ret			; ie default CP/M TPA
sysmod_info:
		ld hl,#sysinfo
		ret
sysmod_rtc_secs:
		xor a
		out (25),a		; RTC register for seconds
		in a,(26)		; Seconds in BCD
		ld h,a
		and #15
		ld l,a			; BCD low bits
		ld a,h
		rra
		rra
		rra
		rra
		and #15
		add a			; high x 2
		ld h,a
		add a			; high x 4
		add a			; high x 8
		add h			; high x 10
		or l			; now decimal
		ld l,a
		ret
;
;	I/O routines we would like that are missing from CP/M 2.2 but in
;	CP/M 3.
;
sysmod_conost:				; Return 0xFF if console output ready
		ld l, #0xff
		ret
sysmod_auxist:
		ld l,#0xff
		ret
sysmod_auxost:
		ld l,#0xff
		ret

sysmod_monitor:				; no monitor so just reboot
sysmod_reboot:
		; if you have no reboot just spin here
		ld a,#1			; reboot
		out (29),a
failed:					; shouldn't get here
		jr failed

sysmod_joystick:
		ld hl,#0		; No sticks
sysmod_conconf:				; Not supported
sysmod_auxconf:
		ret

		
sysinfo:
		.db 8			; mumber of banks
		.db 2			; feature flags (timer)
		.db 10			; interrupt to 1/10ths divider
		.db 0xff		; no swap
		.dw 0xC000		; common start
		.dw 0			; console doesn't support config
		.dw 0			; aux doesn't support config
