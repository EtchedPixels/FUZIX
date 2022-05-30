# 0 "../dev/ds1302_8085.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "../dev/ds1302_8085.S"
# 1 "../dev/../kernel-8085.def" 1
; Keep these in sync with struct u_data;;

# 1 "../dev/../platform/kernel.def" 1
# 4 "../dev/../kernel-8085.def" 2
# 29 "../dev/../kernel-8085.def"
; Keep these in sync with struct p_tab;;
# 46 "../dev/../kernel-8085.def"
; Keep in sync with struct blkbuf


; Currently only used for 8085
# 2 "../dev/ds1302_8085.S" 2
;
; 8085 version of the DS1302 support for RC2014
;
# 18 "../dev/ds1302_8085.S"
 .setcpu 8085

 .code

; -----------------------------------------------------------------------------
; DS1302 interface
; -----------------------------------------------------------------------------

 .export _ds1302_get_data

_ds1302_get_data:
 in 0x0C ; read input register
        ani 0x01 ; mask off data pin
        mov e, a ; return result in L
 mvi d, 0
        ret

 .export _ds1302_set_driven

_ds1302_set_driven:
 ldsi 2
 lhlx
        mov a, l ; load argument
 ani 1
        lda _rtc_shadow
 jz setreg
        ani 0xFF-0x20 ; 0 - output pin
        jmp writereg
setreg:
        ori 0x20
        jmp writereg

 .export _ds1302_set_data

_ds1302_set_data:
 push b
        lxi b, 0x80 + 0x7F00
        jmp setpin

 .export _ds1302_set_ce

_ds1302_set_ce:
 push b
        lxi b, 0x10 + 0xEF00
        jmp setpin

 .export _ds1302_set_clk

_ds1302_set_clk:
 push b
        lxi b, 0x40 + 0xBF00
 ; Fall through
setpin:
 ldsi 4
 lhlx ; Argument into HL
 mov a, l ; Are we setting or clearing ?
 ora a
 jnz set
 mov c, a ; A is 0 so set C to 0 so clears pin
set:
        lda _rtc_shadow ; load current register contents
        ana b ; unset the pin
        ora c ; set if arg is true
 pop b
writereg:
 out 0x0C ; write out new register contents
        sta _rtc_shadow ; update our shadow copy
        ret
