#include "../kernel-8080.def"
.sect .text

! Which resets we install are determined by statistical analysis of Star
! Trek. When changing these, make sure to update the i80 table to match.
!     97 call .floadn2
!     41 call .floadn4
!     34 call .fload4
!     28 call .fstoren2

.define .rst_init
.rst_init:
    lxi h, .floadn2
    lxi d, 0x0008
    call copy
    lxi h, .floadn4
    call copy
    lxi h, .fload4
    call copy
    lxi h, .fstoren2
! Copies eight bytes from HL to DE.
copy:
    mvi c, 8
.1:
    mov a, m
    stax d
    inx h
    inr e
    dcr c
    jnz .1
    ret