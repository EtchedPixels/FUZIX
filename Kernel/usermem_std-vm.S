#include "kernel-vm.def"

;
;	Simple implementation for now. Should be optimized
;

		.common

.export __uputc

__uputc:
	z80call __bytecode
	cpush 1
	loadl8 4
	push		; ptr to value
	loadl8 8
	deref		; target address
	push
	const U_DATA__U_PAGE
	deref
	call setdbank_a
	call bankcopy
	spmod 6
	constuc 0
	exit

.export __uputw

__uputw:
	z80call __bytecode
	cpush 2
	loadl8 4
	push		; ptr to value
	loadl8 8
	deref		; target address
	push
	const U_DATA__U_PAGE
	deref
	call setdbank_a
	call bankcopy
	spmod 6
	constuc 0
	exit
	
.export __ugetc

__ugetc:
	z80call __bytecode
	call map_proc_always
	constuc 0
	call setdbank_a
	cpush 1
	getsp		; copy over the length we pass
	push
	loadl 6
	swap
	push
	call bankcopy
	spmod 4
	pop		; value read
	exit

.export __ugetw

__ugetw:
	z80call __bytecode
	call map_proc_always
	constuc 0
	call setdbank_a
	cpush 2
	getsp		; copy over the length we pass
	push
	loadl 6
	swap
	push
	call bankcopy
	spmod 4
	pop		; value read
	exit

.export __uget

;
;	Stacked arguments are src.w, dst.w, count.w
;
__uget:
	z80call __bytecode
	call map_proc_always
	constuc 0
	call setdbank_a
	loadl8 6		; count
	deref
	push
	loadl8 4		; source
	deref
	push
	loadl8 6		; dest
	deref
	push
	call bankcopy
	spmod 6
	constuc 0 
	exit


.export __uput

__uput:
	z80call __bytecode
	const U_DATA__U_PAGE
	deref
	call setdbank_a
	loadl8 6
	deref
	push
	loadl8 4
	deref
	push
	loadl8 8
	deref
	push
	call bankcopy
	spmod 6
	call map_kernel
	constuc 0 
	exit


.export __uzero

__uzero:
	z80call __bytecode
	call map_proc_always
	loadl8 4	; length
	deref
	cpush 0		; value
	push
	loadl8 6	; ptr
	call memset
	spmod 6
	call map_kernel
	exit