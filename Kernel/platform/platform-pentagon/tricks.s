; We are in DI so we can poke these directly but must not invoke
; any code outside of common

.macro	switch
	ld bc,#0x7ffd
	or #BANK_BITS
	out (c),a

.endm
	.include "../dev/zx/tricks-big.s"
