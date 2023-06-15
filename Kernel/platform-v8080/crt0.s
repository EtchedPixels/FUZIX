# 0 "crt0.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "crt0.S"
# 1 "../kernel-8080.def" 1
; Keep these in sync with struct u_data;;

# 1 "../platform/kernel.def" 1
# 4 "../kernel-8080.def" 2
# 29 "../kernel-8080.def"
; Keep these in sync with struct p_tab;;
# 46 "../kernel-8080.def"
; Keep in sync with struct blkbuf


; Currently only used for 8085
# 2 "crt0.S" 2

 .code

 .export init

init:
        di
 lxi sp,kstack_top

        call init_early

 ; Common is packed in the BSS space

 lxi b,__common_size
 lxi h,__bss
 lxi d,__common

 ; Copy it high
nextbyte:
 mov a,m
 stax d
 inx h
 inx d
 dcx b
 mov a,b
 ora c
 jnz nextbyte

 ; The discard follows the common

 lxi b, __discard_size
 lxi d, __discard
 dad b
 xchg
 dad b
 xchg
 jmp copydown
;
; We copy discard from the top because it will probably overlap
; on an 8080/8085 type system due to the larger code sizes.
;
nextbyted:
 mov a,m
 stax d
 dcx b
copydown:
 dcx h
 dcx d
 mov a,b
 ora c
 jnz nextbyted

 lxi b,__bss_size
 lxi h,__bss
wipe:
 mvi m,0
 inx h
 dcx b
 mov a,b
 ora c
 jnz wipe

        call init_hardware

        call _fuzix_main
        di
stop: hlt
        jmp stop
