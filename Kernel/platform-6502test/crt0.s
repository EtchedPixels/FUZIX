	        ; Ordering of segments for the linker.
	        ; WRS: Note we list all our segments here, even though
	        ; we don't use them all, because their ordering is set
	        ; when they are first seen.	

        	; imported symbols
	        .export start

	        ; startup code @0
	        .code

start:		
;		orcc #0x10		; interrupts definitely off
;		lds #kstack_top
		; move the common memory where it belongs    
		; we do this dowards, not out of any concern about
		; about overlap (although its correct for this) but because
		; it deals with linker reloc limits nicely
;		ldd #s__INITIALIZER
;		addd #l__COMMONMEM
;		tfr d,x
;		ldd #s__COMMONMEM
;		addd #l__COMMONMEM
;		tfr d,y
		
;copier:		lda ,-x
;		sta ,-y
;		cmpy #s__COMMONMEM
;		bgt copier

;wiper:		ldx #s__DATA
;		ldd #l__DATA
;		clr ,x+
;		subd #1
;		bne wiper

;		jsr init_early
;		jsr init_hardware
;		jsr _fuzix_main
;		orcc #0x10
;stop:		bra stop

