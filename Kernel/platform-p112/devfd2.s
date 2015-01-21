;:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
; D-X Designs Pty Ltd P112 Floppy disk Routines
;       Copyright (C) 1998 by Harold F. Bower
;:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
; 2015-01-17 Will Sowerbutts: Ported to sdas/Fuzix from UZI-180

        .module devfd2
        .z180

        ; imported symbols
        .globl _devfd_dtbl

        ; exported sybols
        .globl _devfd_init
        .globl _devfd_read
        .globl _devfd_write
        .globl _devfd_track
        .globl _devfd_sector
        .globl _devfd_error
        .globl _devfd_buffer
        .globl _fd_tick

        .include "kernel.def"
        .include "../kernel.def"

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
;    093        -  (Not Used)
;    094        - Data-Rate Select (Write) / Main Status Register (Read)
;       7 6 5 4 3 2 1 0                         (Write)

;       7 6 5 4 3 2 1 0                         (Read)
;       | | | | +-+-+-+-- Drives Seeking (0=B0 Set, 1=B1 Set,.. 3=B3 Set)
;       | | | +---------- 1 = Command In Progress, 0 = Command Ended
;       | | +------------ 1 = Non-DMA Execution,   0 = DMA Execution
;       | +-------------- 1 = Read,                0 = Write
;       +---------------- 1 = Request for Master,  0 = Internal Execution
;
;    095        - Data/Command Register         (Read/Write)
;                               (Byte Writes/Reads)
;    096        -  (Not Used)
;    097        - Data Rate Register (Write) / Disk Changed Bit (Read)
;       7 6 5 4 3 2 1 0                         (Write)
;       | | | | | | +-+-- 00=500 kb/s, RPM/LC Hi, 01=250/300 kb/s (RPM/LC Lo)
;       | | | | | |       10=250 kb/s, RPM/LC Lo, 11=1000 kb/s (RPM/LC Hi/Lo)
;       +-+-+-+-+-+------  (Not Used)
;
;       7 6 5 4 3 2 1 0                         (Read)
;       | +-+-+-+-+-+-+-- (Tri-State, used for HD Controller)
;       +---------------- 1 = Disk Changed  (latched complement of DSKCHG inp)
;
;    0A0        - DMA I/O Select Port (DMA configuration Only)
;-------------------------------------------------------------
FDCBAS  .equ    0x90            ; SMC 37C665 Controller Base Address
DCR     .equ    FDCBAS+2        ; Drive Control Register
MSR     .equ    FDCBAS+4        ; Main Status Register
DR      .equ    FDCBAS+5        ; Data Register
DRR     .equ    FDCBAS+7        ; Data Rate Register/Disk Changed Bit in B7

MONTIM  .equ    250             ; Motor On time (Seconds * TICKSPERSEC)

; Offsets in Drive Data Table (defined at end of this module)
oFLG    .equ    0               ; 0 = Not Logged, 1 = Drive Logged
oPRM1   .equ    1               ; Step Rate (B7-4), HUT (3-0)
oPRM2   .equ    2               ; Hd Load in 4mS steps (0=infinite)
oGAP3   .equ    3               ; Gap 3 Length for Read
oSPT    .equ    4               ; Sectors-per-Track
oSEC1   .equ    5               ; First Sector Number
oFMT    .equ    6               ; Bit-mapped Format byte
oSPIN   .equ    7               ; Spinup delay (1/20-secs)
oTRK    .equ    8               ; Current Head Position (Track)

;-------------------------------------------------------------
; Determine if the controller exists and a drive is attached
;       fdInit (int minor);
; Enter: Drive Minor # is on Stack
; Exit : HL = 0 if All is Ok, Non-Zero if Error

_devfd_init:
        XOR     A
        LD      (motim),A       ; Mark Motors as initially OFF
        LD      (hd),A          ;  and initially Head #0

        POP     HL              ; Return Addr
        POP     BC              ;  minor (in C)
        PUSH    BC              ;   Keep on Stack for Exit
        PUSH    HL
        LD      A,C
        LD      (drive),A       ; Save Desired Device
        CP      #4              ; Legal?
        JR      NC,NoDrv        ; ..Exit if Error
        CALL    ActivA          ; Else force Reset (B2=0)
        LD      B,#0
indel1: DJNZ    indel1          ;    (settle)
        CALL    Activ8          ;  then bring out of Reset
indel2: DJNZ    indel2          ;    (settle, B already =0)
        IN      A,(MSR)
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
        RET

NoDrv:  LD      HL,#0xFFFF      ; Set Error Status
        RET

;-------------------------------------------------------------
; This routine Reads/Writes data from buffer trying up to 4 times
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
        LD      A,C
        LD      (drive),A       ; Save Desired Device
;;      CP      4               ; Legal?
;;      JR      NC,NoDrv        ; ..Exit if Error

        CALL    Setup           ; Set up subsystem
;;--    LD      HL,buffer       ;  Point to the host buffer
;;--    LD      (actDma),HL     ;   and set Memory Pointer

        LD      A,#4            ; Get the maximum retry count
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

;;--    LD      HL,(actDma)     ; Get actual DMA Addr
        ld      hl,(_devfd_buffer)      ;;--
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
        OUT     (DR),A

        CALL    WRdyT
        RET     C               ; ..Error if Timed Out
        LD      A,oPRM1(IY)     ;  first Rate Byte (Step Rate, HUT)
        OUT     (DR),A

        CALL    WRdyT
        RET     C               ; ..Error if Timed Out
        LD      A,oPRM2(IY)     ; Get Head Load Time
        ADD     A,A             ;  Shift value left (doubles count)
        INC     A               ;   Set LSB for Non-DMA Operation
        OUT     (DR),A
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
        AND     #0x10           ; Homed?  (B4=1 if No Trk0 found)
        JR      Z,RecOk         ; ..jump to Store if Ok
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
StSiz0: OUT     (DRR),A         ; Set Rate in FDC Reg
        LD      D,A             ;   preserve Rate bits
        IN0     A,(0x1F)        ; Read Z80182 CPU Cntrl Reg (B7=1 if Hi Speed)
        RLA                     ;  Speed to Bit Carry..Turbo?
        LD      A,#(CPU_CLOCK_KHZ/1000)         ;   (Get Processor Rate in MHz)
        JR      C,StSiz1        ;  ..jump if Turbo for longer delay
        SRL     A               ;  Else divide rate by 2
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
; FDCMD - Send Command to DP-8473 FDC
; Enter:  B = # of Bytes in Command, C = Command Byte
;        HL -> Buffer for Read/Write Data (If Needed)
; Exit : AF = Status byte
; Uses : AF.  All other registers preserved/unused

FdCmd:  PUSH    HL              ; Save regs (for Exit)
        PUSH    BC
        PUSH    HL              ;   save pointer for possible Transfer
        CALL    Motor           ; Ensure motors are On
        LD      HL,#comnd       ; Point to Command Block
        LD      (HL),C          ;  command passed in C
        LD      C,#DR           ;   DP8473 Data Port
OtLoop: CALL    WRdy            ; Wait for RQM (hoping DIO is Low) (No Ints)
        OUTI                    ; Output Command bytes to FDC
        JR      NZ,OtLoop       ; ..loop til all bytes sent

        POP     HL              ; Restore Possible Transfer Addr
FdCi1:  CALL    WRdy
        BIT     5,A             ; In Execution Phase?
        JR      Z,FdcRes        ; ..jump if Not to check result
        BIT     6,A             ; Write?
        JR      NZ,FdCi2        ; ..jump if Not to Read
        OUTI                    ; Else Write a Byte from (HL) to (C)
        JR      FdCi1           ;   and check for next

FdCi2:  INI                     ; Read a byte from (C) to (HL)
        JR      FdCi1           ;   and check for next

FdcRes: LD      HL,#st0         ; Point to Status Result area
IsGo:   CALL    WRdy
;;---   CALL    _ei             ;  (Ints Ok now)
        BIT     4,A             ; End of Status/Result?
        JR      Z,FdcXit        ; ..exit if So
        BIT     6,A             ; Another byte Ready?
        JR      Z,FdcXit        ; ..exit if Not
        INI                     ; Else Read Result/Status Byte
        JR      IsGo            ; ..loop for next

FdcXit: POP     BC              ; Restore Regs
        POP     HL
        RET

;-------------------------------------------------------------
; Check for Proper Termination of Seek/Recalibrate Actions by
;  executing a Check Interrupt Command returning ST0 in A.
; Enter: None.  Used after Seek/Recalibrate Commands
; Exit : A = ST0 Result Byte, C = PCN result byte
; Uses : AF and C.  All other registers preserved/unused

FdcDn:  PUSH    HL              ; Don't alter regs
FdcDn0: CALL    WRdy1
        LD      A,#8            ; Sense Interrupt Status Comnd
        OUT     (DR),A
        CALL    WRdy1
        IN      A,(DR)          ; Get first Result Byte (ST0)
        LD      L,A
        CP      #0x80           ; Invalid Command?
        JR      Z,FdcDn0        ; ..jump to exit if So
        CALL    WRdy1
        IN      A,(DR)          ; Read Second Result Byte (Trk #)
        LD      C,A             ; ..into C
        LD      A,L
        BIT     5,A             ; Command Complete?
        JR      Z,FdcDn0        ; ..loop if Not
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
; Uses  : HL.  Remaining Registers Preserved/Not Affected

Motor:  PUSH    AF              ; Save Regs
        LD      A,(motim)       ; Get remaining Seconds
        OR      A               ; Already On?
        LD      A,#MONTIM       ;  (get On Time)
        LD      (motim),A       ;   always reset
        JR      NZ,MotorX       ; ..exit if already running
        PUSH    BC
        LD      A,(hdr)         ; Get current Drive
        OR      #0xF4           ;   Set All Motors On and Controller Active
        CALL    Activ8          ;     Do It!
        LD      A,(drive)       ; Get Current drive
        CALL    GetPrm          ;  Pt to Param table
        LD      BC,#oSPIN
        ADD     HL,BC           ;   offset to Spinup Delay
        LD      A,(HL)          ;    Get value
        LD      (mtm),A         ;     to GP Counter
        EI                      ; Ensure Ints are ABSOLUTELY Active..
MotoLp: LD      A,(mtm)         ;  ..otherwise, loop never times out!
        OR      A               ; Up to Speed?
        JR      NZ,MotoLp       ; ..loop if Not
        DI                      ; No Ints now..
        POP     BC
MotorX: POP     AF              ; Restore Reg
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
        IN      A,(MSR)         ; Read Status Reg
        AND     #0x80           ; Interrupt Present (also kill Carry)?
        RET     NZ              ; ..return Ok if Yes
        JR      WRdyT0          ;  ..else loop to try again
 
;-------------------------------------------------------------
; Wait for FDC RQM to become Ready, return DIO status in
; Zero Flag.  Pause before reading status port (~12 mS
; specified, some assumed in code).

WRdy:   ;DI                     ; No Ints while we are doing I/O
                                ;  (entry to avoid Disabling Ints)
WRdy1:  LD      A,(dlyCnt)      ; Get delay count
WRdy0:  DEC     A               ;  count down
        JR      NZ,WRdy0        ;   for ~6 uS Delay
WRdyL:  IN      A,(MSR)         ; Read Main Status Register
        BIT     7,A             ; Interrupt Present?
        RET     NZ              ;  Return if So
        JR      WRdyL           ;   Else Loop

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
; Exit : A = Current DCR Register / "active" byte settings
; Uses : AF

MotOff: XOR     A
        LD      (motim),A       ; Ensure Motors Marked as OFF
        LD      A,(active)      ; Get current settings
        AND     #7              ;  strip off Motor bits
Activ8: OR      #4              ;   (ensure FDC out of Reset)
ActivA: LD      (active),A      ;    save
        OUT     (DCR),A         ;     and Command!
        RET

;------------------- Data Storage Area -----------------------
; Disk and Drive parameters are dictated by table entries.  While
; not all parameters are implemented in this module, they may be
; added as desired.  A bit-mapped byte is used defined as:

; D D D D D D D D               Format Byte
; 7 6 5 4 3 2 1 0
; | | | | | | +-+----- Sector Size: 000=128, 001=256, 010=512, 011=1024 bytes
; | | | | +-+--------- Disk Size: 00=fixed disk, 01=8", 10=5.25", 11=3.5"
; | | | +------------- 0 = Normal 300 RPM MFM,    1 = "High-Density" Drive
; | | +--------------- 0 = Single-Sided,          1 = Double-Sided
; | +----------------- 0 = Double-Density,        1 = Single-Density
; +------------------- 0 = 250 kbps (normal MFM), 1 = 500 kbps (Hi-Density)

IBMPC3  .equ    0xAE    ; 10101110B     ; HD,  DD, DS, 3.5",   512-byte Sctrs (1.44 MB)
UZIHD3  .equ    0xAF    ; 10101111B     ; HD,  DD, DS, 3.5",  1024-byte Sctrs (1.76 MB)
IBMPC5  .equ    0xAA    ; 10101010B     ; HD,  DD, DS, 5.25",  512-byte Sctrs (1.2 MB)
UZIHD5  .equ    0xAB    ; 10101011B     ; HD,  DD, DS, 5.25", 1024-byte Sctrs (1.44 MB)
DSQD3   .equ    0x2F    ; 00101111B     ; MFM, DD, DS, 3.5",  1024-byte Sctrs (800 KB)
DSDD3   .equ    0x2E    ; 00101110B     ; MFM, DD, DS, 3.5",   512-byte Sctrs (800 KB)
DSQD5   .equ    0x2B    ; 00101011B     ; MFM, DD, DS, 5.25", 1024-byte Sctrs (800 KB)
DSDD5   .equ    0x2A    ; 00101010B     ; MFM, DD, DS, 5.25",  512-byte Sctrs (800 KB)

_devfd_dtbl:                            ; Drive Param Table.  1 Entry Per Drive.
        .db 0, 0xCF,   1,  27, 18,  1, IBMPC3, 10, 0, 160
        ;   |     |    |    |   |   |    |      |  |    +- Number of Cylinders
        ;   |     |    |    |   |   |    |      |  +- Current Track Number
        ;   |     |    |    |   |   |    |      +- Spinup (1/20-secs)
        ;   |     |    |    |   |   |    +-- Format Byte (See above)
        ;   |     |    |    |   |   +------- First Sector Number
        ;   |     |    |    |   +------- Physical Sectors-Per-Track
        ;   |     |    |    +----------- Gap3 (Size 512=27, 1024=13)
        ;   |     |    +---------------- Hd Load in 4mS steps (0=inf)
        ;   |     +--------------------- Step Rate (B7-4) = 4mS (2's compl)
        ;                                   HUT (B3-0) = 240 mS
        ;   +--------------------------- Drive Logged (FF), Unlogged (0)
TBLSIZ   .equ  . - _devfd_dtbl
        .db 0, 0xCF,   1,  27,  18, 1, IBMPC3, 10, 0, 160
        .db 0, 0xCF,   1,  27,  18, 1, IBMPC3, 10, 0, 160
        .db 0, 0xCF,   1,  27,  18, 1, IBMPC3, 10, 0, 160

; -->>> NOTE: Do NOT move these next two variables out of sequence !!! <<<--
motim:  .db     0               ; Motor On Time Counter
mtm:    .db     0               ; Floppy Spinup Time down-counter

dlyCnt: .db     (CPU_CLOCK_KHZ/1000)    ; Delay to avoid over-sampling status register

;------------------------------------------------------------------------------
         .area _DATA

drive:  .ds     1               ; (minor) Currently Selected Drive
active: .ds     1               ; Current bits written to Dev Contr Reg (DCR)
;;--block:      DEFS    1               ; Index in Buffer for desired 512-byte block
;;--buffer:     DEFS    1024            ; Physical Sector Buffer.  Max Possible Size

_devfd_sector:  .ds     1
_devfd_track:   .ds     1               ; LSB used as Head # in DS formats
_devfd_error:   .ds     1
_devfd_buffer:  .ds     2

comnd:  .ds     1               ; Storage for Command in execution
hdr:    .ds     1               ; Head (B2), Drive (B0,1)
trk:    .ds     1               ; Track (t)
hd:     .ds     1               ; Head # (h)
sect:   .ds     1               ; Physical Sector Number
rsz:    .ds     1               ; Bytes/Sector (n)
eot:    .ds     1               ; End-of-Track Sect #
gpl:    .ds     1               ; Gap Length
dtl:    .ds     1               ; Data Length

; FDC Operation Result Storage Area

st0:    .ds     1               ; Status Byte 0
st1:    .ds     1               ; Status Byte 1 (can also be PCN)
        .ds     1               ; ST2 - Status Byte 2
        .ds     1               ; RC - Track #
        .ds     1               ; RH - Head # (0/1)
        .ds     1               ; RR - Sector #
        .ds     1               ; RN - Sector Size

actDma: .ds     2               ; 16-bit DMA Address

; DISK Subsystem Variable Storage

rdOp:   .ds     1               ; Read/write flag
retrys: .ds     1               ; Number of times to try Opns
rwRtry: .ds     1               ; Number of read/write tries
DRVSPD: .ds     1               ; Drive Speed
DRVSIZ: .ds     1               ; Drive Size
