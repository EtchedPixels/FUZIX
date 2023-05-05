********************************************************************
* dwload implementation for Dragon
*
* Copyright 2014-2015 Tormod Volden
* Copyright 2008 Boisy G. Pitre (parts taken from toolshed/dwdos)
*
* Distributed under the GNU General Public License, version 2 or later.
*
* For patching BASIC ROM (upper 8KB), replacing DLOAD command:
* lwasm --pragma=condundefzero -r -b -odwload.bin dwload.asm -DD32ROM
*
* Standalone, and using Becker interface
* lwasm --pragma=condundefzero -r -b -odwload.bin dwload.asm -DBECKER
*
* Build a "dweeb" for chainloading by a "normal" build:
* lwasm -r -b -odwload.bin dwload.asm -DDWEEB
*
* Example build for other address:
* lwasm -r -b -odwload.bin dwload.asm -DDWEEB -DRAMORG=0x600
********************************************************************


* dwload temporary RAM storage
noexec   equ   $10		inhibit exec after loading
bootflag equ   $11		zero if run from command line, $55 if autorun on boot
dnum     equ   $12		assigned drive number from server
sstack   equ   $1cd		backup of original stack
sector   equ   $1cf		counter, used when loading file contents, just below namebuf/DECB header
reqbuf   equ   namebuf-1
namebuf  equ   $1d1		fixed by BASIC read filename function
decbhdr  equ   namebuf		DECB headers get copied in here
startbuf equ   namebuf+8	dw input data buffer, can reuse buffer after DECB header slot
endbuf   equ   startbuf+256
tmpstack equ   $3fc		normally enough unused area below here (goes 30 bytes deep)

* ROM routines used
OUTCH    equ   $BCAB		print char A to screen
OUTCHR   equ   $B54A		print char A to DEVNUM
OUTSTR   equ   $90E5		print string at X+1 to DEVNUM
OUTNUM   equ   $957A		print number in D to DEVNUM
GETFNAM  equ   $B7AA		read file name and length into $1d1
OKPRMPT  equ   $8371		BASIC OK prompt, command loop
IOERROR  equ   $B84B		IO_error
RUNBASIC equ   $849F		run_basic, enable IRQs etc
BASVECT1 equ   $841F		reset BASIC vectors, reset stack etc
BASVECT2 equ   $83ED		initialize BASIC
SYSERR   equ   $8344		system error
BACKSPC  equ   $9A89		print a backspace to DEVNUM


         IFNDEF RAMORG
         IFDEF DWEEB
RAMORG   equ   $0E00
         ELSE
RAMORG   equ   $7400
         ENDC
         ENDC

         IFDEF D32ROM
Top      equ   $BE7F		Dragon 32 unused ROM space
         ELSE
Start    equ   RAMORG
         ENDC
 
* Entry point from command line (DLOAD or EXEC)
         IFDEF D32ROM
         org   $A049		DLOAD dispatch
         ELSE
         org   Start

         IFDEF DW4
         ldx   #goturbo
         ldy   #1
         lbsr  DWWrite
         ldx   #$A000
Spint    leax  -1,x
         bne   Spint
         bra   Entry
goturbo  fcb   $E6
         ENDC

         ENDC
* for later to see that we came from DLOAD command
         clr   <bootflag	clear our autoboot flag
         ldx   #dwtext-1
         jsr   OUTSTR		print string

Entry
         IFDEF DWEEB
         ldu   ,s               get stacked PC
         ldx   1,u
         stx   DWReadv
         ldx   3,u
         stx   DWWritev
         ldx   5,u
         stx   DoReadv
         ENDC

         orcc  #IntMasks	disable FIRQ, IRQ
* Set up MOOH for nx32 "emulation"
* The particular write order is for compatibility with Spinx-512
* where these addresses are used for segmented activation
         lda #3
         sta $FFA7
         deca
         sta $FFA6
         deca
         sta $FFA5
         deca
         sta $FFA4

* CoCo 1/2 Initialization Code (from toolshed/dwdos)
         ldx   #PIA1Base               PIA1
         IFDEF HWINIT
         clr   -3,x                    clear PIA0 Control Register A
         clr   -1,x                    clear PIA0 Control Register B
         clr   -4,x                    set PIA0 side A to input
         ldd   #$FF34
         sta   -2,x                    set PIA0 side B to output
         stb   -3,x                    enable PIA0 peripheral reg, disable PIA0
         stb   -1,x                    MPU interrupts, set CA2, CA1 to outputs
         clr   1,x                     $FF20 = DDR, motoroff
         clr   3,x                     $FF22 = DDR, sound disabled
         deca                          A = $FE after deca
         sta   ,x                      bits 1-7 are outputs, bit 0 is input on PIA1 side A
         lda   #$F8
         sta   2,x                     bits 0-2 are inputs, bits 3-7 are outputs on B side
         stb   1,x                     enable peripheral registers, disable PIA1 MPU
         stb   3,x                     interrupts and set CA2, CB2 as outputs
         ENDC
         ldb   #$02
         stb   ,x                      make RS-232 output marking
         IFDEF HWINIT
         aslb
         clr   -2,x
         bitb  2,x

         lda   #$37
         sta   PIA1Base+3

         lda   PIA0Base+3
         ora   #$01

         IFDEF D32ROM
         bra   dload2           continue in next dload hunk
eoclob1  equ   *		must be below or equal $A08B (FM error used by cloadm) !
         org   $a0f4
dload2
         ENDC

         sta   PIA0Base+3

         lda   PIA1Base+2
         anda  #$07
         sta   PIA1Base+2
         ENDC

         IFNDEF DWEEB
* Spin for a while so that the RS-232 bit stays hi for a time
Reset
         ldx   #$A000
Spin     leax  -1,x
         bne    Spin

* Request named object from DW server
         lda   <bootflag	coming from autoboot?
         lbne   autoreq		then use default name
         ENDC
         jsr   <$a5		peek at next character
*         beq   noname		end of line peeked
         suba  #'N		DLOADN ?
         sta   <noexec		0 = noexec
         bne   getn
         jsr   <$9f		read next char (the N)
* or here, after reading the zero character?
getn     jsr   GETFNAM		read file name and length into $1d1 = namebuf
         ldx   #namebuf-1	packet buffer start
         clr   ,x		zero MSB for Y
         ldy   ,x++		length of file name (16 bit)
         lbeq   noname		no file name given?
         ldb   -1,x		length of name (8 bit)
         clr   b,x		zero terminate name string (for error)
         inc   ,--x		1 = DriveWire OP_NAMEOBJ_MOUNT
         leay  2,y		length of DW packet, name length + 2

         IFDEF D32ROM
         IFNDEF HWINIT
         bra   dload2           continue in next dload hunk
eoclob1  equ   *		must be below or equal $A08B (FM error used by cloadm) !
         org   $a0f4
dload2
         ENDC
         ENDC

reqobj
         jsr   DWWrite
         ldx   #dnum		Our drive number variable
         clr   ,x
         leay  1,y		read one byte back (Y is 0 after DWWrite)
         jsr   DWRead		get drive number
         tst   ,x
         bne   ReadDECB		successful mount
         tst   <bootflag
         lbne  stealth		silent failure, BASIC OK prompt
         ldb   #$19		MO ERROR
         jmp   SYSERR		system error
autoreq  sta   <noexec          a is non null, autoexec
noname   ldx   #autoname
         ldy   #(autonend-autoname)	length of DW packet
         bra   reqobj

* named object has been mounted, proceed to read in file
ReadDECB
         ldx   #0000
         stx   sector
         ldu   #endbuf    * start with empty buffer
         ldx   #tmpstack  * our temporary stack location
         exg   x,s        * move stack here
         stx   sstack     * save original stack pointer
* read DECB header
nextseg  ldy   #5
         ldx   #decbhdr   * copy DECB header here
         jsr   copybuf    * moves x ahead
         lda   -5,x       * DECB segment header starts with zero
         beq   ml         * normal data segment

         cmpa  #$55       * Dragon DOS file?
         bne   noddd
         leay  4,y        * remaining header bytes (y is 0 here)
         jsr   copybuf

         dec   -8,x       * Dragon DOS BASIC = 1
         bne   ddbin
         leax  -1,x       * length at x-5
         bra   ldbas

ddbin    ldy   -5,x       * length
         ldx   -7,x       * load address
         jsr   copybuf    * read whole file

         ldx   #decbhdr+8 * exec address ptr + 2
         bra   oldsp
*         sty   #decbhdr+4 * clear out new sp
*         ldx   #decbhdr+8 * exec address ptr + 2
*         bra   endseg

noddd    inca
         lbne  ErrDWL     * must be $FF otherwise
         leay  1,y        * Y is 0 after copybuf
         cmpy  sector     * only first sector can be BASIC header
         bne   endseg     * otherwise end flag
         cmpu  #startbuf+5 * must be first segment also
         bne   endseg
* loading DECB BASIC file
* bytes 4 and 5 are actually part of the program
         leau  -2,u       * u was 5 bytes into read buffer here

* at this point x is past 5-byte header, u is 3 bytes into first 256-bytes block
* for Dragon DOS x is 8 bytes into header, u is 9 bytes into first 256-bytes block
ldbas    ldy   -4,x       * read whole BASIC program
         ldx   <$19
         jsr   copybuf
         jsr   prbang
* set up BASIC pointers and finish
         stx   <$1b       * end of BASIC program
         stx   <$1d
         stx   <$1f
         tst   <noexec
	 lbeq  $B72D      * print OK, run BASVECT1, BASVECT2, readline
         jsr   BASVECT1   * BasVect1 reset BASIC stack etc
         jsr   BASVECT2   * BasVect2 initialize BASIC
         jmp   RUNBASIC   * run_basic

ml       ldy   -4,x       * DECB segment length
         ldx   -2,x       * DECB segment loading address
         jsr   copybuf
         bra   nextseg

prbang   jsr   BACKSPC    * print backspace to devnum
         lda   #'!        * print bang
         jmp   OUTCHR     * print to devnum

endseg
         ldu   -4,x       * new stack pointer specified?
         bne   setsp
oldsp    ldu   sstack     * otherwise restore original SP
setsp    tfr   u,s
         bsr   prbang
         IFNDEF FUZIX
         andcc #~IntMasks * enable interrupts
         ENDC
         ldx   -2,x       * exec address
         tfr   x,d
         inca
         beq   retbas     * return to basic if exec address $FFxx
         stx   <$9d       * save BASIC EXEC address
         tst   <noexec
         beq   retbas
         IFDEF FUZIX
         jmp   ,x         * don't use stack and don't look back
         ELSE
         jsr   ,x         * run loaded program
         ENDC
retbas   rts

* vector table for chainloaders etc
         fdb   DWRead
         fdb   DWWrite
         fdb   DoRead
         fdb   0

dwtext   fcc   /DWLOAD/
         fcb   $0D,0

         IFDEF D32ROM
eoclob2  equ   *		must be below or equal $A1CC !

* Hook DWLOAD into boot code
* Use "non-invasive" location to avoid issues with programs copying 
* parts of the boot code to perform hardware initialisation.
* Use JSR to record where we are coming from.
         org   $b466		overwrite end of boot code
         jsr   fromboot		is originally a jump to command loop

* rest of code goes to top of ROM

         org Top

fromboot
         ldu   ,s++		check return address (and drop it)
         cmpu  #$b469		ROM code location (not a copy)
         bne   gocmdl		return if run from a copied code segment
         cmpx  #$B44F		coldstart sets this, warmstart doesn't
         bne   gocmdl		return if warmstart

* check for SHIFT key
chkshift lda   $FF02		save PIA
         ldb   #$7F
         stb   $FF02
         ldb   $FF00
         sta   $FF02		restore PIA
         comb
         andb  #$40
	 bne   gocmdl		return if shift key pressed

         incb			B was 0 before
         stb   <bootflag	our autoboot flag = 1
         jsr   Entry
         bra   gocmdl
         ENDC

autoname fcb   $01		OP_NAMEOBJ_MOUNT
         fcb   (autonend-autoname-2)		length of name string
         IFDEF FUZIX
         fcc   /fuzix.bin/
         ELSE
         fcc   /AUTOLOAD.DWL/
         ENDC
autonend

ErrDWL   lbra  IOERROR		BASIC IO ERROR
stealth  andcc #~IntMasks	enable interrupts
gocmdl   jmp   OKPRMPT		BASIC OK prompt
* does SP need to be restored on error?

* copy y chars from read buffer to x, updates u
copybuf
copyl    cmpu  #endbuf
         bne   copym
* fill up buffer via DW - resets buffer pointer u
         pshs  x,y
         ldx   sector
         ldy   #startbuf
         tfr   y,u
         bsr   DoRead
         bcs   ErrDWL
         bne   ErrDWL
         leax  1,x
         stx   sector
         lda   #'.		print dot
         jsr   OUTCHR
         puls  x,y
copym    lda   ,u+
         IFDEF FUZIX
         clr   $FFBF		map in bank 0 on nx32
         ldb   #64		MMU enable on MOOH
         stb   $FF90
         ENDC
         sta   ,x+
         IFDEF FUZIX
         clr   $FFBE
         clr   $FF90
         ENDC
         leay  -1,y
         bne   copyl
         rts


* also used by DWRead/DWWrite
PIA0Base equ   $FF00
PIA1Base equ   $FF20
IntMasks equ   $50

         IFDEF DWEEB
DoRead   fcb   $7E
DoReadv  fdb   0
DWRead   fcb   $7E
DWReadv  fdb   0
DWWrite  fcb   $7E
DWWritev fdb   0
         end   Entry * or Start
         ENDC

* below code is taken from toolshed/dwdos

DoRead
         lda   <dnum		our drive number
         clrb			LSN bits 23-16
         pshs  d,x,y
         lda   #OP_READEX
ReRead   pshs  a
         leax  ,s
		 ldy   #$0005
		 lbsr  DWWrite
		 puls  a

		 ldx   4,s			get read buffer pointer
		 ldy   #256			read 256 bytes
		 ldd   #133*1		1 second timeout
		 bsr   DWRead
         bcs   ReadEx
         bne   ReadEx
* Send 2 byte checksum
		 pshs  y
		 leax  ,s
		 ldy   #2
		 lbsr  DWWrite
		 ldy   #1
		 ldd   #133*1
		 bsr   DWRead
		 leas  2,s
		 bcs   ReadEx
                 bne   ReadEx
* Send 2 byte checksum
		 lda   ,s
		 beq   ReadEx
		 cmpa  #E_CRC
		 bne   ReadErr
		 lda   #OP_REREADEX
		 clr   ,s
		 bra   ReRead  
ReadErr  comb
ReadEx	 puls  d,x,y,pc

* Used by DWRead and DWWrite
CoCo     equ   1
NOINTMASK equ  1
Level    equ   1
Carry    equ   1
DAT.Regs equ   $FFA0
E$NotRdy equ   246
Vi.PkSz  equ   0
V.SCF    equ   0

         IFEQ  DW4-1
         use   dw4read.s
         use   dw4write.s
         ELSE
         use   ../dev/drivewire/dwread-6809.s
         use   ../dev/drivewire/dwwrite-6809.s
         ENDC

         use   ../dev/drivewire/dw-6809.def

csize    equ   *-Entry
eom      equ   *-Top

* Fill pattern

         end   Entry
         ENDC

