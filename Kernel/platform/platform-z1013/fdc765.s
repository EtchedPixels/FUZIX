# 0 "fdc765.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "fdc765.S"
# 1 "kernelu.def" 1
; FUZIX mnemonics for memory addresses etc

U_DATA__TOTALSIZE .equ 0x200 ; 256+256 bytes @ 0x0100



PROGBASE .equ 0x6000
PROGLOAD .equ 0x6000

;
; SPI uses the top bit
;
# 2 "fdc765.S" 2
