# 0 "ppide.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "ppide.S"
# 1 "../kernel-8085.def" 1
; Keep these in sync with struct u_data;;

# 1 "../platform/kernel.def" 1
# 4 "../kernel-8085.def" 2
# 29 "../kernel-8085.def"
; Keep these in sync with struct p_tab;;
# 46 "../kernel-8085.def"
; Keep in sync with struct blkbuf


; Currently only used for 8085
# 2 "ppide.S" 2
;
; First draft on an 8085 PPIDE driver.
;
; Does not yet include swap support.
;
# 20 "ppide.S"
 .setcpu 8085

 .code

 .export _devide_readb
;
; Read a byte from an IDE register and hand it back to the C code
;
_devide_readb:
 ldsi 2
 ldax d
 out 0x22
 push b
 mov b,a
 ori 0x40
 out 0x22
 in 0x20
 mov l,a
 mov a,b
 out 0x22
 mvi h,0
 pop b
 ret

 .export _devide_writeb
;
; Write a byte to an IDE register for C code
;
_devide_writeb:
 ldsi 4
 lhlx
 ldsi 2
 push b

 mvi a,0x80
 out 0x23
 ldax d
 out 0x22
 mov a,l
 out 0x20
 mov a,h
 out 0x21
 ldax d
 mov b,a
 ori 0x20
 out 0x22
 mov a,b
 out 0x22
 mvi a,0x92
 out 0x23
 pop b
 ret

 .common

 .export _devide_read_data
;
; Perform the data block transfer when reading. blk_op holds all the
; parameter information we need
;
_devide_read_data:
 push b
 ; Point at the data register
 mvi a,0x08
 out 0x22
 ; Get the destination buffer
 lhld _blk_op + 0
 ; Set d and e up as fast copies of the values we need to out
 mov d,a
 ori 0x40
 mov e,a
 ; If we are doing a user transfer map the process
 ; We don't yet support swap or buffer mappings (FIXME)
 lda _blk_op + 2
 ora a
 push psw
 cnz map_proc_always
 ; 256 words per transfer
 mvi b,0
goread:
 ; Write the register | RD
 mov a,e
 out 0x22
 ; Read the byte back from the interface (no polling - it is faster
 ; than we are)
 in 0x20
 ; Save it
 mov m,a
 inx h
 ; Repeat for second byte
 in 0x21
 mov m,a
 inx h
 ; Drop RD
 mov a,d
 out 0x22
 ; Check if done
 dcr b
 jnz goread
 ; Fix up mappings
 pop psw
 pop b
 rz
 jmp map_kernel

 .export _devide_write_data

_devide_write_data:
 ; Point at the data register
 mvi a,0x08
 out 0x22
 ; Switch the bus direction on the 82C55 for ports A and B
 mvi a,0x80
 out 0x23
 ; Get the source buffer
 lhld _blk_op + 0
 ; Set up d and e as the register values we need to out
 mvi d,0x08
 mvi e,0x08 + 0x20
 ; Map the user space if needed
 ; Need to do swap and buffer mappings properly
 lda _blk_op + 2
 ora a
 push b
 push psw
 cnz map_proc_always
 mvi b,0
gowrite:
 ; Write bit off
 mov a,d
 out 0x22
 ; Now put the data value on the bus (the register is already set)
 mov a,m
 inx h
 out 0x20
 mov a,m
 inx h
 out 0x21
 ; Strobe write with data pending and settled
 mov a,e
 out 0x22
 ; Check if we are done
 dcr b
 jnz gowrite
 ; Drop the write bit
 mov a,d
 out 0x22
 ; Put the bus back in the read direction as all other code expects
 mvi a,0x92
 out 0x23
 ; Recover the mappings
 pop psw
 pop b
 rz
 jmp map_kernel

 .discard

 .export _ppide_init

;
; Initialize PPIDE. This consists of setting up the registers
; for read state
;
_ppide_init:
 mvi a,0x92
 out 0x23
 mvi a,0x0F
 out 0x22
 ret
