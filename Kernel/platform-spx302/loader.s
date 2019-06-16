# 1 "loader.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 32 "<command-line>" 2
# 1 "loader.S"
.mri 1

;
; CP/M 68K loader for Fuzix
;
; For simplicity we make this entirely relocatable
;
; Phase 1 we relocate our tiny helper code to BFF00
; We then use that to write the kernel to $400 (all over CP/M and
; everything but the vectors). Finally we jump to Fuzix
;

;
; Just assemble the CP/M 68K executable header into the binary
;
header:
 word $601A ; simple CP/M 68K binary
 long data-start ; code size
 long $E000 ; data size (patched by the glue tool later ?)
 long 0 ; BSS size
 long 0 ; no symbols
 long 0
 long 0 ; base/start of code
 word 0 ; no relocations

start:
 move.w #$3e,d0 ; CP/M 68K supervisor mode
 trap #2
 move.w #$0700,sr ; Interrupts off
 move.l go(pc),a0
 lea.l $bff00,a1
 move.w #64,d0
loop: move.l (a0)+,(a1)+
 dbra d0,loop
 lea.l data(pc),a0
 move.w $6FFF,d0 ; copy E000 bytes
 jmp $bff00
go:
 move.w #$400,a1
loop2:
 move.l (a0)+,(a1)+
 dbra d0,loop2
 jmp $400
data: ; We append the data to the file
