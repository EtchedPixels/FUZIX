
		.code

;
;	FIXME: udata will really be a pointer
;
unix_syscall_entry:
	push ds
	push es

	push bp
	mov bp, sp
	mov bx, cs:_kernel_ds	; Load our DS
	mov ds, bx
	mov es, bx

	; We are now referencing the kernel except for the stack
	; and that has our arguments on it with 6 bytes of trap
	; return info before it and two segment registers we
	; added. bp will reference ss: so we can now stash away our
	; arguments in peace

	mov bx, _udataptr
	mov U_DATA__U_CALLNO[bx], ax
	mov ax, 10[bp]
	mov U_DATA__U_ARGN[bx], ax
	mov ax, 12[bp]
	mov U_DATA__U_ARGN1[bx], ax
	mov ax, 14[bp]
	mov U_DATA__U_ARGN2[bx], ax
	mov ax, 16[bp]
	mov U_DATA__U_ARGN3[bx], ax

	;
	; Time to stack switch (we need SP for brk() even though the
	; entry/exit code has no need of it and can use bp
	;
	mov ax, sp
	mov U_DATA__U_SYSCALL_SP[bx], ax
	; save SS anyway - will be useful if we ever do multi-segment
	; binaries
	mov ax, ss
	mov U_DATA__U_SYSCALL_SS[bx], ax

	;
	; Stack switch. Load sp the instruction after ss. Doesn't matter
	; currently as IRQs are off, but may if this changes
	;
	mov ax, ds
	mov ss, ax
	mov sp, U_DATA__U_KSTACK[bx]

	mov byte _kernel_flag, #1

	sti	
	call _unix_syscall
	cli

	mov byte _kernel_flag, #0
;
;	SIGNALS TODO
;

	;
	; We effective return a 32bit ulong to bcc half of which is return
	; and error
	;
	mov dx, U_DATA__U_ERROR[bx]
	mov ax, U_DATA__U_RETVAL[bx]

	;
	; Stack back
	;
	mov bx, U_DATA__U_SYSCALL_SS[bx]
	mov ss, bx
	mov sp, bp
	pop es
	pop ds
	iret

;
;	AX is the address to jump to
;
_doexec:
	cli
	mov byte _kernel_flag, #0
	mov bx, _udata_ptr
	;
	; Stack the new CS
	;
	mov dx, U_DATA__U_CS[bx], dx
	push dx
	push ax			; 32bit target address
	;
	; Load the initial stack
	;
	mov dx, U_DATA__U_SIP[bx]
	mov sp, dx
	mov dx, U_DATA__U_SS[bx], dx
	mov ss, dx
	;
	; Load the data segment into ES
	;
	mov dx, U_DATA__U_DS[bx], dx
	mov es, dx
	;
	; Just ES and DS left to go
	;
	mov byte U_DATA__U_INSYS, #0
	;
	; And go
	;
	mov ds, dx
	mov ax, #0
	mov bx, ax
	mov cx, ax
	mov dx, ax
	mov si, ax
	mov di, ax
	mov bp, ax
	sti			; will occur after the retf completes
	retf

interrupt_handler:
	/* TODO */
	iret

trap_signal:
	mov bx, _udata_ptr
	mov U_DATA__U_PTAB[bx], ax
	jmp _ssig

outstring:
outstringhex:
outax:
outcharhex:	
	ret

_di:
	pushf
	pop ax
	cli
	ret

_irqrestore:
	push ax
	popf
	ret
	