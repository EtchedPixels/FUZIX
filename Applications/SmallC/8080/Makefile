.SUFFIXES:      .o .c .asm

ASSEMS = bdos.asm bdos1.asm chio8080.asm exit.asm io8080.asm sbrk.asm

.c.asm:
        tscc    $*.c

all:    $(ASSEMS)
