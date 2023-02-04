sector		equ	0xCFF0

		org 0x600

bootstrap:
		clr $FFC7		; Display to 0x200
		clr $FFC8
		ldx #0x0200
		ldb #0xA0
wiper:
		stb ,x+
		cmpx #0x0400
		bne wiper
		pshs cc
		orcc #$50		; IRQs off for this lot
		sta $FFDF		; 64K mode on
		ldx #start
		ldy #0xC000
copy:					; Move the loader somewhere safe
		lda ,x+
		sta ,y+
		cmpx #end
		bne copy
		jmp 0xC000
		
;
;	This code *MUST* remain relocatable
;
;	Load IDE blocks 1+ into memory off an LBA drive. With 512 byte
; sectors we load just under 0xB000 bytes. The first 0x0000-0x8000 bytes are
; our low memory image. 0x8000-0x93FF holds the high image (0xF200-0xFFFF) and
; then the blocks at 0xA000-0xBFFF are used for discard (0x8000-8FFF will be
; trashed before discard is gone by the exec arguments). Having done the
; shuffle we then jump in 64K mode to 0x0400 which is our start vector.
;
; Important cheat. 256 x 512 is 128K, therefore in LBA mode we never need
; to touch anything but the low 8bit count for the LBA, the rest we clear
; and just leave alone.
;
start:
		ldd #'B'*256+'O'
		std 0x0200
		ldd #'O'*256+'T'
		std 0x0202
		ldd #':'*256+' '
		std 0x0204
		tfr s,u			; Save old stack
		lds #$FE00		; Point the stack somewhere clear of
					; our loading
		ldx #$0
		lda #$FF
		tfr a,dp
		tfr x,y			; Load from 0

l0:					; Wait for IDE ready
		leay -1,y
		beq timeout
		lda #$80
		bita <$FF57
		bne l0
		lda #'X'
		sta $0205
		clr <$FF54
		clr <$FF55
		lda #0xE0
		sta <$FF56		; LBA mode
		lda #$01		; Leave block 0 for MBR etc stuff
		sta sector
		ldx #$0000
next_sector:
		ldb #$80
l1:					; Wait busy to clear
		leay -1,y
		beq timeout
		bitb <$FF57
		bne l1

		sta $FF53		; sector number to read
		lda #0x01
		sta $FF52		; Some drives clear this each I/O
		lda #'*'
		sta 0x0205
		lda #0x40
l2:
		leay -1,y
		beq timeout
		bita <$FF57
		beq l2
		lda #$20		; READ
		sta $FF57
		ldb #$08
l3:
		leay -1,y
		beq timeout
		bitb <$FF57		; Wait DRQ
		beq l3
		lda #'+'
		sta 0x0205
		ldy #0
		clra
l4:
		ldb $FF50
		stb ,x+
		ldb $FF58
		stb ,x+
		deca
		bne l4
		cmpx #$B000
		beq loaded
		lda #'-'
		sta $0205
		inc sector
		lda sector
		bra next_sector		; Wait busy clear and do next
timeout:
		ldb #1			; Error 1
bail:
		ldx #0
bail_w:
		leax 1,x
		cmpx #0
		bne bail_w
		clr $FFC6
		clr $FFC9		; Display back where BASIC wants it
		clra
		tfr u,s			; Recover stack
		; FIXME: need to put something below 32K to make this
		; work...
stop:		bra stop
		sta $FFDE		; Back into 32K mode
		puls cc,pc

diskerr:
		ldb #2
		bra bail

loaded:
		lda #'@'
		sta $0205
;
;	We have loaded from 0000 to B000 now relocate bits from 8000-93FF to
;	F200-FFFF, A000-B000 is discard space
;
		ldx #$8000
		ldy #$F000
l5:		ldd ,x++
		std ,y++
		cmpy #$FF00		; skip last 256 bytes (I/O area)
		bne l5
		lda #'.'
		sta $0205
		jmp $0400
end:
