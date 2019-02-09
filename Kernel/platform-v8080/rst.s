#include "../kernel-8080.def"

.sect .common
!
!	We need this in common because
!	1. We need to use .rst_init to install the hooks in other banks
!	2. User space also shares these rst handlers (why store them
!	repeatedly). That does create an ABI concern but if it becomes a
!	problem we just make user apps override
!
.define .rst_init
.rst_init:
    mvi a, 0xc3     ! jmp <a16>
    sta 0x08
    sta 0x10
    sta 0x18
    lxi h, rst1
    shld 0x09
    lxi h, rst2
    shld 0x11
    lxi h, rst3
    shld 0x19
    ret

    ! de = [bc+const1] (remember bc is the frame pointer)
rst1:
    pop h
    mov a, m
    inx h
    push h

	mov l, a
	ral
	sbb a
	mov h, a

	dad b
    mov e, m
    inx h
    mov d, m
    ret

    ! [bc+const1] = de (remember bc is the frame pointer)
rst2:
    pop h
    mov a, m
    inx h
    push h

	mov l, a
	ral
	sbb a
	mov h, a

	dad b
    mov m, e
    inx h
    mov m, d
    ret

    ! hl = bc+const1
rst3:
    pop h
    mov a, m
    inx h
    push h

    mov l, a
    ral
    sbb a
    mov h, a

    dad b
    ret
    