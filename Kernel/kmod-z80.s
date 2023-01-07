;
;	Module stubs for Z80. This goes at the front of the module as loaded
;	into memory after relocation. It orders all the segments and ensures
;	discard is first.
;
;	We put discard first as we'll load to top and want to discard bottom
;	space.
;
;	We assume the helper will clear the bss.
;
	.area _DISCARD
	.globl _module_init

start:
	jp	_module_init

	.area _HOME
	.area _MODCODE
	.area _CONST
	.area _DATA
	.area _BSS
	.area _INITIALIZED
	.area _INITIALIZER	; sorted out by the mod tools but we pack
				; it on the end
