	.text

.globl __pro_cpu_entry
__pro_cpu_entry:
	movi sp, stack+256
	j __pro_cpu_main

.comm stack, 256

