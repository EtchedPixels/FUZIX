;:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
; D-X Designs Pty Ltd P112 Floppy disk Routines
;       Copyright (C) 1998 by Harold F. Bower
;:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
; 2015-01-17 Will Sowerbutts: Ported to sdas/Fuzix from UZI-180
; 2017-01-21 Will Sowerbutts: Improvements for reliable operation at 6MHz

        .module devfd_hw

CPU_Z180	.equ	Z80_TYPE-2
.ifeq	CPU_Z180
	.z180
.endif

        ; imported symbols
        .globl map_buffers
        .globl map_kernel_restore
        .globl map_proc_always
        .globl _devfd_dtbl
	.globl _plt_idle

        ; exported sybols
        .globl _devfd_init
        .globl _devfd_read
        .globl _devfd_write
        .globl _devfd_track
        .globl _devfd_sector
        .globl _devfd_error
        .globl _devfd_buffer
        .globl _devfd_userbuf
        .globl _fd_tick

        .include "../../build/kernel.def"
        .include "../../cpu-z80/kernel-z80.def"


;------------------------------------------------------------------------------
        .area _CODE

;    092        - Drive Control Register        (Write Only)
;       7 6 5 4 3 2 1 0
;       | | | | | | +-+-- Drive (00=0, 01=1, 10=2, 11=3)
;       | | | | | +------ 1 = Normal Opn, 0 = Reset Controller
;       | | | | +-------- 1 = Enable DMA Pins, 0 = Disable DRQ,DAK,INT pins
;       | | | +---------- 1 = Enable Drive 0 Motor
;       | | +------------ 1 = Enable Drive 1 Motor
;       | +-------------- 1 = Enable Drive 2 Motor
;       +---------------- 1 = Enable Drive 3 Motor

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
;    FDC_CCR    - Data Rate Register (Write, not present on older FDCs)
;       7 6 5 4 3 2 1 0                         (Write)
;       | | | | | | +-+-- 00=500 kb/s, RPM/LC Hi, 01=250/300 kb/s (RPM/LC Lo)
;       | | | | | |       10=250 kb/s, RPM/LC Lo, 11=1000 kb/s (RPM/LC Hi/Lo)
;       +-+-+-+-+-+------  (Not Used)
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
oSPIN   .equ    7               ; spinup:  Spinup delay (1/20-secs)
oTRK    .equ    8               ; curtrk:  Current Head Position (Track)
oNCYL   .equ    9               ; ncyl:    Number of cylinders
TBLSIZ  .equ    10              ; sizeof() entry in _devfd_dtbl

;-------------------------------------------------------------
; Determine if the controller exists and a drive is attached
;       fdInit (int minor);
; Enter: Drive Minor # is on Stack
; Exit : HL = 0 if All is Ok, Non-Zero if Error

_devfd_init:
        XOR     A
        LD      (motim),A       ; Mark Motors as initially OFF
        LD      (hd),A          ;  and initially Head #0

        LD A, #0x20             ; increase delay time for init
        LD (dlyCnt),A

        POP     HL              ; Return Addr
        POP     BC              ;  minor (in C)
        PUSH    BC              ;   Keep on Stack for Exit
        PUSH    HL
	PUSH	IY		; Must be saved for the C caller
        LD      A,C
        LD      (drive),A       ; Save Desired Device
        CP      #4              ; Legal?
        JR      NC,NoDrv        ; ..Exit if Error
        CALL    ActivA          ; Else force Reset (B2=0)
        LD      B,#0
indel1: DJNZ    indel1          ;    (settle)
        CALL    Activ8          ;  then bring out of Reset
indel2: DJNZ    indel2          ;    (settle, B already =0)
        IN      A,(FDC_MSR)
	CP	#0xD0		; Came out of reset with a pending interrupt
	JR	NZ, NoPend
	IN	A,(FDC_DATA)	; Eat the status
	IN	A,(FDC_MSR)	; Check again
NoPend:
        CP      #0x80           ; Do we have correct Ready Status?
        JR      NZ,NoDrv        ; ..exit Error if Not

        LD      A,(drive)
        CALL    GetPrm          ; Pt to this drive's table entry
        PUSH    HL
        POP     IY
        LD      oFLG(IY), #0    ; Ensure drive is Unlogged
        CALL    Spec            ; Set Controller Params
        JR      C,NoDrv         ; ..Error if TimeOut, Else..
        CALL    Recal           ; Recalibrate (home) Drive
        JR      NZ,NoDrv        ; ..Error if it failed
        LD      oFLG(IY), #1    ; Mark drive as active
        LD      HL,#0           ; Load Ok Status
	POP	IY
        RET

NoDrv:  LD      HL,#0xFFFF      ; Set Error Status
	POP	IY
        RET

;-------------------------------------------------------------
; This routine Reads/Writes data from buffer trying up to 15 times
; before giving up.  If an error occurs after the next-to-last
; try, the heads are homed to force a re-seek.
;
; Enter: Drive Minor # is on Stack.  Entry Point sets Read/Write Flag
; Exit :  A = 0, Zero Set if Ok, A <> 0, Zero Reset if Errors
;        (also returns H = 0 and L set to A for compatibilty with C code)
; Uses : AF,HL

_devfd_read:
        LD      A,#1
        .db 0x21 ;..Trash HL, fall thru..
_devfd_write:
        LD      A,#0            ; has to be two bytes -- do not optimise to xor a!
        LD      (rdOp),A

        POP     HL              ; Return Addr
        POP     BC              ;  minor (->C)
        PUSH    BC              ;   Keep on Stack for Exit
        PUSH    HL

	PUSH	IY

        LD      A,C
        LD      (drive),A       ; Save Desired Device

        CALL    Setup           ; Set up subsystem

        LD      A,#15           ; Get the maximum retry count
Rwf1:   LD      (rwRtry),A
        LD      D,#0xFF         ;  (Verify needed)
        CALL    SEEK            ; Try to seek to the desired track
        JR      NZ,Rwf2         ; ..jump if No Good

        LD      A,(rdOp)
        OR      A               ; Read operation?
        LD      A,#0x05         ; Load DP8473 Write Command
        JR      Z,SWrite        ; No, must be Write
        INC     A               ; (A=06H) Load DP8473 Read Command
SWrite: OR      #0x40           ;  Set MFM Mode Bit
        PUSH    BC              ;   Save Regs
        LD      C,A             ;  Save
        LD      B,#9            ; Read/Write Comnds are 9 bytes

        LD      A,(eot)         ; Get Last Sctr #
        PUSH    AF              ;  (save for Exit)
        LD      A,(sect)        ; Get Desired Sector #
        LD      (eot),A         ;  make last to Read only one Sector

        ld      hl,(_devfd_buffer)
        CALL    FdCmd           ; Execute Read/Write

        POP     AF              ; Restore Last Sctr #
        LD      (eot),A         ;  to Comnd Blk

        LD      A,(st1)         ; Get Status Reg 1
        AND     #0x34           ; Return Any Error Bits
        POP     BC              ; Restore Regs
        LD      (_devfd_error),A        ;  (store Error bits)
        JR      Z,FhdrX         ; ..jump to return if No Errors

Rwf2:   LD      A,(rwRtry)      ; Get retry count
        CP      #2              ; Are we on Next to last try?
        CALL    Z,Recal         ;  Return to Track 0 if so
        LD      A,(rwRtry)      ;   and re-fetch try count
        DEC     A               ; Do we have more retries left?
        JR      NZ,Rwf1         ; ..jump to try again if more tries remain

        OR      #0xFF           ; Else show Error
FhdrX:  LD      L,A
        LD      H,#0
	POP	IY
        RET                     ;   and Exit

;-------------------------------------------------------------
; SPEC - Do a Specify Command, setting Step Rate and Head
;  Load/Unload Time.  Settings require tailoring to Drive.
;
; Enter: IY -> Drive Table entry for current drive
; Exit : Nothing
; Uses : AF,BC

Spec:   CALL    WRdyT           ; Wait for RQM (hope DIO is Low!), Disable Ints
        RET     C               ; ..Error if Timed Out
        LD      A,#0x03         ; Do an FDC Specify Command
        OUT     (FDC_DATA),A

        CALL    WRdyT
        RET     C               ; ..Error if Timed Out
        LD      A,oPRM1(IY)     ;  first Rate Byte (Step Rate, HUT)
        OUT     (FDC_DATA),A

        CALL    WRdyT
        RET     C               ; ..Error if Timed Out
        LD      A,oPRM2(IY)     ; Get Head Load Time
        ADD     A,A             ;  Shift value left (doubles count)
        INC     A               ;   Set LSB for Non-DMA Operation
        OUT     (FDC_DATA),A
        XOR     A               ;  Return Ok Flag
        RET

;-------------------------------------------------------------
; RECAL  Recalibrate Current "drive" (moves heads to track 0).
; Enter : IY -> Current Drive Table Entry
;         Variable "drive" set to desired floppy unit
; Return:  A = 0 if Ok, NZ if Error.  Flags reflect A
; Uses  : AF            All other Registers Preserved/Not Affected
;
; NOTE: BC Must be preserved by this routine.

Recal:  LD      A,(hd)          ; Get head #
        ADD     A,A
        ADD     A,A             ;  Shift to B3
        PUSH    HL              ;   (preserve regs)
        LD      HL,#drive
        OR      (HL)            ;   add Drive bits
        POP     HL              ;    (restore regs)

        LD      (hdr),A         ;   in Command Block
        LD      A,#3            ; Give this 3 chances to Home
Recal1: LD      (retrys),A
        PUSH    BC              ; Save needed regs
        PUSH    HL
        LD      BC,#(2*256+7)   ;   (2-byte Recalibrate Comnd = 07H)
        CALL    FdCmd           ;  execute Recalibrate
        CALL    FdcDn           ; Clear Pending Ints, Wait for Seek Complete
        POP     HL              ;   (restore regs)
        POP     BC
	JR	Z, RecalFail
        AND     #0x10           ; Homed?  (B4=1 if No Trk0 found)
        JR      Z,RecOk         ; ..jump to Store if Ok
RecalFail:
        LD      A,(retrys)
        DEC     A               ; Any trys left?
        JR      NZ,Recal1       ; ..loop if So
        DEC     A               ; Else set Error Flag (0-->FF)
        RET

RecOk:  XOR     A               ; Get a Zero (track / flag)
        LD      oTRK(IY),A      ;  Set in Table
        RET                     ;   and return

;-------------------------------------------------------------
; READID - Read the first Valid Address Mark on a track.
;
; Enter : "hdr" byte set in Command Blk
; Return:  A = 0 if Ok, NZ if Error.  Flags reflect A
; Uses  : AF            All other Registers Preserved/Not Affected

ReadID: LD      A,#0x4a         ; Load ReadID Command + MFM Mode byte
        PUSH    BC              ; Save regs
        LD      B,#2            ;  two bytes in ReadID Command
        LD      C,A             ;   move Command to C
        CALL    FdCmd           ; Activate DP8473 FDC

        LD      A,(st1)         ; Get Status Reg 1
        AND     #0x25           ;  Return Any Error Bits
        POP     BC              ;   Restore regs
        RET                     ;  ..and quit

;-------------------------------------------------------------
; SETUP - Set parameters necessary to Read/Write from Active Drive
; Enter: Variable "drive" set to current Drive (0..3)
;        Variables _devfd_track and _devfd_sector set to desired block address
; Exit : IY -> Drive's Table entry
; Uses : AF,BC,HL

Setup:  LD      A,(drive)
        PUSH    AF
        CALL    GetPrm          ; Pt to Current Drive's Table
        PUSH    HL
        POP     IY
        POP     BC
        LD      A,(active)      ; Get current Activation Byte
        AND     #0xf0           ;  keep only motors
        OR      B               ;   add drive bits
        CALL    Activ8          ;    save new byte and activate FDC
        LD      A,(_devfd_track)        ; Get Host Track #
        SRL     A               ;  Div by 2 (LSB to Carry)
        LD      (trk),A         ;   Physical Track # to Comnd Blk
        LD      A,#0
        ADC     A,A             ; LSB becomes Head #
        LD      (hd),A          ;  save in Comnd Blk
        ADD     A,A
        ADD     A,A             ; Shift to B3
        LD      HL,#drive
        OR      (HL)            ;  add Drive bits
        LD      (hdr),A         ;   Save in Comnd Blk
        LD      A,oGAP3(IY)
        LD      (gpl),A         ; Set Gap3 Length
        LD      A,oSPT(IY)
        LD      (eot),A         ;  Final Sector # on Trk
        LD      A,oFMT(IY)
        AND     #3              ; B0/1 of Format byte is Sector Size
        LD      (rsz),A         ;  save in Comnd Blk
        LD      A,#0xFF
        LD      (dtl),A         ;   Set Data Length code
        LD      A,(_devfd_sector)
        ADD     A,oSEC1(IY)     ;    Offset Sector # (base 0) by 1st Sector #
        LD      (sect),A        ;     set in Comnd Blk
        XOR A                   ;  (Preset Hi 500 kbps, 3.5 & 5.25" Rate)
        BIT     7,oFMT(IY)      ; Hi (500 kbps) Speed?
        JR      NZ,StSiz0       ; ..jump if Hi-Density/Speed to Set if Yes
        LD      A,oFMT(IY)
        AND     #0x0c           ; Else Get Drive size
        CP      #0x08           ; 5.25"?
        LD      A,#0x02         ;  (Prepare for 250 kbps)
        JR      NZ,StSiz0       ; ..jump if Not 5.25" w/Rate Set
        BIT     4,oFMT(IY)      ; Hi-Density capable drive?
        LD      A,#0x02         ;  (Prepare for 250 kbps)
        JR      Z,StSiz0        ; ..jump if No
        LD      A,#0x01         ; Else set to 300 kbps (@360 rpm = 250kbps)
StSiz0:
.ifne FDC_CCR
	OUT     (FDC_CCR),A     ; Set Rate in FDC Reg
.endif
        LD      D,A             ;   preserve Rate bits
.ifeq CPU_Z180
        IN0     A,(0x1F)        ; Read Z80182 CPU Cntrl Reg (B7=1 if Hi Speed)
        RLA                     ;  Speed to Bit Carry..Turbo?
        LD      A,#(CPU_CLOCK_KHZ/1000)	; (Get Processor Rate in MHz)
        JR      C,StSiz1        ;  ..jump if Turbo for longer delay
.else
	LD	A,#(CPU_CLOCK_KHZ/1000)	; (Get Processor Rate in MHz)
.endif
        SRL     A               ;  Divide rate by 2
StSiz1: INC     D
        DEC     D               ; 500 kb/s (Hi-Speed) Rate (D=0)?
        JR      NZ,StSiz2       ; ..jump if Not
        LD      A,#1            ;  Else minimum delay for "High-Speed"
StSiz2: LD      (dlyCnt),A      ;   save delay count
        RET

;-------------------------------------------------------------
; SEEK - Set the Track for disk operations and seek to it.
;
; Enter :  A = Desired Track Number
;          D = Verify flag (0=No, FF=Yes)
; Return:  A = 0, Zero Flag Set (Z) if Ok, A <> 0 Zero Clear (NZ) if Error
; Uses  : AF            All other Registers Preserved/Not Affected

SEEK:   PUSH    HL              ; Save Regs used here
        PUSH    DE
        PUSH    BC

        LD      A,(trk)         ;  Get Track #
        CP      oTRK(IY)        ; Is desired Track same as last logged?
        LD      oTRK(IY),A      ;  (set as if we made it there)
        JR      NZ,SEEKNV       ; ..jump if Not Same
        INC     D               ; Else Set to No Verify (FF->0)
SEEKNV: LD      A,#4            ; Get the maximum Retry Count
SEEK1:  LD      (retrys),A      ;  save remaining Retry Count
        LD      BC,#(3*256+0x0F);   (3-byte Seek Command = 0FH)
        CALL    FdCmd           ; Execute the Seek
        CALL    FdcDn           ; Clear Pending Int, wait for Seek Complete
	JR	Z,SEEK2

        AND     #0xE0
        CP      #0x20
        JR      NZ,SEEK2        ;;
        
        AND     #0x40           ; Set NZ if Abnormal Termination

        LD      B,A             ;; Save Seek Status
        LD      A,(trk)         ;; Check track #
        CP      C               ;;  Same track?
        JR      NZ,SEEK2        ;;   Jump to Retry if NOT
        LD      A,B             ;; Restore Seek Status

        INC     D               ; Are we Verifying (FF -> 0)?
        CALL    Z,ReadID        ;   Read next ID Mark if So
        DEC     D               ;    (Correct for Test, 0 -> FF)

        OR      A               ; Set Status (Seek Status if No ReadID)
        JR      Z,SEEKX         ; ..exit if Ok

SEEK2:  LD      A,(retrys)      ; Else get trys remaining
        DEC     A               ; Any left (80-track could need two)?
        JR      NZ,SEEK1        ; ..loop to try again if More
        DEC     A               ; Else set Error Flag (0->FF)

SEEKX:  POP     BC              ; Restore Regs
        POP     DE
        POP     HL
        RET


;-------------------------------------------------------------
; Check for Proper Termination of Seek/Recalibrate Actions by
;  executing a Check Interrupt Command returning ST0 in A.
; Enter: None.  Used after Seek/Recalibrate Commands
; Exit : A = ST0 Result Byte, C = PCN result byte
; Uses : AF and C.  All other registers preserved/unused
;
; Returns Z on timeout, NZ on success

FdcDn:  PUSH    HL              ; Don't alter regs
	PUSH	DE
	LD	DE,#0x2000	; So we time out eventually
FdcDn0:	CALL    WRdy1
	DEC	DE
	LD	A,D
	OR	E
	JR	Z, FdcNotDn	; Timed out
        LD      A,#8            ; Sense Interrupt Status Comnd
        OUT     (FDC_DATA),A
        CALL    WRdy1
        IN      A,(FDC_DATA)    ; Get first Result Byte (ST0)
        LD      L,A
        CP      #0x80           ; Invalid Command?
        JR      Z,FdcDn0        ; ..jump to exit if So
        CALL    WRdy1
        IN      A,(FDC_DATA)          ; Read Second Result Byte (Trk #)
        LD      C,A             ; ..into C
        LD      A,L
        BIT     5,A             ; Command Complete?
        JR      Z,FdcDn0        ; ..loop if Not
FdcNotDn:
	POP	DE
        POP     HL
        RET

;-------------------------------------------------------------
; MOTOR CONTROL.  This routine performs final selection of
; the drive control latch and determines if the Motors are
; already spinning.  If they are off, then the Motors are
; activated and the spinup delay time in tenths-of-seconds
; is performed before returning.
;
; Enter : None
; Return: None
; Uses  : HL.
;         AF, BC, DE, IY guaranteed to be preserved. Note external call.

Motor:  PUSH    AF              ; Save Regs
        PUSH    BC
        LD      A,#MONTIM       ; Get motor timeout
        LD      (motim),A       ; Reset the countdown timer
        LD      A,(drive)       ; Get current Drive (range 0...3)
        LD      B,#0x10         ; Bit for drive 0 motor
        OR      A               ; Test if zero
MtrNxt: JR      Z,MtrSet        ; Drive bit in position?
        SLA     B               ; Shift bit
        DEC     A               ; Count down
        JR      MtrNxt          ; See if we're done
MtrSet: ; now B contains the relevant motor bit we need to be set in the FDC DOR
        LD      A,(active)      ; Load current DOR contents
        AND     #0xFC           ; Clear bottom two bits (selected drive number)
        LD      C,A             ; Save copy in C
        AND     B               ; Is the relevant motor bit set?
        JR      NZ,MotorX       ; Motor is on already, we're done here
        LD      A,(drive)       ; Load active drive
        OR      C               ; Mix in current DOR contents
        OR      B               ; Set bit for additional motor to spin up
        CALL    Activ8          ; Send to FDC DOR, update active
        ; TODO this is a busy loop -- we should set a timer and yield
        LD      A,(drive)       ; Get Current drive
        CALL    GetPrm          ;  Pt to Param table
        LD      BC,#oSPIN
        ADD     HL,BC           ;   offset to Spinup Delay
        LD      A,(HL)          ;    Get value
        LD      (mtm),A         ;     to GP Counter
        EI                      ; Ensure Ints are ABSOLUTELY Active..
;
;	FIXME: this is wrong on two levels
;	#1 We shouldn't rely upon an IRQ (we can busy wait too)
;	#2 The timers are set in 1/20ths but it's not clear everyone is
;	using 1/20ths for the IRQ call (See p112)
;
MotoLp:	PUSH	IY
	PUSH	DE
	CALL	_plt_idle
	POP	DE
	POP	IY
	LD      A,(mtm)         ;  ..otherwise, loop never times out!
        OR      A               ; Up to Speed?
        JR      NZ,MotoLp       ; ..loop if Not
        DI                      ; No Ints now..
MotorX: POP     BC
        POP     AF              ; Restore Reg
        RET

;-------------------------------------------------------------
; Wait for FDC RQM to become Ready with Timeout indicator.
; Timeout Length is arbitrary and depends on CPU Clock Rate.

WRdyT:  ;DI                     ; No Ints while we are doing I/O
        LD      BC,#30000       ; << Arbitrary >>
WRdyT0: DEC     BC
        LD      A,B
        OR      C               ; Timed Out?
        SCF                     ;  (set Error Flag in case)
        RET     Z               ; ..return Error Flag if Yes
        IN      A,(FDC_MSR)     ; Read Status Reg
        AND     #0x80           ; Interrupt Present (also kill Carry)?
        RET     NZ              ; ..return Ok if Yes
        JR      WRdyT0          ;  ..else loop to try again
 
;-------------------------------------------------------------
; Return Pointer to Parameters of selected Drive
; Enter: A = Drive (0..3)
; Exit : HL -> Parameter entry of drive
; Uses : AF,HL

GetPrm: PUSH    DE
        LD      DE,#TBLSIZ      ; Entry Size
        LD      HL,#_devfd_dtbl ;  Init to table start
        INC     A
GetPr0: DEC     A               ; End?
        JR      Z,GetPrX        ; ..quit if Yes, Ptr set
        ADD     HL,DE           ; Else step to next
        JR      GetPr0          ; ..loop til found

GetPrX: POP     DE
        RET

;-------------------------------------------------------------
; This routine called at each Clock Interrupt.  It is used
; to provide any necessary timer/timeout functions.
; Enter: None.
; Exit : HL -> mtm byte variable
;        AF - destroyed
; Uses : AF,HL

_fd_tick:
        LD      HL,#motim       ; Point to FDC Motor-On timer
        LD      A,(HL)
        OR      A               ; Already Timed out?
        JR      Z,TDone         ; ..jump if Yes
        DEC     (HL)            ; Else count down
        CALL    Z,MotOff        ;   stop motors if timed out
TDone:  INC     HL              ; Advance ptr to watchdog/spinup timer (mtm)
        LD      A,(HL)
        OR      A               ; Timed out?
        RET     Z               ; ..quit if Yes
        DEC     (HL)            ; Else count down
        RET                     ;   exit

;-------------------------------------------------------------
; Motor Off routine.  Force Off to delay on next select
; Enter: None. (Motoff)
;        A = FDC Device Control Reg bits (Activ8/ActivA)
; Exit : A = Current FDC_DOR Register / "active" byte settings
; Uses : AF

MotOff: XOR     A
        LD      (motim),A       ; Ensure Motors Marked as OFF
        LD      A,(active)      ; Get current settings
        AND     #7              ;  strip off Motor bits
Activ8: OR      #4              ;   (ensure FDC out of Reset)
ActivA: LD      (active),A      ;    save
        OUT     (FDC_DOR),A     ;     and Command!
        RET

;-------------------------------------------------------------
; FDCMD - Send Command to DP-8473 FDC
; Enter:  B = # of Bytes in Command, C = Command Byte
;        HL -> Buffer for Read/Write Data (If Needed)
; Exit : AF = Status byte
; Uses : AF.  All other registers preserved/unused

FdCmd:  PUSH    HL              ; Save regs (for Exit)
        PUSH    BC
        PUSH    DE

        ; rewrite FdCmdXfer code so data flows in the correct direction
        LD      A,(rdOp)
        OR      A
        LD      A,#0xA2         ; Load second byte of INI opcode (doesn't update flags)
        JR      NZ,FdCiUpd      ; ... if read, skip increment
        INC     A               ; ... if write, A=0xA3, second byte of OUTI opcode
FdCiUpd:LD      (FdCiR1+1),A    ; update second byte of INI/OUTI instruction

        ; is the buffer in user memory?
        LD      A,(_devfd_userbuf)
        LD      D,A             ; store userbuf flag in D

        ; prepare the drive
        CALL    Motor           ; Ensure motors are On

        CALL    FdCmdXfer       ; Do the data transfer (using code in _COMMONMEM)

        LD      HL,#st0         ; Point to Status Result area
IsGo:   CALL    WRdy
        BIT     4,A             ; End of Status/Result?
        JR      Z,FdcXit        ; ..exit if So
        BIT     6,A             ; Another byte Ready?
        JR      Z,FdcXit        ; ..exit if Not
        INI                     ; Else Read Result/Status Byte
        JR      IsGo            ; ..loop for next
FdcXit: 
        POP     DE              ; Restore Regs
        POP     BC
        POP     HL
        RET

;------------------------------------------------------------
; COMMON MEMORY
;------------------------------------------------------------
        .area _COMMONMEM

; inner section of FdCmd routine, has to touch buffers etc
FdCmdXfer:
        BIT     0,D             ; Buffer in user memory?
	JR	Z,  kernxfer
        CALL    map_proc_always
	JR 	doxfer
kernxfer:
	CALL	map_buffers
doxfer:
        ; send the command (length is in B, command is in C)
        PUSH    HL              ; save pointer for possible Transfer
        LD      HL,#comnd       ; Point to Command Block
        LD      (HL),C          ;  command passed in C
        LD      C,#FDC_DATA     ;   FDC Data Port
OtLoop: CALL    WRdy            ; Wait for RQM (hoping DIO is Low) (No Ints)
        OUTI                    ; Output Command bytes to FDC
        JR      NZ,OtLoop       ; ..loop til all bytes sent
        POP     HL              ; Restore Possible Transfer Addr
        JR      FdCiR2          ; start sampling MSR

; transfer loop
FdCiR1: INI                     ; *** THIS INSTRUCTION IS MODIFIED IN PLACE to INI/OUTI
        ; INI  = ED A2
        ; OUTI = ED A3
FdCiR2: IN      A,(FDC_MSR)     ; Read Main Status Register
        BIT     7,A
        JR      Z, FdCiR2       ; loop until interrupt requested
FdCiR3: AND     #0x20           ; are we still in the Execution Phase? (1 cycle faster than BIT 5,A but destroys A)
        JR      NZ, FdCiR1      ; if so, next byte!

; tidy up and return
FdCmdXferDone:
        JP      map_kernel_restore ; else remap kernel and return

;-------------------------------------------------------------
; Wait for FDC RQM to become Ready, return DIO status in
; Zero Flag.  Pause before reading status port (~12 mS
; specified, some assumed in code).

WRdy:
WRdy1:  LD      A,(dlyCnt)      ; Get delay count
WRdy0:  DEC     A               ;  count down
        JR      NZ,WRdy0        ;   for ~6 uS Delay

WRdyL:  IN      A,(FDC_MSR)     ; Read Main Status Register
        BIT     7,A             ; Interrupt Present?
        RET     NZ              ;  Return if So
        JR      WRdyL           ;   Else Loop

dlyCnt: .db     (CPU_CLOCK_KHZ/1000)    ; Delay to avoid over-sampling status register

; FDC command staging area
comnd:  .ds     1               ; Storage for Command in execution
hdr:    .ds     1               ; Head (B2), Drive (B0,1)
trk:    .ds     1               ; Track (t)
hd:     .ds     1               ; Head # (h)
sect:   .ds     1               ; Physical Sector Number
rsz:    .ds     1               ; Bytes/Sector (n)
eot:    .ds     1               ; End-of-Track Sect #
gpl:    .ds     1               ; Gap Length
dtl:    .ds     1               ; Data Length

;------------------------------------------------------------
; DATA MEMORY
;------------------------------------------------------------
         .area _DATA

drive:  .ds     1               ; (minor) Currently Selected Drive
active: .ds     1               ; Current bits written to FDC_DOR

_devfd_sector:  .ds     1
_devfd_track:   .ds     1               ; LSB used as Head # in DS formats
_devfd_error:   .ds     1
_devfd_buffer:  .ds     2
_devfd_userbuf: .ds     1

; DISK Subsystem Variable Storage
; FDC Operation Result Storage Area
st0:    .ds     1               ; Status Byte 0
st1:    .ds     1               ; Status Byte 1 (can also be PCN)
        .ds     1               ; ST2 - Status Byte 2
        .ds     1               ; RC - Track #
        .ds     1               ; RH - Head # (0/1)
        .ds     1               ; RR - Sector #
        .ds     1               ; RN - Sector Size

; -->>> NOTE: Do NOT move these next two variables out of sequence !!! <<<--
motim:  .ds     1               ; Motor On Time Counter                <<<--
mtm:    .ds     1               ; Floppy Spinup Time down-counter      <<<--

rdOp:   .ds     1               ; Read/write flag
retrys: .ds     1               ; Number of times to try Opns
rwRtry: .ds     1               ; Number of read/write tries
DRVSPD: .ds     1               ; Drive Speed
DRVSIZ: .ds     1               ; Drive Size
