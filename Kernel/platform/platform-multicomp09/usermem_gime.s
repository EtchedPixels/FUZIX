	.module usermem

;;;
;;;	6809 copy to and from userspace via
;;;     Multicomp09 MMU which imitates a Color Computer 3 GIME mmu
;;;

	include "kernel.def"
        include "../../cpu-6809/kernel09.def"
        include "platform.def"

	;; window xfer threshhold - any user/krn space xtfers
	;; bigger than this will be routed to the banking/windowing
	;; transfers, rather than the original slower routines
WINTR	equ	256		; window xfer threshold

	; exported
	.globl 	__ugetc
	.globl 	__ugetw
	.globl 	__uget
	.globl 	__uputc
	.globl 	__uputw
	.globl 	__uput
	.globl 	__uzero

	; imported
	.globl 	map_proc_always
	.globl 	map_kernel
	.globl _krn_mmu_map

	.area 	.common

__ugetc:
	pshs 	cc	; save IRQ state
	orcc 	#0x10
	jsr 	map_proc_always
	ldb 	,x
	jsr 	map_kernel
	clra
	tfr 	d,x
	puls 	cc,pc	; back and return

__ugetw:
	pshs 	cc
	orcc 	#0x10
	jsr 	map_proc_always
	ldx 	,x
	jsr 	map_kernel
	puls 	cc,pc

__uget:
	pshs	u,y,cc
	orcc	#0x10
	ldd	9,s		; save count ptr
	cmpd	#WINTR		; are we smaller than threshold?
	blo	__uget1		; yes then goto simple tranfer
	std	count	
	ldd	7,s		; save kernel address
	std	krn
	stx	usr		; save kernel ptr
	clr	way		; xfer direction: to kernel
	com	way		; 
	jmp	uxfer
	
__uget1:
	ldu 	7,s	; user address
	ldy 	9,s	; count
ugetl:
	jsr 	map_proc_always
	lda 	,x+
	jsr 	map_kernel
	sta 	,u+
	leay 	-1,y
	bne 	ugetl
	ldx 	#0
	puls 	u,y,cc,pc

__uputc:
	pshs 	cc
	orcc 	#0x10
	ldd 	3,s
	jsr 	map_proc_always
	exg 	d,x
	stb 	,x
	jsr 	map_kernel
	ldx 	#0
	puls 	cc,pc

__uputw:
	pshs 	cc
	orcc 	#0x10
	ldd 	3,s
	jsr 	map_proc_always
	exg 	d,x
	std 	,x
	jsr 	map_kernel
	ldx 	#0
	puls 	cc,pc


__uput:
	pshs	u,y,cc
	orcc	#0x10
	ldd	9,s		; save count
	cmpd	#WINTR		; are we smaller than threshold?
	blo	__uput1		; yes then do old routine
	std	count
	ldd	7,s		; save user address
	std	usr
	stx	krn		; save krnel address
	clr	way		; xfer direction: to userpace
	jmp	uxfer		; go transfer

	
;;;	X = source, user, size on stack
__uput1:
	ldu 	7,s		; user address
	ldy 	9,s		; count
uputl:
	lda 	,x+
	jsr 	map_proc_always
	sta 	,u+
	jsr 	map_kernel
	leay 	-1,y
	bne 	uputl
	ldx 	#0
	puls 	u,y,cc,pc

__uzero:
	pshs	y,cc
	lda 	#0
	ldy 	5,s
	orcc 	#0x10
	jsr 	map_proc_always
uzloop:
	sta 	,x+
	leay 	-1,y
	bne 	uzloop
	jsr 	map_kernel
	ldx 	#0
	puls 	y,cc,pc



icount	.dw	0		; no of bytes to copy without changing mmu
count	.dw	0		; total no of bytes left to copy
krn	.dw	0		; kernel address
usr	.dw	0		; user address
way	.db	0		; which way to xtfer:
				;   0  = to userspace
				;   !0 = to kernel

;;; find max no of byte copyable in 16k bank window
;;;   takes: D = address
;;;   returns: max copyable pushed onto U stack
;;;   modifies: D
max:	anda	#$3f	
	pshu	d
	ldd	#$4000
	subd	,u
	std	,u
	rts
	
;;; find the minimum of two values
;;;   takes: two 16 bit values on U stack
;;;   returns: least of the two popped values on U stack
;;;   modifies: D
min:
	pulu	d
	cmpd	,u
	blo	a@
	rts
a@	std	,u
	rts

;;;  A faster user-kernel space copy
;;;    takes: usr = usr space ptr
;;;    takes: krn = kernel space ptr
;;;    takes: way = copy direction
;;;    takes: count = number of bytes to xfer
;;;
;;; While the simple uput and uget remap mmu each byte of transfer,
;;; this routine pre-calculates how many bytes we can copy without
;;; remapping the mmu, copies those bytes, *then* re-computes the
;;; mmu banking and repeats until all bytes are transfered.
;;;
;;; "borrow" the low 4 kernel MMU mappings; treating them as 2, 16K
;;; windows, map kernel space into the lower and user space into the
;;; upper, then copy from one to the other. At the end of the routine,
;;; restore the kernel MMU mappings from the values stored in
;;; _krn_mmu_map.
uxfer:
	;; make a data stack
	leau	-8,s		; allow 4 levels in S
	;; calc max src
b@	ldd	krn		; calc max byte copyable from source
	bsr	max		; and push it onto data stack
	;; calc max dest
	ldd	usr		; calc max byte copyable from destination
	bsr	max		; and push it onto data stack
	;; compare count to all
	ldd	count		; push total bytes to copy onto data stack
 	pshu	d		;
	bsr	min		; find the minimum of all three
	bsr	min		;
	pulu	d		; get the result from data stack
	std	icount		;
	;; set kernel bank window (at CPU addr 0x0000)
	ldy	#MMUADR		; Y = beginning of mmu.. for storing
	lda	krn		; get kernel ptr
	rola			; rotate top two bits
	rola			; down to the bottom
	rola			;
	lsla			; multiply by two
	anda	#7		; clean off extra bits
	ldx	#_krn_mmu_map	; kernel's mmu ptr.. for reading
	;;
	;;
	;;
	ldb	curr_tr
	orb	#8
	stb	0,y		; MMUADR set mapsel=8
	;;
	ldb	a,x		; get mmu entry
	stb	1,y		; MMUDAT store in mmu
        ;;
	;;
	ldb	curr_tr
        orb     #9
	stb	0,y		; MMUADR set mapsel=9
	;;
        inca                    ; increment index [NAC HACK 2016May07] or a,x+ above and omit this??
	ldb	a,x		; get next mmu entry
	stb	1,y		; MMUDAT store in mmu
	ldd	krn		; get kernel ptr
	anda	#$3f		; get 16k offset
	pshs	u		; save data stack
	tfr	d,u		; U = ptr to window to kernel
	;; set user bank (at CPU addr 0x4000)
	lda	usr		; get userspace ptr
	rola			; rotate top three bits
	rola			; down to the bottom of A
	rola			;
	anda	#3		; mask off junk [NAC HACK 2016May03] why not 7??
	ldx	#U_DATA__U_PAGE	; X = ptr to user page table
	;;
	ldb	curr_tr
        orb     #10
	stb	0,y		; MMUADR set mapsel=10
	;;
	ldb	a,x		; B = page table entry
	stb	1,y		; MMUDAT store in MMU
	incb			; increment for next page no [NAC HACK 2016May03] coz we use page pairs?
	;;
	lda     curr_tr	        ; use A because B is busy this time
	ora     #11
	sta	0,y		; MMUADR set mapsel=11
	;;
	stb	1,y		; MMUDAT store in mmu
	ldd	usr		; get destination
	anda	#$3f		; get 16k offset
	addd	#0x4000		; add window base
	tfr	d,x		; X = ptr to window to user
	;; inner loop
	ldy	icount		; Y = count
	tst	way		; which way are we copying?
	beq	a@		; to userspace no dest/src swapping
	exg	u,x		; flip to copy the other way....
a@	lda	,u+		; get a byte
	sta	,x+		; store it
	leay	-1,y		; bump counter
	bne	a@		; loop if not done
	;; end inner loop
	puls	u		; restore data stack
	;; clean up kernel mmu's for next mapping or returning
	ldx	#MMUADR
	ldy	#_krn_mmu_map
	lda	curr_tr		; Select MMU register associated with
        ora     #8              ; mapsel=8
        ldb     ,y+             ; page from _krn_mmu_map
        std     ,x              ; Write A to MMUADR to set MAPSEL=8, then write B to MMUDAT
        inca                    ; next mapsel
        ldb     ,y+             ; next page from _krn_mmu_map
        std     ,x              ; Write A to MMUADR to set MAPSEL=9, then write B to MMUDAT
        inca                    ; next mapsel
        ldb     ,y+             ; next page from _krn_mmu_map
        std     ,x              ; Write A to MMUADR to set MAPSEL=a, then write B to MMUDAT
        inca                    ; next mapsel
        ldb     ,y+             ; next page from _krn_mmu_map
        std     ,x              ; Write A to MMUADR to set MAPSEL=b, then write B to MMUDAT
	;;
	;; increment out loop variables
	ldd	krn		; add this iteration's byte count
	addd	icount		; from source address
	std	krn		;
	ldd	usr		; add this iteration's byte count
	addd	icount		; from destination address
	std	usr		;
	ldd	count		; subtract this iteration's byte count
	subd	icount		; from total byte to copy
	std	count
	lbne	b@		; if bytes left to copy then repeat
	;; return
	ldx 	#0		; return #0 - success
	puls 	u,y,cc,pc	; return
