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
	.module cpm
	.area	ASEG(ABS)

fcb 	.EQU	0x005C		; Default CP/M FCB
buff	.EQU	0x0080		; Default CP/M Buffer
BS	.EQU	0x08		; ASCII BackSpace
TAB 	.EQU	0x09		; ASCII Horizontal Tab
LF	.EQU	0x0A		; ASCII Line Feed
CR	.EQU	0x0D		; ASCII Carriage Return
ESC	.EQU	0x1B		; ASCII ESCape Char

;
;	Reserved space items
;
char	.EQU	0x3F		; Byte storage for Conin/Conout
SysSP	.EQU	0x40		; System stack pointer
UserSP	.EQU	0x42		; User process stack
dmaSav	.EQU	0x44		; Saved DMA pointer
dmaadr	.EQU	0x46		; DMA address
srchFD	.EQU	0x48		; Directory search fd
LSeekData .EQU	0x4A		; 4 bytes
cnt	.EQU	0x4E		; Count of waiting keys

TCGETS	.EQU	1		; Fuzix - get tty data
TCSETS	.EQU	2		; Fuzix - set tty data
TIOCINQ	.EQU	5		; Fuzix - characters pending

STDIN	.EQU	0	; file descriptor value of keyboard
STDOUT	.EQU	1	; file descriptor value of display

	.area _CODE
EStart:
;
;	Required image header, filled in by the tool chain and
;	consumed by our loader code.
;
		.dw 0x80A9		; Magic number
		.db 0x01		; 8080 family
		.db 0x02		; Z80 featureset required
		.db 0x00		; Relocating

		.db 0x00		; No hints
		.dw 0x0000		; Text size (updated by tools)
		.dw 0x0000		; Data size (updated by tools)
		.dw 0x0000		; BSS size (updated by tools)
		.db 16			; Start address
		.db 0			; Default hint for grab all space
		.db 0			; Default no stack hint
		.db 0			; No zero page on Z80

		; 16 bytes in ...

start2:
;
;
; Load the binary of argv[1] at 0x0100 (obliterating runcpm) (passed as fd
; 3)
;
	LD	HL, (0x31)		; JP xxxx is what FUZIX leaves at 0x30
	LD	(syscall_patch + 1), HL

	LD	(SysSP), SP		; save stack pointer for cold starts
	LD	DE,#EStart - 0x100	; largest possible binary space
	BIT	7,D			; Oversized ?
	JR	Z, load_in_one
	;	Load first chunk
	LD	DE,#0x7F00
	PUSH	DE
	LD	DE,#0x0100
	PUSH	DE
	LD	DE,#3
	PUSH	DE
	LD	A,#7
	CALL	syscall
	LD	A,H
	AND	L
	CP	#0xFF
	JP	Z, Quit
	; HL is bytes read
	LD	A,#0x7F
	CP	H
	JR	NZ, short_read
	POP	AF
	POP	AF
	POP	AF
	LD	DE,#EStart-0x8000
	PUSH	DE
	LD	DE,#0x8000
	JR	load_2
load_in_one:
	PUSH	DE
	LD	DE, #0x0100	; address
load_2:
	PUSH	DE
	LD	DE,#3		; file handle
	PUSH	DE
	LD	A,#7		; read(3, 0x100, size)
	CALL	syscall
	LD	A,H
	OR	L
	CP	#0xFF
	JP	Z, Quit
	POP	DE
	POP	DE
	POP	DE
short_read:
	LD	DE,#3
	PUSH	DE
	LD	A,#2
	CALL	syscall		; close (3)
	POP	DE

	LD	DE,#dir
	LD	B,#128
	CALL	ZeroDE		; Clear Directory Buffer

	LD	HL,#0
	LD	(0x0003),HL	; Clear IOBYTE and Default Drive/User


	JP	__bios		; Go to Cold Start setup

;==========================================================
;     Resident Portion of Basic Disk Operating System
;==========================================================
; bdos()
; {
__bdos:	JP	_bdos0
; }

;.....
; BDOS Function Dispatch Table

fcnTbl:	.dw	Fcn0		; Warm Boot
	.dw	Fcn1		; ConIn
	.dw	Fcn2		; ConOut
	.dw	Fcn3		; Reader In
	.dw	Fcn4		; Punch Out
	.dw	Fcn5		; List Output
	.dw	Fcn6		; Direct Console IO
	.dw	Fcn7		; Get IOBYTE
	.dw	Fcn8		; Set IOBYTE
	.dw	Fcn9		; WrBuf
	.dw	Fcn10		; RdBuf
	.dw	Fcn11		; Get Console Status
	.dw	Fcn12		; Return Version #
	.dw	Fcn13		; Reset Disk Drive
	.dw	Fcn14		; Select Disk
	.dw	Fcn15		; Open File
	.dw	Fcn16		; Close File
	.dw	Fcn17		; Search First Occurance
	.dw	Fcn18		; Search Next Occurance
	.dw	Fcn19		; Delete File
	.dw	Fcn20		; Read File
	.dw	Fcn21		; Write File
	.dw	Fcn22		; Create File
	.dw	Fcn23		; Rename File
	.dw	Fcn24		; Return Disk Login Vector
	.dw	Fcn25		; Return Current Disk
	.dw	Fcn26		; Set DMA
	.dw	Fcn27		; Get Allocation Map
	.dw	Fcn28		; Write Protect Disk
	.dw	Fcn29		; Get R/O Vector Address
	.dw	Fcn30		; Set File Attributes
	.dw	Fcn31		; Get Disk Parameter Table Address
	.dw	Fcn32		; Set/Get User Code
	.dw	Fcn33		; Read Random
	.dw	Fcn34		; Write Random
	.dw	Fcn35		; Compute File Size
	.dw	Fcn36		; Set Random Record Field in FCB
;	.dw	Fcn37		; Reset Multiple Drives
;	.dw	null		; (Fcn38 not implemented)
;	.dw	Fcn39		; Get Fixed Disk Vector
;	.dw	Fcn40		; Write Random
TBLSZ	 .EQU	.-fcnTbl
MAXFCN	 .EQU	TBLSZ/2

;------------------------------------------------
; bdos0()
; {

_bdos0:	LD	(_arg),DE
	LD	A,C
	LD	(_call),A
	CP	#MAXFCN		; Legal Function?
	LD	A,#0xFF		;  Prepare Error code
	LD	L,A
	RET	NC		; ..return if Illegal
	LD	(UserSP),SP
	LD	SP,(SysSP)	; return to system stack
	PUSH	IX
	PUSH	IY
	LD	B,#0		; Fcn # to Word
	LD	HL,#_bdosX
	PUSH	HL		;  (ret Addr to Stack)
	LD	HL,#fcnTbl
	ADD	HL,BC
	ADD	HL,BC		;   Pt to Fcn entry in Table
	LD	A,(HL)
	INC	HL
	LD	H,(HL)
	LD	L,A
	JP	(HL)		; "Call" Function #

_bdosX:	POP	IY
	POP	IX
	LD	SP,(UserSP)
	LD	DE,(_arg)	; Return Orig contents of DE
	LD	A,(_call)
	LD	C,A		; Return Orig contents of C
	LD	A,L
	LD	B,H		; Strict compatibility
	OR	A
	RET
; }

;------------------------------------------------
;         case 0: _exit();			/* Warm Boot */

Fcn0:	JP	WBoot

;------------------------------------------------
;         case 6: if (arg < 0xfe)		/* Direct Console I/O */
;                     goto conout;
;                 else if (arg == 0xfe)
;                     return (ConSt);
;                 else if (ConSt)       /* 0xff */
;                     goto conout;
;                 else return (0);

Fcn6:	LD	A,E		; _arg in DE
	CP	#0x0FE		; < 0FE ?
	JR	C,Fcn2		; ..jump if Write if Yes
	PUSH	AF
	CALL	BConSt		; Else get Console Status
	POP	AF
	INC	A
	RET	NZ		; ..exit with 0ffh if 0feh
	LD	A,H
	OR	L		; Any char ready?
	RET	Z		; ..exit if Nothing available
			;..else fall thru to Fcn1..
;------------------------------------------------
;         case 1:				/* Console Input */
;         conin:  read (0, &c, 1);
;                 if (c == '\n')
;                     c = '\r';
;                 return (c);

Fcn1:	CALL	BConIn		; Get Char from Bios
Fcn1A:	LD	H,#0
	CP	#0x0A		; \n?
	LD	L,A		;  (prepare for return
	RET	NZ		; ..return if Not
	LD	L,#0x0D		; Else return CR
	RET

;------------------------------------------------
;         case 3:				/* Reader (Aux) Input */
;         conin:  read (0, &c, 1);
;                 if (c == '\n')
;                     c = '\r';
;                 return (c);

Fcn3:	CALL	AuxIn		; Get Char from Bios
	JR	Fcn1A		; ..exit via common code

;------------------------------------------------
;         case 2:				/* Console Output */
;         conout: if (arg == '\r')
;                     return (0);
;                 c = arg;
;                 write (1, &c, 1);
;                 break;

Fcn2:	LD	C,E		; _arg in DE, need char in C
	JP	BConOu

;------------------------------------------------
;         case 4:				/* Punch (Aux) Output */
;         conout: if (arg == '\r')
;                     return (0);
;                 c = arg;
;                 write (1, &c, 1);
;                 break;

Fcn4:	LD	C,E		; _arg in DE, need char in C
	JP	AuxOut

;------------------------------------------------
;         case 5: if (arg == '\r')		/* List (Prntr) Output */
;                     return (0);
;                 c = arg;
;                 write (2, &c, 1);
;                 break;

Fcn5:	LD	A,E		; _arg in DE
	CP	#13		; \r?
	RET	Z
	JP	List		; ..go to Bios

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
Fcn9:	LD	A,(DE)		; Get char
	INC	DE		;   pt to Next
	CP	#'$'		; End?
	RET	Z		; ..quit if Yes
	LD	C,A
	PUSH	DE
	CALL	BConOu
	POP	DE
	JR	Fcn9		; ..loop Til done

;------------------------------------------------
;         case 10: rdbuf (arg);
;                  break;
; rdbuf (arg)
; char *arg;
; {
;     int nread;

;     nread = read (0, arg+2, *arg & 0xff);

Fcn10:
	push	de
	ex	de,hl		; hl - buffer
	ld	e,(hl)		; e - max chars
	inc	hl
	inc	hl
	ld	d,#0		; d - char cnt
get:	push	hl
	push	de
	call	BConIn
	pop	de
	pop	hl
	cp	#8
	jr	z,del
	cp	#0x7F
	jr	z,del
	cp	#3
	jp	z,0
	push	hl
	push	de
	push	af
	ld	c,a
	call	BConOu
	pop	af
	pop	de
	pop	hl
	ld	(hl),a
	cp	#CR
	jr	z,eol
	ld	a,e
	cp	d
	jr	z,eol1
	inc	hl
	inc	d
	jr	get
del:	ld	a,d
	or	a
	jr	z,get
	push	hl
	push	de
	ld	c,#8
	call	BConOu
	ld	c,#' '
	call	BConOu
	ld	c,#8
	call	BConOu
	pop	de
	pop	hl
	dec	hl
	dec	d
	jr	get
eol:	ld	(hl),#0
eol1:	ld	a,d
	pop	de
	inc	de
	ld	(de),a
	ld	hl,#0
	ret


;------------------------------------------------
;         case 11: return (ConSt);		/* Get Console Status */

Fcn11:	JP	BConSt

;------------------------------------------------
;         case 12:				/* Return Version # */

Fcn12:	LD	HL,#0x0022	; Say this is CP/M 2.2
	RET

;------------------------------------------------
;         case 13:				/* Reset Disk Drive */
;              SDma(0x80);
;              break;
Fcn13:
	LD	BC,#0x80
	JP	BSDma

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

Exit0:	LD	HL,#0
	RET

;------------------------------------------------
;         case 15: return (openfile (arg));		/* Open File */
; openfile (blk)
; {
;     desc = open (getname (arg), 2);
				; DE -> arg
Fcn15:	CALL	Fcn17		; Does this file exist?
	LD	A,H
	AND	L
	INC	A		; File Not Found (-1)?
	RET	Z		; ..return -1 if File doesn't exist

Open1:	CALL	CkSrch		;  (Close Search File)

;     arg.recno = 0;

	CALL	ZeroCR
	LD	13(IY),#0x80	; use S1 as file open flag

	JR	Exit0		;  Return Dir Code for Entry

;.....
; Common File Open Routine.  Used by Read, Write and Search First.
; Enter: DE = File Mode
;	 HL = Ptr to Null-terminated Path String
; Exit : A = 0 if Error, HL = -1
;	     File Descriptor, A <> 0 if Ok

OpenF:	PUSH	DE		; Mode
	PUSH	HL		;  Path
	LD	A,#1		;   Fuzix Open Fcn #
	CALL	syscall		;    _open (Path, Mode);
	POP	BC		; Clean Stack
	POP	BC
	LD	A,H
	AND	L
	INC	A		; FF -> 0?
	RET			; ..return (HL=-1/A=0 if Err, HL=fd/A<>0 of Ok)

;------------------------------------------------
;         case 16: return (closefile (arg));		/* Close File */

;     if (close (arg->desc) == -1)

Fcn16:	LD	IY,(_arg)
	LD	13(IY),#0	; clear file open flag

	LD	A,#11		; Fuzix sync function #
	CALL	syscall		; Execute!		FIXME???

	JP	Exit0		; Return OK

;....
; Close file descriptor

CloseV:	PUSH	DE
	LD	A,#2		;  Fuzix Close Fcn #
	CALL	syscall		;   Execute!
	POP	BC		; Clean Stack
	RET

;------------------------------------------------
;         case 17:					/* Search First */

Fcn17:	CALL	CkSrch		; Ensure Search File closed
	LD	HL,#'.'		; Open current directory
	LD	(RName),HL	;  store name in Secondary work string
	LD	DE,#0		; Open Read-Only
	LD	HL,#RName
	CALL	OpenF		;  _open ('.', 0);
	RET	Z		; HL = -1, A = 0 if Can't Open

	LD	(srchFD),HL	; Else Ok, Save File Descriptor
	LD	(curFil),HL	;   Duplicate for Reading
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
Fcn18:	LD	HL,(dmaadr)
	LD	(dmaSav),HL	; Save "real" DMA
Fcn18A:	LD	HL,#dir+16
	LD	(dmaadr),HL	;  Set DMA for Dir Op'n
	LD	A,#24		;  Getdirent
	LD	DE,#32		;  Len of Dir entries
	CALL	RdWrt0		;   Read an Entry
	JR	C,#Fcn18E	; Error if Carry Set
	OR	A		; Read Ok?
	JR	Z,Fcn18E	; ..Return HL=-1 if EOF
	CALL	ChkDir		; Else Set Dir to CP/M, Check Match
	OR	A
	JR	NZ,Fcn18A	; ..loop if No Match

	LD	A,(_call)
	CP	#15		; Is this a File Open internal Call?
	LD	HL,#0		;  (set Success, Index 0)
	JR	Z,Fcn18X	; ..exit now if Yes

	LD	HL,#dir		; Else
	LD	DE,(dmaSav)	;  Move Dir Buffer to "real" DMA
	LD	BC,#128
	LDIR
	LD	L,B		; Use 0 in BC
	LD	H,C		;   to show Index 0 (success)
	JR	Fcn18X		;  ..exit

Fcn18E:	LD	HL,#-1
Fcn18X:	LD	DE,(dmaSav)
	LD	(dmaadr),DE	; Restore "real" DMA Addr
	RET

;------------------------------------------------
;         case 19: return (delete (arg));		/* Delete File */

Fcn19:	CALL	CkSrch		; Ensure Search file closed

;     if (unlink (getname (arg)) == -1)
				; DE -> arg
	CALL	GetNam		;  Parse to String
	PUSH	HL		; String
	LD	A,#6		;  FUZIX Unlink Fcn #
	CALL	syscall		;   Execute!
	POP	BC		; Clean Stack

;         return (255);
;     return (0);

	LD	A,H
	AND	L
	INC	A		; FF->0?
	JP	NZ,Exit0	;  return Ok if No

ExitM1:	LD	HL,#-1
	RET


;------------------------------------------------
;         case 33:					/* Read File Random */
;	    readrandom (fcb)
;	    {

Fcn33:	CALL	RWprep		; Prepare File for access
	JP	Z,Exit1
	CALL	RecCvt
	CALL	DoRead
	JR	RWEx

;....
DoRead:
	CALL	LSeek		; Seek to Offset (128-byte rec in Block)

	CALL	BRead		; Read 1 Sector

	PUSH	AF
	LD	DE,(curFil)
;;;;	CALL	CloseV		; Close the file
	LD	DE,#0
	LD	(curFil),DE
	POP	AF

	RET

RecCvt:
	; Record count to EX/CR
	; CR = record & 7F
	; EX = record >> 7
	LD	IY,(_arg)
	LD	A,33(IY)	; Low byte
	AND	#0x7F
	LD	32(IY),A	; Low 7bits into record

	LD	A, 34(IY)	; High byte
	ADD	A		; double
	BIT	7,33(IY)	; Carry over ?
	JR	Z, NoCarry
	INC	A		; And drop in the upper bit of byte 0
NoCarry:
	LD	12(IY),A
	RET

;------------------------------------------------
;         case 20: return (readfile (arg));		/* Read File */
; readfile (arg)
; {
;     nread = read (blk->desc, dmaadr, 128);
				; DE -> arg (FCB)
Fcn20:	CALL	RWprep		; Prepare file for access
	JP	Z,Exit1

	CALL	DoRead		; Read 1 Sector

;     arg.recno++;

	CALL	IncCR		; Bump Current Record #

RWEx:	JP	C,Exit1		; ..Error if Carry Set

;     if (nread == 0)
;         return (0);

	OR	A		; Good Read?
	JP	Z,Exit0		;   exit w/0 if Yes

;     else  return (1)

	JP	Exit1

;------------------------------------------------
;         case 34:					/* Write File Random */
;	    writerandom (fcb)
;	    {

Fcn34:	CALL	RWprep		; Prepare file for access
	JP	Z,Exit1
	CALL	RecCvt
	CALL	DoWrite
	JR	RWEx

;....
DoWrite:
	CALL	LSeek		; Seek to Offset (128-byte rec in Block)

	CALL	BWrit		; Write 1 Sector

	PUSH	AF
	LD	DE,(curFil)
;;;;	CALL	CloseV		; Close the file
	LD	DE,#0
	LD	(curFil),DE
	POP	AF

	RET

;------------------------------------------------
;         case 21: return (writefile (arg));		/* Write File */
; writefile (arg)
; {
;     if (write (blk->desc, dmaadr, 128) != 128)

				; DE -> arg (FCB)
Fcn21:	CALL	RWprep		; Prepare file for access
	JP	Z,Exit1

Fcn21A:	CALL	DoWrite		;   Write

;     arg.recno++;

	CALL	IncCR		; Bump Current Record #

;         return (255);
;     return (0);

	JR	RWEx		; ..exit via Common R/W Code
; }

;------------------------------------------------
;         case 22: return (makefile (arg));		/* Create File */
; makefile (arg)
; {
;     desc = creat (getname (blk), 0666);

Fcn22:	CALL	CkSrch		; Ensure Search file closed
	LD	HL,#0x1B6	; Own/Grp/Oth are Read/Execute
	PUSH	HL		; DE -> arg
	LD	HL,#0x502	; O_CREAT|O_RDWR|O_TRUNC
	CALL	GetNam		;  This name string
	PUSH	HL
	LD	A,#1		;   Fuzix open Fcn #
	CALL	syscall		;    Execute!
	POP	BC		; Clean Stack
	POP	BC
	POP	BC

;     if (desc == -1)

	LD	A,H
	AND	L
	INC	A		; FF -> 0?

;         return (255);

	RET	Z		; ..return -1 if Yes

;     arg.recno = 0;

	EX	DE,HL
	CALL	CloseV
	JP	Open1

;------------------------------------------------
;         case 23: return (rename (arg));		/* Rename File */
; rename (arg)
; {
;     RName = getname (arg);
;
;	FIXME: should use rename() syscall now
;
Fcn23:	CALL	CkSrch		; Ensure Search file closed
	PUSH	DE		; Save FCB Ptr
	CALL	GetNam		;  parse to UZI String

	LD	HL,#FName
	LD	DE,#RName
	LD	BC,#12
	LDIR			; Copy to Rename string

;     FName = getname (arg+16);

	POP	DE		; DE -> _arg
	LD	HL,#16
	ADD	HL,DE		; Offset to New Name
	EX	DE,HL
	CALL	GetNam		;  parse it returning HL -> FName

;     if (link (RName, FName) < 0) {

	PUSH	HL		; New Name
	LD	HL,#RName	;  Old Name
	PUSH	HL
	LD	A,#5		;   FUZIX link Fcn #
	CALL	syscall		;    Execute!
	POP	BC		; Clean Stack
	POP	BC

;         return (-1);

	JP	C,ExitM1	; Exit w/Err if Bad
;     }
;     if (unlink (RName) < 0) {

	LD	HL,#RName	; Old Name
	PUSH	HL
	LD	A,#6		;  UZI unlink Fcn #
	CALL	syscall		;   Execute!
	POP	BC		; Clean Stack
	JP	NC,Exit0	;   exit w/0 if Ok

;         unlink (FName);
				; Else remove the new iNode
	LD	HL,#FName	; New Name
	PUSH	HL
	LD	A,#6		;  UZI unlink Fcn #
	CALL	syscall		;   Execute!
	POP	BC		; Clean Stack

;         return (-1);

	JP	C,ExitM1	;  return -1 if Bad
;     }
;     return (0);

	JP	Exit0		;   else return Ok
; }

;------------------------------------------------
;         case 24: return (1);			/* Return Disk Login Vector */

Fcn24:
Exit1:	LD	HL,#1
	RET

;------------------------------------------------
;         case 26: dmaadr = (char *)arg;		/* Set DMA Address */
;                  break;
				; Enter DE = DMA Address
Fcn26:	LD	C,E
	LD	B,D		; Move to Bios Regs
	JP	BSDma		;  Set in Bios & return

;------------------------------------------------
;         case 27: return (-1)			/* Get Allocation Map */
;         case 29: return (-1)			/* Get R/O Vector Address */
Fcn27:
Fcn29:	LD	HL,#-1
	RET

;------------------------------------------------
;         case 31: return (&dpb);		/* Get Disk Param Table Addr */

Fcn31:	LD	HL,#dpb
	RET
; }

;------------------------------------------------
;         case 35:				/* Return File Size in FCB */
;				   /* Use stat fcn, rounding up to mod-128 */
;	    if (_stat (dname, &statbuf) == 0) {
				; DE -> fcb
Fcn35:	CALL	CkSrch		; Ensure Search file closed
	CALL	GetNam		;  parse to UZI String
	LD	DE,#stBuf
	PUSH	DE		; &statbuf
	PUSH	HL		;  dname
	LD	A,#15		;   UZI stat Fcn #
	CALL	syscall		;    Execute!
	POP	BC		; Clean Stk
	POP	BC
	LD	IY,(_arg)
	LD	A,H
	OR	L		; 0?
	JR	NZ,Fcn35X	; ..jump if Bad

;			      >> 7;
;	FIXME: offset is now a simple number so this is wrong
;

	LD	D,#0
	LD	HL,(stBuf+15)	; Upper bytes of size (256 byte blocks)
	BIT	7,H
	JR	Z, notover
	INC	D		; Overflowed
notover:
	ADD	HL,HL		; Now in CP/M blocks
	LD	A,(stBuf+14)	; Low byte
	BIT	7,A
	JR	Z, noblockadd
	INC	HL		; There is another whole block
	AND	#127		; Can't cause an overflow as prev value is even
noblockadd:
	OR	A
	JR	Z, noround
	INC	HL		; Partial block
noround:
	LD	33(IY),L
	LD	34(IY),H	;  Store in RR fields in FCB
	LD	35(IY),D	; (D = 0)

;		return (0);

	LD	L,D
	LD	H,D		; HL = 0
	RET

;	    else {
;		(int)fcb+33 = 0;

Fcn35X:	LD	33(IY),#0
	LD	34(IY),#0
	LD	35(IY),#0

;		return (-1);

	LD	HL,#-1
;	    }
	RET

;------------------------------------------------
;         case 36:			/* Set Random Record Field in FCB */

Fcn36:	LD	IY,(_arg)
	LD	A,32(IY)	; Fetch RecNo
	LD	33(IY),A	;  place in LSB of RR field (r0)
	LD	A,12(IY)
	LD	34(IY),A	;  set (r1)
	LD	35(IY),#0	;  Clear Hi byte of RR (r2)

	LD	HL,#0		; Return Ok
	RET

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
GetNam:	LD	IX,#FName	; Dest to string
	EX	DE,HL
	PUSH	HL		;   (save)
	INC	HL		;  adv to 1st char of FN

;     for (j = 0; j < 8; ++j)
;     {
;         if (!blk->name[j] || blk->name[j] == ' ')
;             break;

	LD	B,#8
GetN0:	LD	A,(HL)
	INC	HL
	OR	A
	JR	Z,GetN1
	CP	#' '
	JR	Z,GetN1

;         *p++ = chlower (blk->name[j]);

	CALL	ChLower
	LD	0(IX),A
	INC	IX
	DJNZ	GetN0
;     }

GetN1:	POP	HL
	LD	DE,#9
	ADD	HL,DE		; Pt to 1st char of FT
	LD	A,(HL)
	CP	#' '		; Any Type?
	JR	Z,GetNX		; ..quit if Not

;     *p++ = '.';

	LD	0(IX),#'.'
	INC	IX

;     for (j = 0; j < 3; ++j)

	LD	B,#3

;     {
;         if (!blk->ext[j] || blk->ext[j] == ' ')
;             break;

GetN2:	LD	A,(HL)
	INC	HL
	CP	#' '
	JR	Z,GetNX

;         *p++ = chlower (blk->ext[j]);

	CALL	ChLower
	LD	0(IX),A
	INC	IX
	DJNZ	GetN2

;     }
;     *p = '\0';

GetNX:	LD	0(IX),#0

;     return (name);

	LD	HL,#FName
	RET
; }

;
;	We use lseek for Fuzix
;
;	The internal format of the FCB (abused by some apps) has
;	EX at 0x0C and CR at 0x20. The actual disk byte offse tis
;
;	128 * CR + 16384 * EX
;
LSeek:	LD	BC,#0		; Push 0 for absolute
	PUSH	BC
	LD	IY, (_arg)	; FCB

	LD	B, 32(IY)
	LD	C, #0
	SRL	B
	RR	C		; BC = 128 x CR

	LD	L,12(IY)
	LD	H,#0
	ADD	HL,HL		; 512
	ADD	HL,HL		; 1024
	ADD	HL,HL		; 2048
	ADD	HL,HL		; 4096
	ADD	HL,HL		; 8192
	ADD	HL,HL		; 16384
	; HL is now the upper 24 bits of the offset
	LD	A,H
	LD	H,L
	LD	L,#0		; AHL is now the 24bit value
	ADD	HL,BC		; Add in bits from CR
	ADC	#0		; Update A if needed

	LD	(LSeekData), HL	; low bits
	LD	(LSeekData + 2), A	; high bit (top byte already clear)?
	XOR	A
	LD	(LSeekData + 3),A	; Needed ????
	LD	BC, #LSeekData	; push pointer
	PUSH	BC
	LD	HL, (curFil)
	PUSH	HL
	LD	A,#9		; _lseek()
	CALL	syscall		; Syscall
	POP	BC		; Recover stack
	POP	BC
	POP	BC
	RET

;.....
; Perform File Access Preparatory actions:
; Open file for R/W and Seek to current Record #
; Enter: DE = Ptr to FCB
; Exit : A = 0 and HL = -1 if Error, A <> 0 if Ok

RWprep:	CALL	CkSrch		; Ensure Search file closed
	LD	HL,#13		; offset to S1 (file open flag)
	ADD	HL,DE
	LD	A,(HL)
	AND	#0x80
	LD	HL,#-1
	RET	Z

	CALL	GetNam		;  Parse FCB Fn.Ft to String

	CALL	COpen
	RET	Z		; ..return -1 on error

	LD	(curFil),HL	; store file descriptor for Bios
	RET

COpen:	PUSH	HL
	LD	DE,#CName
chk:	LD	A,(DE)
	CP	(HL)		; compare filename with cached name
	JR	NZ,differ
	OR	A
	JR	Z,same
	INC	HL
	INC	DE
	JR	chk
same:	POP	DE		; if same, just return the cached file descr
	LD	HL,(Cfd)
	LD	A,H
	AND	L
	INC	A
	RET	NZ
	EX	DE,HL
	JR	op1
differ:	LD	HL,(Cfd)
	LD	A,H
	AND	L
	INC	A
	EX	DE,HL
	CALL	NZ,CloseV	; close old file
	POP	HL		; restore file name
	CALL	Ccopy
op1:	LD	DE,#2		; open for R/W
	CALL	OpenF
	LD	(Cfd),HL
	RET

Ccopy:	PUSH	HL
	LD	DE,#CName
cpy:	LD	A,(HL)
	LD	(DE),A
	INC	HL
	INC	DE
	OR	A
	JR	NZ,cpy
	POP	HL
	RET

;.....
; Convert FUZIX Directory Entry at dir+16 to CP/M FCB entry at dir, Zero rest.
; Ambiguously compare FCB FN.FT at dir to that passed at arg, returning Zero
; if Match, Non-Zero if mismatch.

ChkDir:	LD	DE,#dir
	LD	HL,#dir+16+2	; Pt to 1st char of Name
	XOR	A
	LD	(DE),A		; Zero Drive field
	INC	DE		;  Pt to 1st char of FN
	LD	B,#8
	CALL	ChkD0		;   Fix Name
	LD	B,#3
	CALL	ChkD0		;    & Type
	LD	B,#21
	CALL	ZeroDE		;     Clear rest of Dir entry

	LD	DE,(_arg)
	INC	DE		; Pt to 1st char of FN
	LD	A,(DE)
	CP	#' '		; Any Name present?
	JR	NZ,ChkFN0	; ..jump if Yes
	LD	HL,#8
	ADD	HL,DE		;  Else offset to 1st char of FT
	LD	A,(HL)
	CP	#' '		;   Type present?
	LD	A,#0x0FF		;    (Assume Error)
	RET	Z		;     Return w/Err Flag if no Type either

ChkFN0:	LD	HL,#dir+1	; Else Compare name/type fields
	LD	B,#11
			; Ambiguous FN.FT compare of (HL) to (DE)
ChkL:	LD	A,(DE)
	CP	#'?'		; Accept anything?
	JR	Z,ChkL0		; ..jump if ambiguous
	XOR	(HL)
	AND	#0x7F		; Match?
	RET	NZ		; .Return Non-Zero if Not
ChkL0:	INC	HL
	INC	DE
	DJNZ	ChkL		; ..loop til Done
	XOR	A		;    return Zero for Match
	RET

;.....
; Parse FileSpec addressed by HL into FN.FT Spec addressed by DE.

ChkD0:	LD	A,(HL)		; Get Char
	CP	#'a'
	JR	C,ChkD1
	CP	#'z'+1
	JR	NC,ChkD1
	AND	#0x5F		; Convert to Uppercase
ChkD1:	OR	A		; End of String?
	JR	Z,ChkDE		; ..jump if End
	INC	HL		;     (bump Inp Ptr if Not End)
	CP	#'.'
	JR	Z,ChkDE		;  ..or Period field separator
	LD	(DE),A		; Store char
	INC	DE		;  bump Dest
	DJNZ	ChkD0		; ..loop til field done
ChkD2:	LD	A,(HL)		; Get Next
	OR	A
	RET	Z		;  Exit at End of string
	INC	HL		;   (adv to next)
	CP	#'.'
	RET	Z		;   or field separator
	JR	ChkD2		;  ..loop til end found

ChkDE:	LD	A,#' '		; Fill rest w/Spaces
ChkD3:	INC	B
	DEC	B		; More in field?
	RET	Z		; ..exit if Not
	JR	ZeroL		;  ..else stuff spaces til field ends

;.....
; Zero area addressed by DE for B Bytes.  Uses A,B,DE.

ZeroDE:	XOR	A
ZeroL:	LD	(DE),A
	INC	DE
	DJNZ	ZeroL
	RET

;.....
; Close the Directory if we just exitted a SearchF/SearchN sequence

CkSrch:	PUSH	DE		; Save Regs
	PUSH	HL
	LD	DE,(srchFD)	; Get File Desc
	LD	A,D
	OR	E		; Anything open?
	CALL	NZ,CloseV	;  Close file if Yes
	LD	HL,#0
	LD	(srchFD),HL	;   Mark as closed
	POP	HL		;    (ignore Errors)
	POP	DE
	RET

;.....
; Bump current Record # for sequential R/W operations. CP/M uses only
; 7bits of the low byte

IncCR:	
	PUSH	AF
	LD	IY,(_arg)
	INC	32(IY)
	BIT	7,32(IY)
	JR	Z, NoBump
	LD	32(IY),#0	
	INC	12(IY)		; Bump Hi byte
NoBump:
	POP	AF
	RET

;.....
; Init Current Record #

ZeroCR:	LD	IY,(_arg)
	LD	32(IY),#0	; Clear Lo Byte
	LD	12(IY),#0	; Clear Hi Byte
	RET

;.....
; Convert char in A to Lowercase Ascii

ChLower:
	CP	#'A'
	RET	C
	CP	#'Z'+1
	RET	NC
	OR	#0x20		; Convert to Lcase
	RET

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

_arg:	.dw	00		; Argument passed to Bdos	(char *arg;)
_call:	.db	0		; Bdos Function #		(char call;)
FName:	.ascii	'            '	; Storage for FCB "name" String
RName:	.ascii	'            '	; 2nd Storage for FCB "name" String (rename)

CName:	.ascii	'            '	; cached filename
	.db	0
Cfd:	.dw	-1		; cached file descriptor

curFil:	.dw	00		; Storage for File Descriptor of FCB
				;  (set by Bdos, Used by Bios)
stBuf:	.ds	30		; Buffer for stat() results

DOSSIZ	  .EQU  .-EStart
RESV	.EQU	0x100-(DOSSIZ&0x0FF)
	.ds	RESV		; Pad to make Bios start on even mult of 256

;============================================================
; The Bios Jump table MUST start on an even MOD 256 boundary
;============================================================

__bios:	JP	__cold		; 0 Cold Boot
WBoot:	JP	Exit		; 1 Warm Boot
BConSt:	JP	ConSt		; 2 Console Status
BConIn:	JP	ConIn		; 3 Console Input
BConOu:	JP	ConOut		; 4 Console Output
	JP	List		; 5 Printer Output
	JP	AuxOut		; 6 Auxiliary Output (Punch)
	JP	AuxIn		; 7 Auxiliary Input (Reader)
	JP	Home		; 8 Home drive head
	JP	SelDsk		; 9 Select Drive
	JP	SetTrk		; 10 Set Track
	JP	SetSec		; 11 Set Sector
BSDma:	JP	SetDMA		; 12 Set DMA Address
BRead:	JP	Read		; 13 Read Sector
BWrit:	JP	Write		; 14 Write Sector
	JP	ListSt		; 15 Printer Status
	JP	SecTrn		; 16 Translate Sector

;------------------------------------------------
; Cold Entry.  Set up CP/M vectors and Stack, Get
; Current TTY Parms, Save for Exit, and begin

__cold:

	LD	A,#0xC3
	; Needs to differ from the saved workspace or we'll trash any
	; small stack a CP/M app assumes existss
	LD	SP,#AppStack	; Set CP/M Stack for execution
	LD	(0x0005),A	;  Set Bdos Vector
	LD	HL,#__bdos
	LD	(0x0006),HL
	LD	HL,#WBoot
	LD	(0x0000),A	;   Set Bios Warm Boot Vector
	LD	(0x0001),HL
	LD	HL, #0x80
	LD	(dmaadr), HL
	LD	HL,#0
	LD	(srchFD),hl
	LD	HL,#ttTermios0	; & buf
	LD	DE,#TCGETS	;  ioctl fcn to Get Parms
	CALL	IoCtl		;   Execute ioctl fcn on STDIN
	LD	HL,#ttTermios0
	LD	DE,#ttTermios
	LD	BC, #20
	LDIR			;  Move to Work Area
		; Now we need to Change Modes defined in DEVTTY as:
		;   RAW    = 20H	(0000040)
		;   CRMOD  = 10H	(0000020)
		;   ECHO   = 08H	(0000010)
		;   CBREAK = 02H	(0000002)
		;   COOKED = 00H	(0000000)
	LD	HL, #0
	; Turn all the input and output magic off
	LD	(ttTermios), HL
	LD	(ttTermios + 2), HL
	XOR	A
	LD	(ttTermios+6), A	; Echo etch off
	LD	A, (ttTermios+7)
	AND	#0xF0		; canonical processing off
	LD	(ttTermios+7), A
	LD	HL, #1			; VTIME 0
	LD	(ttTermios+8), HL	; VMIN 1
	LD	HL, #ttTermios
	LD	DE, #TCSETS
	CALL	IoCtl			; Set terminal bits 

	CALL	0x0100		;  ..Execute!

;.....
; 1 - Warm Boot Vector (Exits back to UZI)		{exit (0);}
;     TTY Port Settings are restored to original state.

Exit:	LD	C,#0x0D
	CALL	ConOut
	LD	C,#0x0A
	CALL	ConOut
	LD	HL,#ttTermios0	; & buf
	LD	DE,#TCSETS	;  ioctl fcn to Set Parms
	CALL	IoCtl		;   Execute ioctl Fcn on STDIN
Quit:
	LD	HL,#0		; Exit Good Status
	PUSH	HL
	XOR	A		;  UZI Fcn 0 (_exit)
	CALL	syscall		;   Execute!
Spin:	JR	Spin		; Can't happen!

;.....
; 2 - Return Console Input Status

ConSt:	LD	HL,#cnt		; &buf
	LD	DE,#TIOCINQ	;  ioctl fcn to read queue count
	CALL	IoCtl		;   Execute ioctl on STDIN
	LD	HL,(cnt)
	LD	A,H
	OR	L		; Anything There?
	RET	Z		; ..return Zero if Not
	OR	#0x0FF		; Else signify char waiting
	RET

;.....
; 3 - Read Console Input Char			{read (stdin, &char, 1);}

ConIn:	call	ConSt
	jr	z,ConIn
	LD	HL,#1		; 1 char
	PUSH	HL
	LD	DE,#char		;  Addr to put char
	PUSH	DE
	LD	L,#STDIN		;   fd
	PUSH	HL
	LD	A,#7		;    UZI Read Fcn
ChrV0:	CALL	syscall		;     Execute
	POP	BC
	POP	BC
	POP	BC
	LD	A,(char)
	RET

;.....
; 4 - Write Char in C to Console		{write (stdout, &char, 1);}

ConOut:	LD	A,C
	LD	DE,#char
	LD	(DE),A		; Stash char
	LD	HL,#1		; 1 char
	PUSH	HL
	PUSH	DE		;  Addr to get char
	LD	L,#STDOUT	;   fd
	PUSH	HL
	LD	A,#8		;    UZI Write Fcn
	JR	ChrV0		;   ..go to common code

;.....

List:				; Bios Fcn 5
AuxOut:				; Bios Fcn 6
AuxIn:				; Bios Fcn 7
Home:				; Bios Fcn 8
SetTrk:				; Bios Fcn 10
SetSec:				; Bios Fcn 11
ListSt:				; Bios Fcn 15
SecTrn:	XOR	A		; Bios Fcn 16.  These are No-Ops
	RET

;.....
; 9 - Select Disk.  Simply return the DPH pointer

SelDsk:	LD	HL,#dph		; Return DPH Pointer
	RET

;.....
; 12 - Set DMA Transfer Address

SetDMA:	LD	(dmaadr),BC	; Save Address
	Ret

;.....
; 13 - Read a "Sector" to DMA Address		{read (curFil, dmaadr, 128);}

Read:	LD	A,#7		; Set UZI Read Fcn
	CALL	RdWrt		;  Do the work
	RET	C		; ..exit if Error
	OR	A		; 0 bytes Read?
	JR	Z,XErr1		; ..Return Error if Yes (EOF)
	SUB	#128		; A full 128 bytes Read?
	RET	Z		;   return Ok if Yes
	LD	DE,(dmaadr)
	ADD	HL,DE		; Else offset to byte after end
Feof:	LD	(HL),#0x1A	;  stuff EOF in case of text
	INC	HL
	INC	A
	JR	NZ,Feof
	RET			;   exit with OK status

;.....
; 14 - Write a "Sector" from DMA Address	{write (curFil, dmaadr, 128);}

Write:	LD	A,#8		; Set UZI Write Fcn
	CALL	RdWrt		;  Do the work
	RET	C		; ..exit if Error
	SUB	#128		; Good Write?
	RET	Z		;   return Ok if Yes
XErr1:	SCF
	JR	XErr		;  Else Return Error

; Common Read/Write Support Routine

RdWrt:	LD	DE,#128		; 1 "Sector" char
			; Entry Point accessed by Search Next (BDos)
RdWrt0:	PUSH	DE
	LD	HL,(dmaadr)	;  from here
	PUSH	HL
	LD	HL,(curFil)	;   to this file
	PUSH	HL
	;    A = R/W Fcn #
	CALL	syscall		;     Execute!
	POP	BC		; Clear Stack
	POP	BC
	POP	BC
	LD	A,L		; Shuffle possible byte quantity
	RET	NC		; ..return if No Error
XErr:	LD	A,#0x01		; Else Signal Error (keeping Carry)
	RET

;==========================================================
;		 Bios Support Utilities
;==========================================================
; Execute ioctl Function on STDIN
; Enter: HL = Addr of Parm Block
;	 DE = ioctl Function to execute
; Exit : None
; Uses : AF,BC,DE,HL

IoCtl:	PUSH	HL		; &buf
	PUSH	DE		;  ioctl fcn
	LD	E,#STDIN		;   fd
	PUSH	DE
	LD	A,#29		;    Fuzix ioctl Fcn #
	CALL	syscall		;     Execute!
	POP	BC		; Clean Stack
	POP	BC
	POP	BC
	RET

;- - - - - - - - - - Data Structures - - - - - - - - -

dph:	.dw	0		; Ptr to Skew Table
	.dw	0,0,0		; Scratch Words for BDos use
	.dw	dir		; Ptr to Directory Buffer
	.dw	dpb		; Ptr to DPB
	.dw	0		; Ptr to Disk Checksum Buffer
	.dw	0		; Ptr to ALV Buffer


dpb:	.dw	64		; Dummy Disk Parameter Block
	.db	4
	.db	15
	.dw	0x0FFFF
	.dw	1023
	.db	0x0FF,0
	.db	0,0,0,0

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
	ret	nc
	; Error case - HL is errno code not result
	ld	hl,#0xffff
	ret
ttTermios:
	.ds	20		; Working TTY Port Settings
ttTermios0:
	.ds	20		; Initial TTY Port Settings

dir:	.ds	128		; Directory Buffer (can cut to 48 bytes?)

	.ds	64		; Mini stack for application entry
AppStack:

BIOSIZ	.EQU	.-__bios
CPMSIZ	.EQU	.-__bdos

; To keep the linker happy
	.area _INITIALIZED
	.area _INITIALIZER
	.area _DATA
	.area _BSS
;       .end

