
	.module scm
	.area _COMMONMEM
;
;	Small Computer Monitor interface
;
;	The caller has to provide the scm_romcall interface as it is system
;	dependent unfortunately
;
;	Quirks:
;
;	1. The timer functions are not usually real timers but serial idle
;	and as we carefully don't block in serial input can't be used. No
;	big deal. We do expose the idle call for platform_idle to use for
;	other users
;
;	2. The IRQ handler installed has to be different to our system one
;	in RAM as we need to bank the ROM out and back in. If we were to
;	task switch in one that would be hell - but we don't call the ROM
;	from user space and we don't task switch from kernel so we are good
;	for the most part. The from ROM with alt RAM banked case however is
;	not pretty so we need to avoid that which has costs. For now we
;	don't allow interrupts during ROM calls - that fixes #2 here and
;	#2/3 in problems.
;
;	3. The RST 30 vector we use for syscalls. Not a big problem because
;	a) our RAM and ROM vectors differ and b) we could use the API to get
;	the old vector and call it directly.
;
;	Problems:
;
;	1. There is no generic safe way to call it. As it sets up the 0x30
;	vector in ROM and we are run with ROM off, and don't know how to
;	re-install the ROM we can't generically call it. We can sort of work
;	around this with a bootloader that runs with ROM on and saves all
;	the stuff, but a defined call point in high memory that paged in the
;	ROM (if needed) would be nice.
;
;	2. Because the monitor is not documented as being re-entrant we need
;	to carefully avoid re-entry of the tty logic on an IRQ during a monitor
;	call.
;
;	4. There is no API to ask what consoles are present in a generic
;	fashion. Even the selection API doesn't return an error code to help
;	find out, and set_baud_rate doesn't tell us which was invalid so
;	also doesn't help.
;
;	5. There is no generic API for banked memory. Right now it could be
;	- None
;	- 64K/64K split using port 0x30
;	- 64K/64K split using port 0x38
;	- Banked ROM only
;	- Switchable RAM only between 8000-BFFF on a different port.
;
;	We can do this by hardware type reported and it's not clear that
;	anything else would help much because we need to inline the logic
;	in some cases.
;
;	6. There seems to no way to use RTS/CTS, check modem state or
;	set character size, parity and stop bits. There is also no break
;	support.
;
;	Documentation Gaps
;
;	Stack pointer requirements when calling the monitor
;	Re-entrancy
;	Calling with interrupts on or off
;	Calling the monitor from interrupts
;	State of RAM and ROM mapping on entry and exit
;
	.globl _scm_romcall


	.globl _scm_reset
	.globl _scm_conout
	.globl _scm_version
	.globl _scm_set_irq
	.globl _scm_set_nmi
	.globl _scm_poll
	.globl _scm_input
	.globl _scm_output
	.globl _scm_setbaud
	.globl _scm_execute
	.globl _scm_console
	.globl _scm_farget
	.globl scm_farput
	.globl _scm_ramtop
	.globl _scm_set_ramtop

_scm_reset:
	xor a
	ld c,a
	jp _scm_romcall


_scm_conout:
	ld c,#2
	ld a,l
	call _scm_romcall
	ret
_scm_version:
	ld c,#8
	call _scm_romcall
	ld (_scm_monitor_ver),de
	ld (_scm_config_id),bc
	ld (_scm_hw_info),hl
	ld (_scm_monitor_rev),a
	ret
_scm_set_irq:
	ld c,#0x09
	ld a,#0x07
	ex de,hl
	jp _scm_romcall
_scm_set_nmi:
	ld c,#0x09
	xor a
	ex de,hl
	jp _scm_romcall
_scm_input:
	ld e,l
	ld c,#0x10
	call _scm_romcall
	ld l,a
	ld h,#0
	ret nz
	ld hl,#0xFFFF
	ret
_scm_output:
	ld a,l
	ld e,h
	ld c,#0x11
	call _scm_romcall
	ld l,a
	ret
_scm_poll:
	ld c,#0x12
	jp _scm_romcall
_scm_setbaud:
	ld c,#0x21
	ld a,h
	ld e,l
	call _scm_romcall
	ld l,a
	ret
_scm_execute:
	ld c,#0x22
	ex de,hl
	call _scm_romcall
	ld l,a
	ret
_scm_setconsole:
	ld c,#0x0d
	ld a,l
	jp _scm_romcall
_scm_console:
	ld c,#0x27
	call _scm_romcall
	ex de,hl
	ret
_scm_farget:
	ld c,#0x2A
	ex de,hl
	call _scm_romcall
	ld l,a
	ret
; scm_farput can't be C called but we don't need it for that
scm_farput:
	ld c,#0x2B
	jp _scm_romcall
_scm_ramtop:
	ld c,#0x28
	call _scm_romcall
	ex de,hl
	ret
_scm_set_ramtop:
	ld c,#0x29
	ex de,hl
	jp _scm_romcall


	.area _DATA

	.globl _scm_monitor_ver
	.globl _scm_config_id
	.globl _scm_hw_info
	.globl _scm_monitor_rev

_scm_monitor_ver:
	.dw 0
_scm_config_id:
	.dw 0
_scm_hw_info:
	.dw 0
_scm_monitor_rev:
	.db 0
