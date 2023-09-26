    ; must be loaded on a page boundary (address % 256 == 0) to ensure
    ; the jump table is correctly aligned
    ;
    ; TODO
    ; - Fix up directory mapping a bit more
    ; - Make the code use indirect pointers to the directory buffer etc
    ;   from the scratch space so that we can eventually make it
    ;   multi-threaded for boxes with a large fixed common where we want to
    ;   try and maximise the TPA by having CP/M emulation in the fixed
    ;   common and allocating buffers in that space indexed off the process
    ;

	.code

fcb 	equ	0x005C		; Default CP/M FCB
buff	equ	0x0080		; Default CP/M Buffer
BS	equ	0x08		; ASCII BackSpace
TAB 	equ	0x09		; ASCII Horizontal Tab
LF	equ	0x0A		; ASCII Line Feed
CR	equ	0x0D		; ASCII Carriage Return
ESC	equ	0x1B		; ASCII ESCape Char

;
;	Reserved space items
;
char	equ	0x3F		; Byte storage for Conin/Conout
SysSP	equ	0x40		; System stack pointer
UserSP	equ	0x42		; User process stack
dmaSav	equ	0x44		; Saved DMA pointer
dmaadr	equ	0x46		; DMA address
srchFD	equ	0x48		; Directory search fd
LSeekData equ	0x4A		; 4 bytes
cnt	equ	0x4E		; Count of waiting keys

TCGETS	equ	1		; Fuzix - get tty data
TCSETS	equ	2		; Fuzix - set tty data
TIOCINQ	equ	5		; Fuzix - characters pending

STDIN	equ	0	; file descriptor value of keyboard
STDOUT	equ	1	; file descriptor value of display

	.code
EStart:
;
;	Required image header, filled in by the tool chain and
;	consumed by our loader code.
;
		.word 0x80A9		; Magic number
		.byte 0x01		; 8080 family
		.byte 0x02		; Z80 featureset required
		.byte 0x00		; Relocating

		.byte 0x00		; No hints
		.word 0x0000		; Text size (updated by tools)
		.word 0x0000		; Data size (updated by tools)
		.word 0x0000		; BSS size (updated by tools)
		.byte 16			; Start address
		.byte 0			; Default hint for grab all space
		.byte 0			; Default no stack hint
		.byte 0			; No zero page on Z80

		; 16 bytes in ...

start2:
;
;
; Load the binary of argv[1] at 0x0100 (obliterating runcpm) (passed as fd
; 3)
;
	lhld	0x31			; xxxx is what runcpm leaves at 0x31
	shld	syscall_patch + 1

	lhld	SysSP			; save stack pointer for cold starts
	sphl
	lxi	d,EStart - 0x100	; largest possible binary space
	mov	a,d
	ral
	jnc	load_in_one
	;	Load first chunk
	lxi	d,0x7F00
	push	d
	lxi	d,0x0100
	push	d
	lxi	d,3
	push	d
	mvi	a,7
	call	syscall
	mov	a,h
	ana	l
	cpi	0xFF
	jz	Quit
	; hl is bytes read
	mvi	a,0x7F
	cmp	h
	jz	short_read
	pop	psw
	pop	psw
	pop	psw
	lxi	d,EStart-0x8000
	push	d
	lxi	d,0x8000
	jmp	load_2
load_in_one:
	push	d
	lxi	d,0x0100	; address
load_2:
	push	d
	lxi	d,3		; file handle
	push	d
	mvi	a,7		; read(3, 0x100, size)
	call	syscall
	mov	a,h
	ora	l
	cpi	0xFF
	jz	Quit
	pop	d
	pop	d
	pop	d
short_read:
	lxi	d,3
	push	d
	mvi	a,2
	call	syscall		; close (3)
	pop	d

	lxi	d,dir
	mvi	b,128
	call	ZeroDE		; Clear Directory Buffer

	lxi	h,0
	shld	0x0003		; Clear IOBYTE and Default Drive/User


	jmp	__bios		; Go to Cold Start setup

;==========================================================
;     Resident Portion of Basic Disk Operating System
;==========================================================
; bdos()
; {
__bdos:	jmp	_bdos0
; }

;.....
; BDOS Function Dispatch Table

fcnTbl:	.word	Fcn0		; Warm Boot
	.word	Fcn1		; ConIn
	.word	Fcn2		; ConOut
	.word	Fcn3		; Reader In
	.word	Fcn4		; Punch Out
	.word	Fcn5		; List Output
	.word	Fcn6		; Direct Console IO
	.word	Fcn7		; Get IOBYTE
	.word	Fcn8		; Set IOBYTE
	.word	Fcn9		; WrBuf
	.word	Fcn10		; RdBuf
	.word	Fcn11		; Get Console Status
	.word	Fcn12		; Return Version #
	.word	Fcn13		; Reset Disk Drive
	.word	Fcn14		; Select Disk
	.word	Fcn15		; Open File
	.word	Fcn16		; Close File
	.word	Fcn17		; Search First Occurance
	.word	Fcn18		; Search Next Occurance
	.word	Fcn19		; Delete File
	.word	Fcn20		; Read File
	.word	Fcn21		; Write File
	.word	Fcn22		; Create File
	.word	Fcn23		; Rename File
	.word	Fcn24		; Return Disk Login Vector
	.word	Fcn25		; Return Current Disk
	.word	Fcn26		; Set DMA
	.word	Fcn27		; Get Allocation Map
	.word	Fcn28		; Write Protect Disk
	.word	Fcn29		; Get R/O Vector Address
	.word	Fcn30		; Set File Attributes
	.word	Fcn31		; Get Disk Parameter Table Address
	.word	Fcn32		; Set/Get User Code
	.word	Fcn33		; Read Random
	.word	Fcn34		; Write Random
	.word	Fcn35		; Compute File Size
	.word	Fcn36		; Set Random Record Field in FCB
;	.word	Fcn37		; Reset Multiple Drives
;	.word	null		; (Fcn38 not implemented)
;	.word	Fcn39		; Get Fixed Disk Vector
;	.word	Fcn40		; Write Random
TBLSZ	 equ	TBLSZ-fcnTbl
MAXFCN	 equ	TBLSZ/2

;------------------------------------------------
; bdos0()
; {

_bdos0:	xchg
	shld	_arg
	xchg
	mov	a,c
	sta	_call
	cpi	MAXFCN		; Legal Function?
	mvi	a,0xFF		;  Prepare Error code
	mov	l,a
	rnc			; ..return if Illegal
	
	; Switch stack
	lxi	h,0
	dad	sp
	shld	UserSP
	lhld	SysSP
	sphl
	mvi	b,0		; Fcn # to Word
	lxi	h,_bdosX
	push	h		;  (ret Addr to Stack)
	lxi	h,fcnTbl
	dad	b
	dad	b
	;   Pt to Fcn entry in Table
	mov	a,m
	inx	h
	mov	h,m
	mov	l,a
	pchl			; "Call" Function #

_bdosX:
	xchg			; save HL
	lhld	UserSP
	sphl
	lhld	_arg
	xchg			; Return Orig contents of DE
	lda	_call
	mov	c,a		; Return Orig contents of C
	mov	a,l
	mov	b,h		; Strict compatibility
	ora	a
	ret
; }

;------------------------------------------------
;         case 0: _exit();			/* Warm Boot */

Fcn0:	jmp	WBoot

;------------------------------------------------
;         case 6: if (arg < 0xfe)		/* Direct Console I/O */
;                     goto conout;
;                 else if (arg == 0xfe)
;                     return (ConSt);
;                 else if (ConSt)       /* 0xff */
;                     goto conout;
;                 else return (0);

Fcn6:	mov	a,e		; _arg in DE
	cpi	0xFE		; < 0FE ?
	jc	Fcn2		; ..jump if Write if Yes
	push	psw
	call	BConSt		; Else get Console Status
	pop	psw
	inr	a
	rnz			; ..exit with 0ffh if 0feh
	mov	a,h
	ora	l		; Any char ready?
	rz			; ..exit if Nothing available
			;..else fall thru to Fcn1..
;------------------------------------------------
;         case 1:				/* Console Input */
;         conin:  read (0, &c, 1);
;                 if (c == '\n')
;                     c = '\r';
;                 return (c);

Fcn1:	call	BConIn		; Get Char from Bios
Fcn1A:	mvi	h,0
	cpi	0x0A		; \n?
	mov	l,a		;  (prepare for return
	rnz			; ..return if Not
	mvi	l,0x0D		; Else return CR
	ret

;------------------------------------------------
;         case 3:				/* Reader (Aux) Input */
;         conin:  read (0, &c, 1);
;                 if (c == '\n')
;                     c = '\r';
;                 return (c);

Fcn3:	call	AuxIn		; Get Char from Bios
	jmp	Fcn1A		; ..exit via common code

;------------------------------------------------
;         case 2:				/* Console Output */
;         conout: if (arg == '\r')
;                     return (0);
;                 c = arg;
;                 write (1, &c, 1);
;                 break;

Fcn2:	mov	c,e		; _arg in DE, need char in C
	jmp	BConOu

;------------------------------------------------
;         case 4:				/* Punch (Aux) Output */
;         conout: if (arg == '\r')
;                     return (0);
;                 c = arg;
;                 write (1, &c, 1);
;                 break;

Fcn4:	mov	c,e		; _arg in DE, need char in C
	jmp	AuxOut

;------------------------------------------------
;         case 5: if (arg == '\r')		/* List (Prntr) Output */
;                     return (0);
;                 c = arg;
;                 write (2, &c, 1);
;                 break;

Fcn5:	mov	a,e		; _arg in DE
	cpi	13		; \r?
	rz
	jmp	List		; ..go to Bios

;------------------------------------------------
;         case 9: ptr = (char *)arg;		/* Print '$'-term String */
;                 while (*ptr != '$')
;                 {
;                     if (*ptr != '\r')
;                         write (1, ptr, 1);
;                     ++ptr;
;                 }
;                 break;
				; Enter: DE -> String (arg)
Fcn9:	ldax	d		; Get char
	inx	d		;   pt to Next
	cpi	'$'		; End?
	rz			; ..quit if Yes
	mov	c,a
	push	d
	call	BConOu
	pop	d
	jmp	Fcn9		; ..loop Til done
	
;------------------------------------------------
;         case 10: rdbuf (arg);
;                  break;
; rdbuf (arg)
; char *arg;
; {
;     int nread;

;     nread = read (0, arg+2, *arg & 0xff);

Fcn10:
	push	d
	xchg			; hl - buffer
	mov	e,m		; e - max chars
	inx	h
	inx	h
	mvi	d,0		; d - char cnt
get:	push	h
	push	d
	call	BConIn
	pop	d
	pop	h
	cpi	8
	jz	del
	cpi	0x7F
	jz	del
	cpi	3
	jz	0		; hard restart
	push	h
	push	d
	push	psw
	mov	c,a
	call	BConOu
	pop	psw
	pop	d
	pop	h
	mov	m,a
	cpi	CR
	jz	eol
	mov	a,e
	cmp	d
	jz	eol1
	inx	h
	inr	d
	jmp	get
del:	mov	a,d
	ora	a
	jz	get
	push	h
	push	d
	mvi	c,8
	call	BConOu
	mvi	c,' '
	call	BConOu
	mvi	c,8
	call	BConOu
	pop	d
	pop	h
	dcx	h
	dcr	d
	jmp	get
eol:	mvi	m,0
eol1:	mov	a,d
	pop	d
	inx	d
	stax	d
	lxi	h,0
	ret


;------------------------------------------------
;         case 11: return (ConSt);		/* Get Console Status */

Fcn11:	jmp	BConSt

;------------------------------------------------
;         case 12:				/* Return Version # */

Fcn12:	lxi	h,0x0022	; Say this is CP/M 2.2
	ret

;------------------------------------------------
;         case 13:				/* Reset Disk Drive */
;              SDma(0x80);
;              break;
Fcn13:
	lxi	b,0x80
	jmp	BSDma

;------------------------------------------------
;         case 7:				/* Get IO Byte */
;         case 8: break;			/* Set IO Byte */
;         case 14: break;			/* Select Disk
;         case 25: break;			/* Return Current Disk */
;         case 28: break;			/* Write Protect Disk */
;         case 30: break;			/* Set File Attribytes */
;         case 32: break;			/* Get/Set User Code */
Fcn7:
Fcn8:
Fcn14:
Fcn25:				; 0 = Drive A
Fcn28:
Fcn30:
Fcn32:				; Return User 0
;         default: break;
;     }
;     return (0);

Exit0:	lxi	h,0
	ret

;------------------------------------------------
;         case 15: return (openfile (arg));		/* Open File */
; openfile (blk)
; {
;     desc = open (getname (arg), 2);
				; DE -> arg
Fcn15:	call	Fcn17		; Does this file exist?
	mov	a,h
	ana	l
	inr	a		; File Not Found (-1)?
	rz			; ..return -1 if File doesn't exist

Open1:	call	CkSrch		;  (Close Search File)

;     arg.recno = 0;

	call	ZeroCR
	mvi	d,13
	dad	d
	mvi	m,0x80		; use S1 as file open flag

	jmp	Exit0		;  Return Dir Code for Entry

;.....
; Common File Open Routine.  Used by Read, Write and Search First.
; Enter: DE = File Mode
;	 hl = Ptr to Null-terminated Path String
; Exit : A = 0 if Error, hl = -1
;	     File Descriptor, A <> 0 if Ok

OpenF:	push	d		; Mode
	push	h		;  Path
	mvi	a,1		;   Fuzix Open Fcn #
	call	syscall		;    _open (Path, Mode);
	pop	b		; Clean Stack
	pop	b
	mov	a,h
	ana	l
	inr	a		; FF -> 0
	ret			; ..return (hl=-1/A=0 if Err, hl=fd/A<>0 of Ok)

;------------------------------------------------
;         case 16: return (closefile (arg));		/* Close File */

;     if (close (arg->desc) == -1)

Fcn16:	lhld	_arg
	lxi	d,13
	dad	d
	mvi	m,0		; clear file open flag

	mvi	a,11		; Fuzix sync function #
	call	syscall		; Execute!

	jmp	Exit0		; Return OK

;....
; Close file descriptor

CloseV:	push	d
	mvi	a,2		;  Fuzix Close Fcn #
	call	syscall		;   Execute!
	pop	b		; Clean Stack
	ret

;------------------------------------------------
;         case 17:					/* Search First */

Fcn17:	call	CkSrch		; Ensure Search File closed
	lxi	h,'.'		; Open current directory (.\0)
	shld	RName		;  store name in Secondary work string
	lxi	d,0		; Open Read-Only
	lxi	h,RName
	call	OpenF		;  _open ('.', 0);
	rz			; hl = -1, A = 0 if Can't Open

	shld	srchFD		; Else Ok, Save File Descriptor
	shld	curFil		;   Duplicate for Reading
			;..fall thru to read one entry..
;------------------------------------------------
;         case 18: return (255);			/* Search Next */

;
;	FIXME:
;		We really only need 64bytes of scratch space here
;		32-63 for the FUZIX entry
;		0-31 for the directory entry in CP/M format, but we must
;		return A = 0. We also ought to magic up the second half of
;		the entry but it's not clear how. The rest of the 128 bytes
;		are undefined after this call. Given we can't really magic
;		up the second 16 bytes we may only need 48
;
;
Fcn18:	lhld	dmaadr
	shld	dmaSav		; Save "real" DMA
Fcn18A:	lhld	dir+16
	shld	dmaadr		;  Set DMA for Dir Op'n
	mvi	a,24		;  Getdirent
	lxi	d,32		;  Len of Dir entries
	call	RdWrt0		;   Read an Entry
	jc	Fcn18E		; Error if Carry Set
	ora	a		; Read Ok?
	jz	Fcn18E		; ..Return hl=-1 if EOF
	call	ChkDir		; Else Set Dir to CP/M, Check Match
	ora	a
	jnz	Fcn18A		; ..loop if No Match

	lda	_call
	cpi	15		; Is this a File Open internal Call?
	lxi	h,0		;  (set Success, Index 0)
	jz	Fcn18X		; ..exit now if Yes

	lhld	dmaSav		;  Move Dir Buffer to "real" DMA
	xchg
	lxi	h,dir		; Else
	lxi	b,128
	call	ldir
	lxi	h,0
	jmp	Fcn18X		;  ..exit

Fcn18E:	lxi	h,-1
Fcn18X:	xchg
	lhld	dmaSav		; ; Restore "real" DMA Addr
	shld	dmaadr
	xchg
	ret

;------------------------------------------------
;         case 19: return (delete (arg));		/* Delete File */

Fcn19:	call	CkSrch		; Ensure Search file closed

;     if (unlink (getname (arg)) == -1)
				; DE -> arg
	call	GetNam		;  Parse to String
	push	h		; String
	mvi	a,6		;  FUZIX Unlink Fcn #
	pop	b		; Clean Stack

;         return (255);
;     return (0);

	mov	a,h
	ana	l
	inr	a		; FF->0
	jnz	Exit0		;  return Ok if No

ExitM1:	lxi	h,-1
	ret


;------------------------------------------------
;         case 33:					/* Read File Random */
;	    readrandom (fcb)
;	    {

Fcn33:	call	RWprep		; Prepare File for access
	jz	Exit1
	call	RecCvt
	call	DoRead
	jmp	RWEx

;....
DoRead:
	call	LSeek		; Seek to Offset (128-byte rec in Block)

	call	BRead		; Read 1 Sector

	push	psw
	
;;	LD	DE,(curFil)
;;;;	call	CloseV		; Close the file
	lxi	d,0
	xchg
	shld	curFil
	xchg
	pop	psw

	ret

RecCvt:
	; Record count to EX/CR
	; CR = record & 7F
	; EX = record >> 7
	push	h
	push	d
	lhld	_arg
	lxi	d,33		; Low byte
	dad	d
	mov	a,m
	ani	#0x7F
	dcx	h
	mov	m,a
	inx	h
	inx	h		; Now points back at high byte
	mov	a,m
	add	a
	mov	e,a		; save A a moment
	dcx	h		; get low byte again
	mov	a,m
	add	a
	jnc	NoCarry
	inr	e		; And drop in the upper bit of byte 0
NoCarry:
	mov	a,e
	lhld	_arg
	lxi	d,12
	dad	d
	mov	m,a
	pop	d
	pop	h
	ret


;------------------------------------------------
;         case 20: return (readfile (arg));		/* Read File */
; readfile (arg)
; {
;     nread = read (blk->desc, dmaadr, 128);
				; DE -> arg (FCB)
Fcn20:	call	RWprep		; Prepare file for access
	jz	Exit1

	call	DoRead		; Read 1 Sector

;     arg.recno++;

	call	IncCR		; Bump Current Record #

RWEx:	jc	Exit1		; ..Error if Carry Set

;     if (nread == 0)
;         return (0);

	ora	a		; Good Read?
	jz	Exit0		;   exit w/0 if Yes

;     else  return (1)

	jmp	Exit1

;------------------------------------------------
;         case 34:					/* Write File Random */
;	    writerandom (fcb)
;	    {
;	    /* CAUTION the seek calls MUST be in this order */
;	        _seek (f, (int)(fcb+33) % 128, 0);          /* byte  seek */
;		_seek (f, (int)(fcb+33) / 128, 3);          /* block seek */

Fcn34:	call	RWprep		; Prepare file for access
	jz	Exit1
	call	RecCvt
	call	DoWrite
	jmp	RWEx

;....
DoWrite:
	call	LSeek		; Seek to Offset (128-byte rec in Block)

	call	BWrit		; Write 1 Sector

	push	psw
;;;;	LD	DE,(curFil)
;;;;	call	CloseV		; Close the file
	lxi	d,0
	xchg
	shld	curFil
	xchg
	pop	psw

	ret

;------------------------------------------------
;         case 21: return (writefile (arg));		/* Write File */
; writefile (arg)
; {
;     if (write (blk->desc, dmaadr, 128) != 128)

				; DE -> arg (FCB)
Fcn21:	call	RWprep		; Prepare file for access
	jz	Exit1

Fcn21A:	call	DoWrite		;   Write

;     arg.recno++;

	call	IncCR		; Bump Current Record #

;         return (255);
;     return (0);

	jmp	RWEx		; ..exit via Common R/W Code
; }

;------------------------------------------------
;         case 22: return (makefile (arg));		/* Create File */
; makefile (arg)
; {
;     desc = creat (getname (blk), 0666);

Fcn22:	call	CkSrch		; Ensure Search file closed
	lxi	h,0x1B6		; Own/Grp/Oth are Read/Execute
	push	h		; DE -> arg
	lxi	h,0x502		; O_CREAT|O_RDWR|O_TRUNC
	call	GetNam		;  This name string
	push	h
	mvi	a,1		;   Fuzix open Fcn #
	call	syscall		;    Execute!
	pop	b		; Clean Stack
	pop	b
	pop	b

;     if (desc == -1)

	mov	a,h
	ana	l
	inr	a		; FF -> 0?

;         return (255);

	rz			; ..return -1 if Yes

;     arg.recno = 0;

	xchg
	call	CloseV
	jmp	Open1

;------------------------------------------------
;         case 23: return (rename (arg));		/* Rename File */
; rename (arg)
; {
;     RName = getname (arg);
;
;	FIXME: should use rename() syscall now
;
Fcn23:	call	CkSrch		; Ensure Search file closed
	push	d		; Save FCB Ptr
	call	GetNam		;  parse to UZI String

	lxi	h,FName
	lxi	h,RName
	lxi	b,12
	call	ldir		; Copy to Rename string

;     FName = getname (arg+16);

	pop	d		; DE -> _arg
	lxi	h,16
	dad	d		; Offset to New Name
	xchg
	call	GetNam		;  parse it returning hl -> FName

;     if (link (RName, FName) < 0) {

	push	h		; New Name
	lxi	h,RName		;  Old Name
	push	h
	mvi	a,5		;   FUZIX link Fcn #
	call	syscall		;    Execute!
	pop	b		; Clean Stack
	pop	b

;         return (-1);

	jc	ExitM1		; Exit w/Err if Bad
;     }
;     if (unlink (RName) < 0) {

	lxi	h,RName		; Old Name
	push	h
	mvi	a,6		;  UZI unlink Fcn #
	call	syscall		;   Execute!
	pop	b		; Clean Stack
	jnc	Exit0		;   exit w/0 if Ok

;         unlink (FName);
				; Else remove the new iNode
	lxi	h,FName		; New Name
	push	h
	mvi	a,6		;  UZI unlink Fcn #
	call	syscall		;   Execute!
	pop	b		; Clean Stack

;         return (-1);

	jc	ExitM1		;  return -1 if Bad
;     }
;     return (0);

	jmp	Exit0		;   else return Ok
; }

;------------------------------------------------
;         case 24: return (1);			/* Return Disk Login Vector */

Fcn24:
Exit1:	lxi	h,1
	ret

;------------------------------------------------
;         case 26: dmaadr = (char *)arg;		/* Set DMA Address */
;                  break;
				; Enter DE = DMA Address
Fcn26:	mov	c,e
	mov	b,d		; Move to Bios Regs
	jmp	BSDma		;  Set in Bios & return

;------------------------------------------------
;         case 27: return (-1)			/* Get Allocation Map */
;         case 29: return (-1)			/* Get R/O Vector Address */
Fcn27:
Fcn29:	lxi	h,-1
	ret

;------------------------------------------------
;         case 31: return (&dpb);		/* Get Disk Param Table Addr */

Fcn31:	lxi	h,dpb
	ret
; }

;------------------------------------------------
;         case 35:				/* Return File Size in FCB */
;				   /* Use stat fcn, rounding up to mod-128 */
;	    if (_stat (dname, &statbuf) == 0) {
				; DE -> fcb
Fcn35:	call	CkSrch		; Ensure Search file closed
	call	GetNam		;  parse to UZI String
	lxi	d,stBuf
	push	d		; &statbuf
	push	h		;  dname
	mvi	a,15		;   FUZIX stat Fcn #
	call	syscall		;    Execute!
	pop	b		; Clean Stk
	pop	b
	mov	a,h
	ora	l		; 0?
	jnz	Fcn35X		; ..jump if Bad

;			      >> 7;
;	FIXME: offset is now a simple number so this is wrong
;

	mvi	d,0
	

	lhld	stBuf+15	; low byte blockc count
	xra	a
	dad	h		; 128 byte block count
	aci	0
	mov	e,a		; EHL is now the count ignoring the
				; lowest block bit
	lda	stBuf+14
	ral			; top bit into carry
	jnc	notodd
	inx	h
notodd:				; EHL is now the block count
	mov	a,e
	xchg			; ADE is now the blocks

	lhld	_arg
	push	b
	lxi	b,33
	dad	b
	pop	b
	mov	m,e
	inx	h
	mov	m,d	;  Store in RR fields in FCB
	inx	h
	mov	m,a	; (D = 0)
;		return (0);

	lxi	h,0
	ret

;	    else {
;		(int)fcb+33 = 0;

Fcn35X:	lhld	_arg
	xra	a
	lxi	d,33
	dad	d
	mov	m,a
	inx	h
	mov	m,a
	inx	h
	mov	m,a

;		return (-1);

	lxi	h,-1
;	    }
	ret

;------------------------------------------------
;         case 36:			/* Set Random Record Field in FCB */

Fcn36:	lhld	_arg
	lxi	d,32
	dad	d
	mov	a,m		; Fetch RecNo
	inx	h
	mov	m,a		;  place in LSB of RR field (r0)
	push	h
	lhld	_arg
	lxi	d,12
	dad	d
	mov	a,m
	pop	h		; + 33
	inx	h		; + 34
	mov	m,a		;  set (r1)
	inx	h
	mvi	m,0		;  Clear Hi byte of RR (r2)

	lxi	h,0		; Return Ok
	ret

;===========================================================
;		  BDos Support Routines
;===========================================================
; char *
; getname (struct fcb *blk)
; {
;     int j;
;     static char name[16];
;     char *p;

;     p = name;
				; Enter: DE -> FCB drive byte
GetNam:	
	xchg
	push	h		;   (save)
	inx	h		;  adv to 1st char of FN
	push	d		; save old HL
	lxi	d,FName		; Dest to string

;     for (j = 0; j < 8; ++j)
;     {
;         if (!blk->name[j] || blk->name[j] == ' ')
;             break;

	mvi	b,8
GetN0:	mov	a,m
	inx	h
	ora	a
	jz	GetN1
	cpi	' '
	jz	GetN1

;         *p++ = chlower (blk->name[j]);

	call	ChLower
	stax	d
	inx	d
	dcr	b
	jnz	GetN0
;     }

GetN1:	pop	h
	lxi	b,9
	dad	b	; Pt to 1st char of FT
	mov	a,m
	cpi	' '		; Any Type?
	jz	GetNX		; ..quit if Not

;     *p++ = '.';

	xchg
	mvi	m,'.'
	xchg
	inx	d

;     for (j = 0; j < 3; ++j)

	mvi	b,3

;     {
;         if (!blk->ext[j] || blk->ext[j] == ' ')
;             break;

GetN2:	mov	a,m
	inx	h
	cpi	' '
	jz	GetNX

;         *p++ = chlower (blk->ext[j]);

	call	ChLower
	stax	d
	inx	d
	dcr	b
	jnz	GetN2

;     }
;     *p = '\0';

GetNX:	xra	a
	stax	d

;     return (name);

	lxi	h,FName
	ret
; }

;
;	We use lseek for Fuzix
;
LSeek:	lxi	b,0		; Push 0 for absolute
	push	b
	push	d
	lhld	_arg		; FCB
	lxi	d,12		; This is in records (128 bytes)
	dad	d
	mov	a,m		; Low 7 bits
	lxi	d,20
	dad	d
	add	a		; Double and lose high bit
	mov	e,a
	mov	a,m		; Pull the offset out of the FCB
				; And may overflow 2^16 bytes
	ora	a
	rar
	mov	d,a		; High bits
	mov	a,e		; Get the low ones back
	rar			; Was shifted, shift back and pick up bit
	mov	e,a

	; Shift ADE
	xra	a
	xchg
	dad	h		; x 2
	ral
	dad	h		; x 4
	ral
	dad	h		; x 8
	ral
	dad	h		; x 16
	ral
	dad	h		; x 32
	ral
	dad	h		; x 64
	ral
	dad	h		; x 128
	ral
	
	sta	LSeekData + 2	; high bit (top byte already clear)
	shld	LSeekData
	xchg
	pop	d
	lxi	b, LSeekData	; push pointer
	push	b
	lhld	curFil
	push	h
	mvi	a,9		; _lseek()
	call	syscall		; Syscall
	pop	b		; Recover stack
	pop	b
	pop	b
	ret

;.....
; Perform File Access Preparatory actions:
; Open file for R/W and Seek to current Record #
; Enter: DE = Ptr to FCB
; Exit : A = 0 and hl = -1 if Error, A <> 0 if Ok

RWprep:	call	CkSrch		; Ensure Search file closed
	lxi	h,13		; offset to S1 (file open flag)
	dad	d
	mov	a,m
	ani	0x80
	lxi	h,-1
	rz

	call	GetNam		;  Parse FCB Fn.Ft to String

	call	COpen
	rz			; ..return -1 on error

	shld	curFil		; store file descriptor for Bios
	ret

COpen:	push	h
	lxi	d,CName
chk:	ldax	d
	cmp	m		; compare filename with cached name
	jnz	differ
	ora	a
	jz	same
	inx	h
	inx	d
	jmp	chk
same:	pop	d		; if same, just return the cached file descr
	lhld	Cfd
	mov	a,h
	ana	l
	inr	a
	rnz
	xchg
	jmp	op1
differ:	lhld	Cfd
	mov	a,h
	ana	l
	inr	a
	xchg
	cnz	CloseV		; close old file
	pop	h		; restore file name
	call	Ccopy
op1:	lxi	d,2		; open for R/W
	call	OpenF
	shld	Cfd
	ret

Ccopy:	push	h
	lxi	d,CName
cpy:	mov	a,m
	stax	d
	inx	h
	inx	d
	ora	a
	jz	cpy
	pop	h
	ret

;.....
; Convert FUZIX Directory Entry at dir+16 to CP/M FCB entry at dir, Zero rest.
; Ambiguously compare FCB FN.FT at dir to that passed at arg, returning Zero
; if Match, Non-Zero if mismatch.

ChkDir:	lxi	d,dir
	lxi	h,dir+16+2	; Pt to 1st char of Name
	xra	a
	stax	d		; Zero Drive field
	inx	d		;  Pt to 1st char of FN
	mvi	b,8
	call	ChkD0		;   Fix Name
	mvi	b,3
	call	ChkD0		;    & Type
	mvi	b,21
	call	ZeroDE		;     Clear rest of Dir entry

	xchg
	lhld	_arg
	xchg
	inx	d		; Pt to 1st char of FN
	ldax	d
	cpi	' '		; Any Name present?
	jz	ChkFN0		; ..jump if Yes
	lxi	h,8
	dad	d		;  Else offset to 1st char of FT
	mov	a,m
	cpi	' '		;   Type present?
	mvi	a,0x0FF		;    (Assume Error)
	rz			;     Return w/Err Flag if no Type either

ChkFN0:	lxi	h,dir+1	; Else Compare name/type fields
	mvi	b,11
			; Ambiguous FN.FT compare of (hl) to (DE)
ChkL:	ldax	d
	cpi	'?'		; Accept anything?
	jz	ChkL0		; ..jump if ambiguous
	xra	m
	ani	0x7F		; Match?
	rnz			; .Return Non-Zero if Not
ChkL0:	inx	h
	inx	d
	dcr	b
	jnz	ChkL		; ..loop til Done
	xra	a		;    return Zero for Match
	ret

;.....
; Parse FileSpec addressed by hl into FN.FT Spec addressed by DE.

ChkD0:	mov	a,m		; Get Char
	cpi	'a'
	jc	ChkD1
	cpi	'z'+1
	jnc	ChkD1
	ani	0x5F		; Convert to Uppercase
ChkD1:	ora	a		; End of String?
	jz	ChkDE		; ..jump if End
	inx	h		;     (bump Inp Ptr if Not End)
	cpi	'.'
	jz	ChkDE		;  ..or Period field separator
	stax	d		; Store char
	inx	d		;  bump Dest
	dcr	b
	jnz	ChkD0		; ..loop til field done
ChkD2:	mov	a,m		; Get Next
	ora	a
	rz			;  Exit at End of string
	inx	h		;   (adv to next)
	cpi	'.'
	rz			;   or field separator
	jmp	ChkD2		;  ..loop til end found

ChkDE:	mvi	a,' '		; Fill rest w/Spaces
ChkD3:	inr	b
	dcr	b		; More in field?
	rz			; ..exit if Not
	jmp	ZeroL		;  ..else stuff spaces til field ends

;.....
; Zero area addressed by DE for B Bytes.  Uses A,B,DE.

ZeroDE:	xra	a
ZeroL:	stax	d
	inx	d
	dcr	b
	jnz	ZeroL
	ret

;-----
; Copy BC bytes from HL to DE, equivalent to Z80 LDIR
;
LDIR:	push	psw
LDIRn:	mov	a,m
	stax	d
	inx	h
	inx	d
	dcx	b
	mov	a,b
	ora	c
	jnz	LDIRn
	pop	psw
	ret
;.....
; Close the Directory if we just exitted a SearchF/SearchN sequence

CkSrch:	push	d		; Save Regs
	push	h
	xchg
	lhld	srchFD		; Get File Desc
	xchg
	mov	a,d
	ora	e		; Anything open?
	cnz	CloseV		;  Close file if Yes
	lxi	h,0
	shld	srchFD		;   Mark as closed
	pop	h		;    (ignore Errors)
	pop	d
	ret

;.....
; Bump current Record # for sequential R/W operations

IncCR:	push	h
	push	d
	push	psw
	lhld	_arg
	lxi	d,32
	dad	d
	inr	m
	mov	a,m
	ral
	jnc	nocarry
	mvi	m,0
	lxi	d,-20
	dad	d
	inr	m
nocarry:
	pop	psw
	pop	d
	pop	h
	; TODO Check no caller relies on this setting up IY
	ret	

;.....
; Init Current Record #, return record in H

ZeroCR: push	d
	lhld	_arg
	lxi	d,12
	dad	d
	mvi	m,0		; Clear Hi Byte
	lxi	d,20
	dad	d
	mvi	m,0		; Clear Lo Byte
	pop	d
	ret

;.....
; Convert char in A to Lowercase Ascii

ChLower:
	cpi	'A'
	rc
	cpi	'Z'+1
	rnc
	ori	0x20		; Convert to Lcase
	ret

;= = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
; Bdos data in Text Segment for treating as single module

; struct fcb {
;     char drive;
;     char name[8];
;     char ext[3];
;     char junk1[4];
;     char desc;	/* This byte & 1st byte of Name used for file desc */
;     char name2[8];
;     char ext2[3];
;     char junk2[4];
;     char junk3;
; };

_arg:	.word	00		; Argument passed to Bdos	(char *arg;)
_call:	.byte	0		; Bdos Function #		(char call;)
FName:	.ascii	'            '	; Storage for FCB "name" String
RName:	.ascii	'            '	; 2nd Storage for FCB "name" String (rename)

CName:	.ascii	'            '	; cached filename
	.byte	0
Cfd:	.word	-1		; cached file descriptor

curFil:	.word	00		; Storage for File Descriptor of FCB
				;  (set by Bdos, Used by Bios)
stBuf:	.ds	30		; Buffer for stat() results

DOSSIZ	  equ  .-EStart
RESV	equ	0x100-(DOSSIZ&0x0FF)
	.ds	RESV		; Pad to make Bios start on even mult of 256

;============================================================
; The Bios Jump table MUST start on an even MOD 256 boundary
;============================================================

__bios:	jmp	__cold		; 0 Cold Boot
WBoot:	jmp	Exit		; 1 Warm Boot
BConSt:	jmp	ConSt		; 2 Console Status
BConIn:	jmp	ConIn		; 3 Console Input
BConOu:	jmp	ConOut		; 4 Console Output
	jmp	List		; 5 Printer Output
	jmp	AuxOut		; 6 Auxiliary Output (Punch)
	jmp	AuxIn		; 7 Auxiliary Input (Reader)
	jmp	Home		; 8 Home drive head
	jmp	SelDsk		; 9 Select Drive
	jmp	SetTrk		; 10 Set Track
	jmp	SetSec		; 11 Set Sector
BSDma:	jmp	SetDMA		; 12 Set DMA Address
BRead:	jmp	Read		; 13 Read Sector
BWrit:	jmp	Write		; 14 Write Sector
	jmp	ListSt		; 15 Printer Status
	jmp	SecTrn		; 16 Translate Sector

;------------------------------------------------
; Cold Entry.  Set up CP/M vectors and Stack, Get
; Current TTY Parms, Save for Exit, and begin

__cold:

	mvi	a,0xC3
	lxi	sp,WorkSP	; Small App stack
	sta	0x0005		;  Set Bdos Vector
	lxi	h,__bdos
	shld	0x0006
	lxi	h,WBoot
	sta	0x0000		;   Set Bios Warm Boot Vector
	shld	0x0001
	lxi	h, 0x80
	shld	dmaadr
	lxi	h,0
	shld	srchFD
	lxi	h,ttTermios0	; & buf
	lxi	d,TCGETS	;  ioctl fcn to Get Parms
	call	IoCtl		;   Execute ioctl fcn on STDIN
	lxi	h,ttTermios0
	lxi	d,ttTermios
	lxi	b,20
	call	ldir		;  Move to Work Area
		; Now we need to Change Modes defined in DEVTTY as:
		;   RAW    = 20H	(0000040)
		;   CRMOD  = 10H	(0000020)
		;   ECHO   = 08H	(0000010)
		;   CBREAK = 02H	(0000002)
		;   COOKED = 00H	(0000000)
	lxi	h, 0
	; Turn all the input and output magic off
	shld	ttTermios
	shld	ttTermios + 2
	xra	a
	sta	ttTermios+6		; Echo etch off
	lda	ttTermios+7
	ani	0xF0			; canonical processing off
	sta	ttTermios+7
	lxi	h,1			; VTIME 0
	shld	ttTermios+8		; VMIN 1
	lxi	h, ttTermios
	lxi	d, TCSETS
	call	IoCtl			; Set terminal bits 
	
	call	0x0100		;  ..Execute!

;.....
; 1 - Warm Boot Vector (Exits back to UZI)		{exit (0);}
;     TTY Port Settings are restored to original state.

Exit:	mvi	c,0x0D
	call	ConOut
	mvi	c,0x0A
	call	ConOut
	lxi	h,ttTermios0	; & buf
	lxi	d,TCSETS	;  ioctl fcn to Set Parms
	call	IoCtl		;   Execute ioctl Fcn on STDIN
Quit:
	lxi	h,0		; Exit Good Status
	push	h
	xra	a		;  UZI Fcn 0 (_exit)
	call	syscall		;   Execute!
Spin:	jmp	Spin		; Can't happen!

;.....
; 2 - Return Console Input Status

ConSt:	lxi	h,cnt		; &buf
	lxi	d,TIOCINQ	;  ioctl fcn to read queue count
	call	IoCtl		;   Execute ioctl on STDIN
	lhld	cnt
	mov	a,h
	ora	l		; Anything There?
	rz			; ..return Zero if Not
	ori	0xFF		; Else signify char waiting
	ret

;.....
; 3 - Read Console Input Char			{read (stdin, &char, 1);}

ConIn:	call	ConSt
	jz	ConIn
	lxi	h,1		; 1 char
	push	h
	lxi	d,char		;  Addr to put char
	push	d
	mvi	l,STDIN		;   fd
	push	h
	mvi	a,7		;    FUZIX Read Fcn
ChrV0:	call	syscall		;     Execute
	pop	b
	pop	b
	pop	b
	lda	char
	ret

;.....
; 4 - Write Char in C to Console		{write (stdout, &char, 1);}

ConOut:	mov	a,c
	lxi	d,char
	stax	d		; Stash char
	mvi	h,1		; 1 char
	push	h
	push	d		;  Addr to get char
	mvi	l,STDOUT	;   fd
	push	h
	mvi	a,a8		;    UZI Write Fcn
	jmp	ChrV0		;   ..go to common code

;.....

List:				; Bios Fcn 5
AuxOut:				; Bios Fcn 6
AuxIn:				; Bios Fcn 7
Home:				; Bios Fcn 8
SetTrk:				; Bios Fcn 10
SetSec:				; Bios Fcn 11
ListSt:				; Bios Fcn 15
SecTrn:	xra	a		; Bios Fcn 16.  These are No-Ops
	ret

;.....
; 9 - Select Disk.  Simply return the DPH pointer

SelDsk:	lxi	h,dph		; Return DPH Pointer
	ret

;.....
; 12 - Set DMA Transfer Address

SetDMA:	
	mov	l,c
	mov	h,b
	shld	dmaadr		; Save Address
	ret

;.....
; 13 - Read a "Sector" to DMA Address		{read (curFil, dmaadr, 128);}

Read:	mvi	a,7		; Set UZI Read Fcn
	call	RdWrt		;  Do the work
	rc			; ..exit if Error
	ora	a		; 0 bytes Read?
	jz	XErr1		; ..Return Error if Yes (EOF)
	sbi	128		; A full 128 bytes Read?
	rz			;   return Ok if Yes
	xchg
	lhld	dmaadr
	xchg
	dad	d		; Else offset to byte after end
Feof:	mvi	m,0x1A		;  stuff EOF in case of text
	inx	h
	inr	a
	jnz	Feof
	ret			;   exit with OK status

;.....
; 14 - Write a "Sector" from DMA Address	{write (curFil, dmaadr, 128);}

Write:	mvi	a,8		; Set UZI Write Fcn
	call	RdWrt		;  Do the work
	rc			; ..exit if Error
	sbi	128		; Good Write?
	rz			;   return Ok if Yes
XErr1:	stc			; Set carry
	jmp	XErr		;  Else Return Error

; Common Read/Write Support Routine

RdWrt:	lxi	d,128		; 1 "Sector" char
			; Entry Point accessed by Search Next (BDos)
RdWrt0:	push	d
	lhld	dmaadr		;  from here
	push	h
	lhld	curFil		;   to this file
	push	h
	;    A = R/W Fcn #
	call	syscall		;     Execute!
	pop	b		; Clear Stack
	pop	b
	pop	b
	mov	a,l		; Shuffle possible byte quantity
	rnc			; ..return if No Error
XErr:	mvi	a,0x01		; Else Signal Error (keeping Carry)
	ret

;==========================================================
;		 Bios Support Utilities
;==========================================================
; Execute ioctl Function on STDIN
; Enter: hl = Addr of Parm Block
;	 DE = ioctl Function to execute
; Exit : None
; Uses : AF,BC,DE,hl

IoCtl:	push	h		; &buf
	push	d		;  ioctl fcn
	mvi	e,STDIN		;   fd
	push	d
	mvi	a,29		;    Fuzix ioctl Fcn #
	call	syscall		;     Execute!
	pop	b		; Clean Stack
	pop	b
	pop	b
	ret

;- - - - - - - - - - Data Structures - - - - - - - - -

dph:	.word	0		; Ptr to Skew Table
	.word	0,0,0		; Scratch Words for BDos use
	.word	dir		; Ptr to Directory Buffer
	.word	dpb		; Ptr to DPB
	.word	0		; Ptr to Disk Checksum Buffer
	.word	0		; Ptr to ALV Buffer


dpb:	.word	64		; Dummy Disk Parameter Block
	.byte	4
	.byte	15
	.word	0x0FFFF
	.word	1023
	.byte	0x0FF,0
	.byte	0,0,0,0

;----------------------- Data -----------------------

;
;	The ABI is designed for C and the stack should be
;
;	ARG3		; args if present
;	ARG2
;	ARG1
;	<return addr>	; in C this is the return to the caller of read(...) etc
;	<return addr>	; invocation of syscall stub
;
;
syscall:
syscall_patch:
	call	0
	rnc
	; Error case - hl is errno code not result
	lxi	h,0xffff
	ret
ttTermios:
	.ds	20		; Working TTY Port Settings
ttTermios0:
	.ds	20		; Initial TTY Port Settings

dir:	.ds	128		; Directory Buffer (can cut to 48 bytes?)

	.ds	64
WorkSP:
