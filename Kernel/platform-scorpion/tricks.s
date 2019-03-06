
	.globl switch_bank_nosave

.macro	switch
	call switch_bank_nosave
.endm

	.include "../dev/zx/tricks-big.s"
