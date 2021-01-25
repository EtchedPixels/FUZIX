.section ".header"
.globl _start
_start:
    .word 0x80A8	# Fuzix executable
    .byte 11       # LX106 series
    .byte 1	    # ESP8266
    .byte 0	    # Base page; meaningless
    .byte 0	    # No hints

    # These three are set properly by binman
    .word 0			# code
    .word 0			# data
    .word 0			# bss size

    .byte o - _start   # Run 18 bytes in
    .byte 0			# No size hint
    .byte 0			# No stack hint
    .byte 0			# No ZP hint

    .word __sighandler		# signal handler vector

o:
    # Wipe BSS.

    movi a2, __bss_start
    movi a3, __bss_end
    movi a4, 0
.1:
    bgeu a2, a3, .2
    s32i a4, a2, 0
    addi a2, a2, 4
    j .1
.2:

    # Initialise stdio.

    call0 __stdio_init_vars

    # Fetch argc and argv from the stack and set up the environment
    # pointer.

    l32i a2, sp, 0 # argc
    l32i a3, sp, 4 # argv
    addi a4, sp, 8 # envp
    
    # Store envp.

    movi a5, environ
    s32i a4, a5, 0

    # Call main, returning directly to exit.

    movi a0, exit
    j main
	
.globl environ
.comm environ, 4



