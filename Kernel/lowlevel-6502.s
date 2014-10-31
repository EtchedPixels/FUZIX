

	.export unix_syscall_entry
	.export _doexec
	.export interrupt_handler
	.export null_handler
	.export nmi_handler
	.export trap_illegal

	.export outstring
	.export outstringhex
	.export outnewline
	.export outx
	.export outy
	.export outcharhex


unix_syscall_entry:
_doexec:
interrupt_handler:
null_handler:
nmi_handler:
trap_illegal:
outstring:
outstringhex:
outnewline:
outx:
outy:
outcharhex:
	rts
