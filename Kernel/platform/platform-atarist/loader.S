.mri 1

;
;	The boot loader fits in sector 0/0/1.
;
start:
	bra.s	boot		; requred first 2 bytes
	ascii "Loader"
	word 0
	word 0			; DWord for the serial
	word 2			; BPB not used (512 bytes/sec)
	byte 2			; 2 sec/cluster
	word $100		; 1 sector reserved (intel format)
	byte 1			; 1 extra FAT
	word $100		; entries in root directory
	word $400B		; sectors on disk (intel format)
	byte $0			; more BPB crap we don't need
	word $0
	byte $9			; 9 spt
	word $100		; claim single sided
	word $200		; hidden sectors

	word $0			; execflag
	word $0			; load mode
	word $0			; logical sector
	word $0			; setcnt
	word $0			; ldaaddr
	word $0			; fatbuf

	word $0			; fname
	word $0
	word $0
	word $0
	word $0
	byte $0

	word $0

;
;	We are located somewhere at random. We need to relocate ourself out
;	of the way. For simplicity we locate ourselves up at 192K out of the
;	way of the image we load above us. We always have at least 512K so
;	that is fine.
;
;	The screen may be at $10000-$1FFFF.
;
;	We are only guaranteed that rwabs _floprd and getbpb exist there is
;	nothing else definitely in the ROM
;
boot:
	lea.l run(pc),a0
	move.l #$30000,a1
	move.l a1,a2
	move.w #128,d0
copy:
	move.l (a0)+,(a1)+
	dbra d0,copy
	jmp (a2)

run:
	lea.l	$30000,sp
load:
	clr.w	-(sp)		; Drive A: (FIXME - should work off boot)
	move.w #1,-(sp)		; From sector 1
	move.w #256,-(sp)	; 256 sectors
	move.l #$40000,-(sp)	; read address
	move.w #2,-(sp)		; read, ignore media change
	move.w #4,-(sp)		; rwabs
	trap #13

	; Now run our code
	; All interrupts off while we eat TOS alive
	or.w #$0700,sr

	; Skip the vectors and first 512 bytes of TOS variables in case
	; they are useful
	move.w #$600,a0
	move.l #$40000,a1
	; Relocate 128K
	move.w #$7FFF,d0
install:
	move.l (a1)+,(a0)+
	dbra d0,install

	jmp $600

	; Never returns


;
;	And the checksum goes at $1FE (entire sector summed in motorola
;	word format should be $1234
;
