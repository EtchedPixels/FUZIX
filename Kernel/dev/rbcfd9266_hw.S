;:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
; D-X Designs Pty Ltd P112 Floppy disk Routines
;       Copyright (C) 1998 by Harold F. Bower
;:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
; 2015-01-17 Will Sowerbutts: Ported to sdas/Fuzix from UZI-180
; 2017-01-21 Will Sowerbutts: Improvements for reliable operation at 6MHz
;
; Reworked a bit for the FDC9266 and latch on the RBC/N8VEM hardware

	.z180

        ; imported symbols
        .export map_kernel
        .export map_proc_always
        .export _devfd_dtbl
	.export _plt_idle

#include "../build/kernel.def"
#include "../cpu-z80u/kernel-z80.def"


;------------------------------------------------------------------------------
	.code

;    FDC_MSR    - Main Status Register (Read)
;       7 6 5 4 3 2 1 0                         (Write)
;
;       7 6 5 4 3 2 1 0                         (Read)
;       | | | | +-+-+-+-- Drives Seeking (0=B0 Set, 1=B1 Set,.. 3=B3 Set)
;       | | | +---------- 1 = Command In Progress, 0 = Command Ended
;       | | +------------ 1 = Non-DMA Execution,   0 = DMA Execution
;       | +-------------- 1 = Read,                0 = Write
;       +---------------- 1 = Request for Master,  0 = Internal Execution
;
;    FDC_DATA   - Data/Command Register         (Read/Write)
;                               (Byte Writes/Reads)
;
;    FDC_DOR (write) - Latch
;	7 6 5 4 3 2 1 0
;       | | | | | | | +-------  TC
;       | | | | | | +---------- 1 = Motor on
;       | | | | | +------------ 1 = 3.5/5.25" disk 2 = 8" (or high density)
;       | | +-+-+-------------- P2-P0 (Precomp - set 125ns 100)
;       | +-------------------- 1 = high density 0 = low density
;	+---------------------- 1 = active, 0 = in reset
;
;    FDC_DOR (read)
;	7 6 5 4 3 2 1 0
;       | | | | | | | +-------- DC
;       +-+-+-+-+-+-+---------- 0
;
;-------------------------------------------------------------
MONTIM  .equ    250             ; Motor On time (Seconds * TICKSPERSEC)

; Offsets into _devfd_dtbl
oFLG    .equ    0               ; logged:  0 = Not Logged, 1 = Drive Logged
oPRM1   .equ    1               ; cbyte0:  Step Rate (B7-4), HUT (3-0)
oPRM2   .equ    2               ; cbyte1:  Hd Load in 4mS steps (0=infinite)
oGAP3   .equ    3               ; gap3:    Gap 3 Length for Read
oSPT    .equ    4               ; spt:     Sectors-per-Track
oSEC1   .equ    5               ; sector1: First Sector Number
oFMT    .equ    6               ; format:  Bit-mapped Format byte
oSPin   .equ    7               ; spinup:  Spinup delay (1/20-secs)
oTRK    .equ    8               ; curtrk:  Current Head Position (Track)
oNCYL   .equ    9               ; ncyl:    Number of cylinders
TBLSIZ  .equ    10              ; sizeof() entry in _devfd_dtbl

;-------------------------------------------------------------
; Determine if the controller exists and a drive is attached
;       fdInit (int minor);
; Enter: Drive Minor # is on Stack
; Exit : hl = 0 if All is Ok, Non-Zero if Error

_devfd_init:
        xor     A
        ld      (motim),a       ; Mark Motors as initially OFF
        ld      (hd),a          ;  and initially Head #0

        ld	a, #0x20        ; increase delay time for init
        ld	(dlyCnt),a

        pop     hl              ; Return Addr
        pop     bc              ;  minor (in C)
        push    bc              ;   Keep on Stack for Exit
        push    hl
	push	iy		; Must be saved for the C caller
        ld      a,C
        ld      (drive),a       ; Save Desired Device
        cp      #4              ; Legal?
        jr      nc,NoDrv        ; ..Exit if Error
        call    ActivA          ; Else force Reset (B2=0)
        ld      b,#0
indel1: djnz    indel1          ;    (settle)
        call    Activ8          ;  then bring out of Reset
indel2: djnz    indel2          ;    (settle, B already =0)
        in      a,(FDC_MSR)
        cp      #0x80           ; Do we have correct Ready Status?
        jr      nz,NoDrv        ; ..exit Error if Not

        ld      a,(drive)
        call    GetPrm          ; Pt to this drive's table entry
        push    hl
        pop     iy
        ld      (iy + oFLG), 0  ; Ensure drive is Unlogged
        call    Spec            ; Set Controller Params
        jr      c,NoDrv         ; ..Error if TimeOut, Else..
        call    Recal           ; Recalibrate (home) Drive
        jr      nz,NoDrv        ; ..Error if it failed
        ld      (iy + oFLG), 1  ; Mark drive as active
        ld      hl,#0           ; Load Ok Status
	pop	iy
        ret

NoDrv:  ld      hl,#0xFFFF      ; Set Error Status
	pop	iy
        ret

;-------------------------------------------------------------
; This routine Reads/Writes data from buffer trying up to 15 times
; before giving up.  If an error occurs after the next-to-last
; try, the heads are homed to force a re-seek.
;
; Enter: Drive Minor # is on Stack.  Entry Point sets Read/Write Flag
; Exit :  A = 0, Zero Set if Ok, A <> 0, Zero Reset if Errors
;        (also returns H = 0 and L set to A for compatibilty with C code)
; Uses : af,hl

_devfd_read:
        ld      a,#1
        .byte 0x21 ;..Trash hl, fall thru..
_devfd_write:
        ld      a,#0            ; has to be two bytes -- do not optimise to xor a!
        ld      (rdOp),a

        pop     hl              ; Return Addr
        pop     bc              ;  minor (->C)
        push    bc              ;   Keep on Stack for Exit
        push    hl

	push	iy

        ld      a,C
        ld      (drive),a       ; Save Desired Device

        call    Setup           ; Set up subsystem

        ld      a,#15           ; Get the maximum retry count
Rwf1:   ld      (rwRtry),a
        ld      d,#0xFF         ;  (Verify needed)
        call    SEEK            ; Try to seek to the desired track
        jr      nz,Rwf2         ; ..jump if No Good

        ld      a,(rdOp)
        or      a               ; Read operation?
        ld      a,#0x05         ; Load DP8473 Write Command
        jr      z,SWrite        ; No, must be Write
        inc     a               ; (A=06H) Load DP8473 Read Command
SWrite: or      #0x40           ;  Set MFM Mode Bit
        push    bc              ;   Save Regs
        ld      c,a             ;  Save
        ld      b,#9            ; Read/Write Comnds are 9 bytes

        ld      a,(eot)         ; Get Last Sctr #
        push    af              ;  (save for Exit)
        ld      a,(sect)        ; Get Desired Sector #
        ld      (eot),a         ;  make last to Read only one Sector

        ld      hl,(_devfd_buffer)
        call    FdCmd           ; Execute Read/Write

        pop     af              ; Restore Last Sctr #
        ld      (eot),a         ;  to Comnd Blk

        ld      a,(st1)         ; Get Status Reg 1
        and     #0x34           ; Return Any Error Bits
        pop     bc              ; Restore Regs
        ld      (_devfd_error),a        ;  (store Error bits)
        jr      z,FhdrX         ; ..jump to return if No Errors

Rwf2:   ld      a,(rwRtry)      ; Get retry count
        cp      #2              ; Are we on Next to last try?
        call    z,Recal         ;  Return to Track 0 if so
        ld      a,(rwRtry)      ;   and re-fetch try count
        dec     a               ; Do we have more retries left?
        jr      nz,Rwf1         ; ..jump to try again if more tries remain

        or      #0xFF           ; Else show Error
FhdrX:  ld      l,a
        ld      h,#0
	pop	iy
        ret                     ;   and Exit

;-------------------------------------------------------------
; SPEC - Do a Specify Command, setting Step Rate and Head
;  Load/Unload Time.  Settings require tailoring to Drive.
;
; Enter: iy -> Drive Table entry for current drive
; Exit : Nothing
; Uses : af,bc

Spec:   call    WRdyT           ; Wait for RQM (hope DIO is Low!), Disable Ints
        ret     c               ; ..Error if Timed Out
        ld      a,#0x03         ; Do an FDC Specify Command
        out     (FDC_DATA),a

        call    WRdyT
        ret     c               ; ..Error if Timed Out
        ld      a,(iy + oPRM1)  ;  first Rate Byte (Step Rate, HUT)
        out     (FDC_DATA),a

        call    WRdyT
        ret     c               ; ..Error if Timed Out
        ld      a,(iy + oPRM2)  ; Get Head Load Time
        add     a,a             ;  Shift value left (doubles count)
        inc     a               ;   Set LSB for Non-DMA Operation
        out     (FDC_DATA),a
        xor     a               ;  Return Ok Flag
        ret

;-------------------------------------------------------------
; RECAL  Recalibrate Current "drive" (moves heads to track 0).
; Enter : iy -> Current Drive Table Entry
;         Variable "drive" set to desired floppy unit
; Return:  A = 0 if Ok, NZ if Error.  Flags reflect A
; Uses  : af            All other Registers Preserved/Not Affected
;
; NOTE: bc Must be preserved by this routine.

Recal:  ld      a,(hd)          ; Get head #
        add     a,a
        add     a,a             ;  Shift to B3
        push    hl              ;   (preserve regs)
        ld      hl,#drive
        or      (hl)            ;   add Drive bits
        pop     hl              ;    (restore regs)

        ld      (hdr),a         ;   in Command Block
        ld      a,#3            ; Give this 3 chances to Home
Recal1: ld      (retrys),a
        push    bc              ; Save needed regs
        push    hl
        ld      bc,2*256+7      ;   (2-byte Recalibrate Comnd = 07H)
        call    FdCmd           ;  execute Recalibrate
        call    FdcDn           ; Clear Pending Ints, Wait for Seek Complete
        pop     hl              ;   (restore regs)
        pop     bc
        and     #0x10           ; Homed?  (B4=1 if No Trk0 found)
        jr      z,RecOk         ; ..jump to Store if Ok
        ld      a,(retrys)
        dec     a               ; Any trys left?
        jr      nz,Recal1       ; ..loop if So
        dec     a               ; Else set Error Flag (0-->FF)
        ret

RecOk:  xor     a               ; Get a Zero (track / flag)
        ld      (iy + oTRK),a   ;  Set in Table
        ret                     ;   and return

;-------------------------------------------------------------
; READID - Read the first Valid Address Mark on a track.
;
; Enter : "hdr" byte set in Command Blk
; Return:  A = 0 if Ok, NZ if Error.  Flags reflect A
; Uses  : af            All other Registers Preserved/Not Affected

ReadID: ld      a,#0x4a         ; Load ReadID Command + MFM Mode byte
        push    bc              ; Save regs
        ld      b,#2            ;  two bytes in ReadID Command
        ld      c,a             ;   move Command to C
        call    FdCmd           ; Activate DP8473 FDC

        ld      a,(st1)         ; Get Status Reg 1
        and     #0x25           ;  Return Any Error Bits
        pop     bc              ;   Restore regs
        ret                     ;  ..and quit

;-------------------------------------------------------------
; SETUP - Set parameters necessary to Read/Write from Active Drive
; Enter: Variable "drive" set to current Drive (0..3)
;        Variables _devfd_track and _devfd_sector set to desired block address
; Exit : iy -> Drive's Table entry
; Uses : af,bc,hl

Setup:  ld      a,(drive)
        push    af
        call    GetPrm          ; Pt to Current Drive's Table
        push    hl
        pop     iy
        pop     bc
        ld      a,(active)      ; Get current Activation Byte
        and     0x80           ; keep reset bit
	or	0x20		; 125ns precomp
	push	af		; Save the work so far
;        call    Activ8          ;    save new byte and activate FDC
        ld      a,(_devfd_track)        ; Get Host Track #
        srl     a               ;  Div by 2 (LSB to Carry)
        ld      (trk),a         ;   Physical Track # to Comnd Blk
        ld      a,#0
        adc     a,a             ; LSB becomes Head #
        ld      (hd),a          ;  save in Comnd Blk
        add     a,a
        add     a,a             ; Shift to B3
        ld      hl,#drive
        or      (hl)            ;  add Drive bits
        ld      (hdr),a         ;   Save in Comnd Blk
        ld      a,(iy + oGAP3)
        ld      (gpl),a         ; Set Gap3 Length
        ld      a,(iy + oSPT)
        ld      (eot),a         ;  Final Sector # on Trk
        ld      a,(iy + oFMT)
        and     #3              ; B0/1 of Format byte is Sector Size
        ld      (rsz),a         ;  save in Comnd Blk
        ld      a,#0xFF
        ld      (dtl),a         ;   Set Data Length code
        ld      a,(_devfd_sector)
        add     a,(iy + oSEC1)  ;    Offset Sector # (base 0) by 1st Sector #
        ld      (sect),a        ;     set in Comnd Blk
        xor	a               ;  (Preset Hi 500 kbps, 3.5 & 5.25" Rate)
        bit     7,(iy + oFMT)   ; Hi (500 kbps) Speed?
        jr      nz,StSiz0       ; ..jump if Hi-Density/Speed to Set if Yes
        ld      a,(iy + oFMT)
        and     #0x0c           ; Else Get Drive size
        cp      #0x08           ; 5.25"?
        ld      a,#0x44         ;  (Prepare for 250 kbps)
        jr      nz,StSiz0       ; ..jump if Not 5.25" w/Rate Set
        bit     4,(iy + oFMT)   ; Hi-Density capable drive?
        ld      a,#0x44         ;  (Prepare for 250 kbps)
        jr      z,StSiz0        ; ..jump if No
        ld      a,#0x44         ; Else set to 300 kbps (@360 rpm = 250kbps)
				; Mini, high density.
StSiz0:
	pop	bc		; Get bits back
	or	b		; Merge in the rates
	call	Activ8		; Set them all

        ld      d,a             ;   preserve Rate bits
#ifdef CPU_Z180
        in0     a,(0x1F)        ; Read Z80182 CPU Cntrl Reg (B7=1 if Hi Speed)
        rla                     ;  Speed to Bit Carry..Turbo?
        ld      a,CPU_CLOCK_KHZ/1000	; (Get Processor Rate in MHz)
        jr      c,StSiz1        ;  ..jump if Turbo for longer delay
#else
	ld	a,CPU_CLOCK_KHZ/1000	; (Get Processor Rate in MHz)
#endif
        srl     a               ; Divide rate by 2
StSiz1: bit	2,d		; 500 kb/s (Hi-Speed) Rate (D=0)?
        jr      nz,StSiz2       ; ..jump if Not
        ld      a,#1            ;  Else minimum delay for "High-Speed"
StSiz2: ld      (dlyCnt),a      ;   save delay count
        ret

;-------------------------------------------------------------
; SEEK - Set the Track for disk operations and seek to it.
;
; Enter :  A = Desired Track Number
;          D = Verify flag (0=No, FF=Yes)
; Return:  A = 0, Zero Flag Set (Z) if Ok, A <> 0 Zero Clear (NZ) if Error
; Uses  : af            All other Registers Preserved/Not Affected

SEEK:   push    hl              ; Save Regs used here
        push    de
        push    bc

        ld      a,(trk)         ;  Get Track #
        cp      (iy + oTRK)        ; Is desired Track same as last logged?
        ld      (iy + oTRK),a      ;  (set as if we made it there)
        jr      nz,SEEKNV       ; ..jump if Not Same
        inc     d               ; Else Set to No Verify (FF->0)
SEEKNV: ld      a,#4            ; Get the maximum Retry Count
SEEK1:  ld      (retrys),a      ;  save remaining Retry Count
        ld      bc,#3*256+0x0F  ;   (3-byte Seek Command = 0FH)
        call    FdCmd           ; Execute the Seek
        call    FdcDn           ; Clear Pending Int, wait for Seek Complete

        and     #0xE0
        cp      #0x20
        jr      nz,SEEK2        ;;
        
        and     #0x40           ; Set NZ if Abnormal Termination

        ld      b,a             ;; Save Seek Status
        ld      a,(trk)         ;; Check track #
        cp      c               ;;  Same track?
        jr      nz,SEEK2        ;;   Jump to Retry if NOT
        ld      a,b             ;; Restore Seek Status

        inc     d               ; Are we Verifying (FF -> 0)?
        call    z,ReadID        ;   Read next ID Mark if So
        dec     d               ;    (Correct for Test, 0 -> FF)

        or      a               ; Set Status (Seek Status if No ReadID)
        jr      z,SEEKX         ; ..exit if Ok

SEEK2:  ld      a,(retrys)      ; Else get trys remaining
        dec     a               ; Any left (80-track could need two)?
        jr      nz,SEEK1        ; ..loop to try again if More
        dec     a               ; Else set Error Flag (0->FF)

SEEKX:  pop     bc              ; Restore Regs
        pop     de
        pop     hl
        ret


;-------------------------------------------------------------
; Check for Proper Termination of Seek/Recalibrate Actions by
;  executing a Check Interrupt Command returning ST0 in A.
; Enter: None.  Used after Seek/Recalibrate Commands
; Exit : A = ST0 Result Byte, C = PCN result byte
; Uses : af and C.  All other registers preserved/unused

FdcDn:  push    hl              ; Don't alter regs
FdcDn0: call    WRdy1
        ld      a,#8            ; Sense Interrupt Status Comnd
        out     (FDC_DATA),a
        call    WRdy1
        in      a,(FDC_DATA)    ; Get first Result Byte (ST0)
        ld      l,a
        cp      0x80           ; Invalid Command?
        jr      z,FdcDn0        ; ..jump to exit if So
        call    WRdy1
        in      a,(FDC_DATA)          ; Read Second Result Byte (Trk #)
        ld      c,a             ; ..into C
        ld      a,l
        bit     5,a             ; Command Complete?
        jr      z,FdcDn0        ; ..loop if Not
        pop     hl
        ret

;-------------------------------------------------------------
; MOTOR CONTROL.  This routine performs final selection of
; the drive control latch and determines if the Motors are
; already spinning.  If they are off, then the Motors are
; activated and the spinup delay time in tenths-of-seconds
; is performed before returning.
;
; Enter : None
; Return: None
; Uses  : hl.  Remaining Registers Preserved/Not Affected

Motor:  push    af              ; Save Regs
        push    bc
        ld      a,#MONTIM       ; Get motor timeout
        ld      (motim),a       ; Reset the countdown timer
        ld      a,(active)      ; Load current DOR contents
	bit	1,a		; Check motor on ?
	jr	nz,MotorX	; If so no wait
	or	#2		; Motor on
        call    Activ8          ; Send to FDC DOR, update active
        ; TODO this is a busy loop -- we should set a timer and yield
        ld      a,(drive)       ; Get Current drive
        call    GetPrm          ;  Pt to Param table
        ld      bc,#oSPin
        add     hl,bc           ;   offset to Spinup Delay
        ld      a,(hl)          ;    Get value
        ld      (mtm),a         ;     to GP Counter
        ei                      ; Ensure Ints are ABSOLUTELY Active..
;
;	FIXME: this is wrong on two levels
;	#1 We shouldn't rely upon an IRQ (we can busy wait too)
;	#2 The timers are set in 1/20ths but it's not clear everyone is
;	using 1/20ths for the IRQ call (See p112)
;
MotoLp:	push	de
	call	_plt_idle
	pop	de
	ld      a,(mtm)         ;  ..otherwise, loop never times out!
        or      a               ; Up to Speed?
        jr      nz,MotoLp       ; ..loop if Not
        di                      ; No Ints now..
MotorX: pop     bc
        pop     af              ; Restore Reg
        ret

;-------------------------------------------------------------
; Wait for FDC RQM to become Ready with Timeout indicator.
; Timeout Length is arbitrary and depends on CPU Clock Rate.

WRdyT:  ;DI                     ; No Ints while we are doing I/O
        ld      bc,#30000       ; << Arbitrary >>
WRdyT0: dec     bc
        ld      a,b
        or      c               ; Timed Out?
        scf                     ;  (set Error Flag in case)
        ret     z               ; ..return Error Flag if Yes
        in      a,(FDC_MSR)     ; Read Status Reg
        and     #0x80           ; Interrupt Present (also kill Carry)?
        ret     nz              ; ..return Ok if Yes
        jr      WRdyT0          ;  ..else loop to try again
 
;-------------------------------------------------------------
; Return Pointer to Parameters of selected Drive
; Enter: A = Drive (0..3)
; Exit : hl -> Parameter entry of drive
; Uses : af,hl

GetPrm: push    de
        ld      de,#TBLSIZ      ; Entry Size
        ld      hl,#_devfd_dtbl ;  Init to table start
        inc     a
GetPr0: dec     a               ; End?
        jr      z,GetPrX        ; ..quit if Yes, Ptr set
        add     hl,de           ; Else step to next
        jr      GetPr0          ; ..loop til found

GetPrX: pop     de
        ret

;-------------------------------------------------------------
; This routine called at each Clock Interrupt.  It is used
; to provide any necessary timer/timeout functions.
; Enter: None.
; Exit : hl -> mtm byte variable
;        af - destroyed
; Uses : af,hl

_fd_tick:
        ld      hl,#motim       ; Point to FDC Motor-On timer
        ld      a,(hl)
        or      a               ; Already Timed out?
        jr      z,TDone         ; ..jump if Yes
        dec     (hl)            ; Else count down
        call    z,MotOff        ;   stop motors if timed out
TDone:  inc     hl              ; Advance ptr to watchdog/spinup timer (mtm)
        ld      a,(hl)
        or      a               ; Timed out?
        ret     z               ; ..quit if Yes
        dec     (hl)            ; Else count down
        ret                     ;   exit

;-------------------------------------------------------------
; Motor Off routine.  Force Off to delay on next select
; Enter: None. (Motoff)
;        A = FDC Device Control Reg bits (Activ8/ActivA)
; Exit : A = Current FDC_DOR Register / "active" byte settings
; Uses : af

MotOff: xor     a
        ld      (motim),a       ; Ensure Motors Marked as OFF
        ld      a,(active)      ; Get current settings
	and	#0x7D		; strip out motor bit
	or	#0x80		; out of reset
Activ8:
ActivA: ld      (active),a      ;    save
        out     (FDC_DOR),a     ;     and Command!
        ret

;-------------------------------------------------------------
; FDCMD - Send Command to 9266 FDC
; Enter:  B = # of Bytes in Command, C = Command Byte
;        hl -> Buffer for Read/Write Data (If Needed)
; Exit : af = Status byte
; Uses : af.  All other registers preserved/unused

FdCmd:  push    hl              ; Save regs (for Exit)
        push    bc
        push    de

        ; rewrite FdCmdXfer code so data flows in the correct direction
        ld      a,(rdOp)
        or      a
        ld      a,#0xA2         ; Load second byte of ini opcode (doesn't update flags)
        jr      nz,FdCiUpd      ; ... if read, skip increment
        inc     a               ; ... if write, A=0xA3, second byte of outi opcode
FdCiUpd:ld      (FdCiR1+1),a    ; update second byte of ini/outi instruction

        ; is the buffer in user memory?
        ld      a,(_devfd_userbuf)
        ld      d,a             ; store userbuf flag in D

        ; prepare the drive
        call    Motor           ; Ensure motors are On

        call    FdCmdXfer       ; Do the data transfer (using code in _COMMONMEM)

        ld      hl,#st0         ; Point to Status Result area
IsGo:   call    WRdy
        bit     4,a             ; End of Status/Result?
        jr      z,FdcXit        ; ..exit if So
        bit     6,a             ; Another byte Ready?
        jr      z,FdcXit        ; ..exit if Not
        ini                     ; Else Read Result/Status Byte
        jr      IsGo            ; ..loop for next
FdcXit: 
        pop     de              ; Restore Regs
        pop     bc
        pop     hl
        ret

;------------------------------------------------------------
; COMMON MEMORY
;------------------------------------------------------------

	.common

; inner section of FdCmd routine, has to touch buffers etc
FdCmdXfer:
        bit     0,d             ; Buffer in user memory?
        call    nz, map_proc_always

        ; send the command (length is in B, command is in C)
        push    hl              ; save pointer for possible Transfer
        ld      hl,#comnd       ; Point to Command Block
        ld      (hl),c          ;  command passed in C
        ld      c,#FDC_DATA     ;   FDC Data Port
OtLoop: call    WRdy            ; Wait for RQM (hoping DIO is Low) (No Ints)
        outi                    ; Output Command bytes to FDC
        jr      nz,OtLoop       ; ..loop til all bytes sent
        pop     hl              ; Restore Possible Transfer Addr
        jr      FdCiR2          ; start sampling MSR

; transfer loop
;
;
FdCiR1: ini                     ; *** THIS inSTRUCTION IS MODIFIED in PLACE to ini/outi
        ; ini  = ED A2
        ; outi = ED A3
FdCiR2: in      a,(FDC_MSR)     ; Read Main Status Register
	add	a,a
        jr	nc, FdCiR2      ; loop until interrupt requested
	add	a,a	        ; are we still in the Execution Phase?
        jp      m, FdCiR1       ; if so, next byte!

; tidy up and return
FdCmdXferDone:
        bit     0,d             ; Buffer in user memory?
        ret     z               ; done if not
        jp      map_kernel      ; else remap kernel and return

;-------------------------------------------------------------
; Wait for FDC RQM to become Ready, return DIO status in
; Zero Flag.  Pause before reading status port (~12 mS
; specified, some assumed in code).

WRdy:
WRdy1:  ld      a,(dlyCnt)      ; Get delay count
WRdy0:  dec     a               ;  count down
        jr      nz,WRdy0        ;   for ~6 uS Delay

WRdyL:  in      a,(FDC_MSR)     ; Read Main Status Register
        bit     7,a             ; Interrupt Present?
        ret     nz              ;  Return if So
        jr      WRdyL           ;   Else Loop

dlyCnt: .byte     CPU_CLOCK_KHZ/1000    ; Delay to avoid over-sampling status register

; FDC command staging area
comnd:  .byte	0               ; Storage for Command in execution
hdr:    .byte	0               ; Head (B2), Drive (B0,1)
trk:    .byte	0               ; Track (t)
hd:     .byte	0               ; Head # (h)
sect:   .byte	0               ; Physical Sector Number
rsz:    .byte	0               ; Bytes/Sector (n)
eot:    .byte	0               ; End-of-Track Sect #
gpl:    .byte	0               ; Gap Length
dtl:    .byte	0               ; Data Length

;------------------------------------------------------------
; DATA MEMORY
;------------------------------------------------------------
	.data

drive:  .byte	0               ; (minor) Currently Selected Drive
active: .byte	0               ; Current bits written to FDC_DOR

_devfd_sector:  .byte	0
_devfd_track:   .byte	0               ; LSB used as Head # in DS formats
_devfd_error:   .byte	0
_devfd_buffer:  .word	0
_devfd_userbuf: .byte	0

; DISK Subsystem Variable Storage
; FDC Operation Result Storage Area
st0:    .byte	0               ; Status Byte 0
st1:    .byte	0               ; Status Byte 1 (can also be PCN)
        .byte	0               ; ST2 - Status Byte 2
        .byte	0               ; RC - Track #
        .byte	0               ; RH - Head # (0/1)
        .byte	0               ; RR - Sector #
        .byte	0               ; RN - Sector Size

; -->>> NOTE: Do NOT move these next two variables out of sequence !!! <<<--
motim:  .byte	0               ; Motor On Time Counter                <<<--
mtm:    .byte	0               ; Floppy Spinup Time down-counter      <<<--

rdOp:   .byte	0               ; Read/write flag
retrys: .byte	0               ; Number of times to try Opns
rwRtry: .byte	0               ; Number of read/write tries
DRVSPD: .byte	0               ; Drive Speed
DRVSIZ: .byte	0               ; Drive Size
