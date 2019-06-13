;
;	The BIOS cseg is placed directly after the kernel and will probably
;	not be in common space. The BIOS dseg is placed after the kernel
;	common and owns all the space above that point.
;
;	These are not aligned. Aligned space can be allocated from the top
;	of memory down by the BIOS if needed.
;
;
		dseg

;
;	The jump table. Must be in common space because some vectors
;	are used from common code.
;

		jp init
		jp reboot		; must live in common
		jp monitor		; ditto (at least the entry)
		jp getinfo
		jp set_callbacks
		jp param
		jp idle
		jp set_bank		; must live in common

		jp serial_tx_ready
		jp serial_tx
		jp serial_setup
		jp serial_param
		jp serial_carrier

		jp lpt_busy
		jp lpt_tx

		jp disk_select
		jp disk_set_lba
		jp disk_read		; must live in common
		jp disk_write		; must live in common
		jp disk_flush
		jp disk_param

		jp rtc_get
		jp rtc_set
		jp rtc_secs

		jp init_done

;
;	API notes
;
;	Unless otherwise specified
;
;	- Data is passed in HL and out via HL
;	- Buffers passed in are not in common space and may disappear
;	  after the call returns
;	- Buffers passed out need not be in common space but must not
;	  become invalid.
;	- The caller may freely use AF,BC,DE,HL. The alternate registers
;	  IX and IY must be saved and restored if used
;	- Routines need not be individually re-entrant
;	- The kernel will not task switch within a BIOS routine even
;	  if a callback occurs during it
;	- Routines may be called form a callback
;	- The stack on entry is in common space and can be assumed to
;	  have at least 64 bytes free
;	- Interrupts may be disabled but must be correctly restored
;	- Interrupts must not be enabled if disabled by the OS
;	(The interrupt handling will be getting a rework and soft handlers
;	later)
;
;	Interrupt handlers must save and restore all registers including
;	alternate registers if used, and must restore any bank changes
;	they do. If Fuzix does bank changes it will restore them
;


;
;	Called when the kernel has been loaded and unpacked into the right
;	location by the loader. We keep this in common as we need to write
;	our interrupt vector into all banks
;
init:
	ld b,num_banks
init_all:
	ld a,b
	dec a
	call set_bank
	ld a,c3h
	ld (38h),a
	ld hl,interrupt_handler
	ld (39h),hl
	djnz init_all
	; We can initialize I/O devices here. In our case the
	; disk was initialized by the loader so we can relax

	; Turn on timer ticks ready

	ld a,1
	out (CLOCK),a
	im 1
	ret
;
;	Reboot handler. Called when the OS wants to reboot (if this is
;	not supported then stop). If it will clear the screen then delay
;	or wait for a key.
;
reboot:
;
;	Similar but a request to drop back to the monitor if there is one
;
monitor:
	di
	halt

		cseg

;
;	Return the info block in HL, the info block does not need to be
;	in common space.
;
getinfo:
	ld hl,sysinfo
	ret
sysinfo:
;
;
;	BYTE version	(must be 1)
;	BYTE num_banks	(numbered 0 - num-1, 0 the kernel)
;	WORD common_base
;	WORD mem_top	(last byte of memory present FFFF = 64K)
;	WORD ram_kb
;	WORD bios_top	(byte after banked bios)
;	BYTE num_serial
;	BYTE num_lpt
;	BYTE num_disk
;	WORD features
;	0: Has an RTC
;
	db	1
	db	16
	dw	0C000h
	dw	0FFFFh
	dw	784		; 16 * 48 + 16
	dw	bios_banked_end
	db	1
	db	1
	db	1
	dw	0
;
;	HL points to the callback table (which may not be in common)
;
;	WORD tick		timer/pre-emption (special rules)
;	WORD timer		other tick events (eg vblank)
;	WORD serial		serial events
;	WORD disk		disk events (media change etc)
;
;	Note that we write the two bytes of the patch in one instruction
;	so that we avoid any races.
;
set_callbacks:
	ld e,(hl)
	inc hl
	ld d,(hl)
	inc hl
	ld (clock_patch + 1),de
	; Skip the timer we don't use it yet
	inc hl
	inc hl
	ld e,(hl)
	inc hl
	ld d,(hl)
	inc hl
	ld (char_patch + 1),de
	ret

;
;	Called with each boot argument in HL as a zero terminated string
;	If one of them is consumed by the BIOS return 1 if not 0. Allows
;	boot parameters to be passed to the BIOS.
;
param:
	ld l,0
	ret

;
;	Idle loop. This can be a no-op, it can poll non interrupt driven
;	serial ports or whatever. We are IRQ driven so...
;
idle:
	halt
	ret

		dseg
;
;	set_bank must live in common for obvious reasons. It does not
;	follow the usual API
;
;	On entry the new bank is in A, where 0 is the kernel bank and 1-n
;	are the other banks.
;
;	The caller must restore any register changed except flags
;
set_bank:
	out (11),a
	ret

		cseg
;
;	Called after the platform has initialized and is about to load
;	the init process. If the BIOS has packed initialization logic at
;	the top of the bios cseg then it can adjust the top of BIOS at
;	this point to allow the OS more reclaim space
;
init_done:
	ld l,0
	ret

		dseg

;
;	Test if serial port L is ready for output. If this is actually a
;	video display you probably just want to report 'yes'
;
;	Return 0 = no 1 = yes. Do not use other values as other hints will
;	be added in future
;
;	We ignore L as we only have one port.
;
;	May be called from callbacks, must be re-entrant. For the console
;	port (1) may be called without kernel code banked.
;
serial_tx_ready:
	in a,(0)
	rlca
	ld l,0
	ret nc
	inc l
	ret

;
;	Write character H to serial port L. This can be simple or the serial
;	layer can be complex and do buffering.
;
;	May be called from callbacks, must be re-entrant. For the console
;	port (1) may be called without kernel code banked.
;
;	One port so we ignore L
;
serial_tx:
	ld a,h
	out (1),a
	ret

		cseg
;
;	Set up the modes on a serial port.
;	HL holds the address of a configuration buffer (which may not be in
;	common space)
;
;	BYTE	port		port to configure
;	BYTE	flags
;		0	change
;		1	complete writes and change
;
;	WORD	iflag
;	WORD	oflag
;	WORD	cflag
;	WORD	lflag
;	BYTE	cc[12]
;
;	Possibly relevant are
;	cc[6]	Character to use for start if doing XON/XOFF
;	cc[7]	Character to use for stop if doing XON/XOFF
;
;	iflag
;	bit 2: Ignore break
;	bit 9: Do xoff but any character does for xon
;	bit 10: Do xoff
;	bit 11: Do xon
;	bit 15: mark parity error by setting top bit
;
;	cflag
;	bit 3-0: speed (see table)
;	bit 4-5: character size (5 + n bits), not including parity, stop
;	bit 6: use 2 stop bits not 1
;	bit 7: set if port enabled for read
;	bit 8: generate parity
;	bit 9: 1 = odd 0 = even (ignored if no parity)
;	bit 11: ignore modem lines
;	bit 12: rts/cts flow control
;
;	Speed table
;	0	Hang up (drop DCD)
;	1	50
;	2	75
;	3	110
;	4	134.5
;	5	150
;	6	300
;	7	600
;	8	1200
;	9	2400
;	10	4800
;	11	9600
;	12	19200
;	13	38400
;	14	57600
;	15	115200
;	
;	
;	The other bits belong solely to Fuzix
;
;	Requested hardware features that cannot be supported should be
;	updated accordingly. The mask will cover most of them automatically
;	but others may also be updated.
;
serial_setup:
	; Not yet implemented
	ret
;
;	serial_param
;	For future expansion. Will return port info, mask bits etc in HL
;	TODO
;
serial_param:
	ret

;
;	Printer Interfaces
;
;	Return L = 0 if printer L is not busy L = 1 if it is
;	When an error is present return not busy and report the error
;	in lpt_tx
;
;	Ignore L as we have only one printer
;
lpt_busy:
	in a,(16)
	rrca
	ld l,0
	ret c
	inc l
	ret

;
;	Print character H to port L, return L = 0 or an error code
;
;	Ignore L as we have only one printer
;
lpt_tx:
	ld a,h
	out (16),a
	ret

;
;	Disk interface
;

DATA		equ 10h    	;CF data register
ERROR    	equ 11h    	;CF error reg
COUNT		equ 12h    	;CF sector count reg
LBA_0     	equ 13h   	;CF LA0-7
LBA_1    	equ 14h       	;CF LA8-15
LBA_2   	equ 15h       	;CF LA16-23
LBA_3   	equ 16h       	;CF LA24-27
CMD   		equ 17h       	;CF status/command reg
STATUS		equ 17h       	;CF status/command reg

ERR		equ 0
DRQ		equ 3
READY		equ 6
BUSY		equ 7

ATA_READ	equ 20h
ATA_WRITE	equ 30h
ATA_FLUSH	equ 0e7h

;
;	Select disk L to use for the disk operations that follow - until
;	a further select. Return an error code in L or 0.
;
disk_select:
	; We have a single fixed CF adapter so we ignore the disk code
	ld l,0
	ret

;
;	Set the LBA for the next transfer
;
;	HL is the address of 4 bytes holding the LBA in little-endian
;	format.
;
;	If your hardware is not logical block based then you should compute
;	a C/H/S mapping. The usual rule is cylinder, head, sector. That is
;	block 0 is 0/0/1 block 1 is 0/0/2 until you hit 0/1/1 .. and only
;	after all heads move on to cylinder 2.
;
;	However you may well need to follow whatever your system expects
;	on hard disks especially.
;
disk_set_lba:
	ld c,LBA_0
	outi
	inc c
	outi
	inc c
	outi
	ld a,(hl)
	or 0E0h		; Drive 0 LBA
	out (c),a
	ld a,1
	out (COUNT),a
	ld l,0
	ret

;
;	Read a 512 byte block from the disk. This routine must live in common
;	space
;
;	HL holds the address for the data, the correct bank is mapped
;
;	If your block size is over 512 bytes you need to deblock. If it is
;	smaller you should do multiple requests
;
disk_read:
	ld de,0			; retries
	ld l,EIO
	call wait_ready
	ret z
	ld a,ATA_READ
	out (CMD),a
	call wait_drq
	ret z
	bit ERR,a
	ret nz
	ld bc,DATA		; 256 + data port
	inir
	init
	ld l,0
	ret
;
;	Read a 512 byte block from the disk. This routine must live in common
;	space
;
;	HL holds the address for the data, the correct bank is mapped
;
;	If your block size is over 512 bytes you need to deblock. If it is
;	smaller you should do multiple requests. You can delay writeback
;	until a flush
;
disk_write:
	ld de,0			; retries
	ld l,EIO
	call wait_ready
	ret z
	ld a,ATA_WRITE
	out (CMD),a
	call wait_drq
	ret z
	bit ERR,a
	ret nz
	ld bc,DATA
	otir
	otir
	ld a,1
	ld (disk_dirty),a
	ld l,0
	ret

disk_dirty:
	db	0

;
;	Simple helpers for our ATA disk routines
;
wait_ready:			; NZ = ok
	dec de
	ld a,d
	or e
	ret z
	in a,(STATUS)
	bit BUSY,a
	jr nz,wait_ready
	bit READY,a
	jr z, wait_ready
	ret

wait_drq:
	dec de
	ld a,d
	or e
	ret z
	in a,(STATUS)
	bit ERR,a
	ret nz
	bit DRQ,a
	jr z,wait_drq
	ret

		cseg
;
;	Flush any data to cache and optionally to the media
;
disk_flush:
	ld a,(disk_dirty)
	ld l,a
	ret z
	;
	;	Strictly speaking we should check the identify data
	;	to see if flush cache is supported...
	;
	call wait_ready
	ret nz
	ld a,ATA_FLUSH
	out (CMD),a
	call wait_ready
	ret nz
	xor a
	ld (disk_dirty),a
	ld l,a
	ret
;
;	Report the disk parameter block in HL
;
disk_param:
	ld hl,diskinfo
	ret

diskinfo:
	dw 0		; 32bit little endian size in 512 byte blocks
	dw 2		; should come from the disk identify FIXME
	dw 3
	;	bit 0: set if you can use this media for swapping
	;	bit 1: set if partition tables may be present
	;	bit 2: set if removable
	;	bit 3: set if has floppy feature set (for future)
	;	bit 4: set if has valid geometry info
	;	unused bits must be zero
	dw 0		; cylinders
	dw 0		; heads
	dw 0		; sectors
	dw 512		; true block size


;
;	Real time clock (if present)
;
;
;	Get and set real time clock data
;	HL holds the address of a buffer (which may not in common space)
;
;	These routines are not called often so may be slow (eg bitbanged
;	I2C)
;
;	The buffer is formatted as follows
;	BYTE	type
;		0		BCD
;		1		Decimal
;		2		Unix clock (seconds since 1/1/1970 GMT)
;	BYTE    buffer[8]
;
;	The buffer holds
;	Type 0:		YYYYMMDDHHMMSS in BCD format (7 bytes)
;	Type 1:		YYYYMMDDHHMMSS in decimal format where YYYY is
;					  2 byte little endian (7 bytes)
;	Type 2:		64bit little endian time in seconds
;
;	The set function will use whatever format the get function
;	provided.
;
rtc_get:
rtc_set:
	ld l,EOPNOTSUPP
	ret
;
;	If your RTC has a fast to read seconds counter then you can
;	report the correct seconds time here and Fuzix will try and
;	lock internal time to this. If not return 255 in L
;
rtc_secs:
	ld l,255
	ret

		dseg
;
;	Supporting code. Interrupt handlers and the like
;
;	The BIOS owns
;	- interrupt vectors
;	- acknowledging interrupts
;	- calling back into the OS
;	- choice of interrupt modes
;
;	The OS owns
;	- always having a valid stack when interrupts are enabled
;	- the use of reti within the timer handler due to task switching
;	FIXME: right now if you have reti aware peripherals and your timer
;	doesn't use reti cycles you may not pick an event up until the
;	next timer 8(
;

interrupt_handler:
	push af
	in a,(0)		; was the ACIA the cause ?
	and 2
	call nz, acia_char
	in a,(CLOCK)
	rlca
	jr c, clock_event
	pop af
no_handler:
	ei
	reti
;
;	The clock is special. We must end by jumping into the Fuzix handler
;	having restored our registers as it may task switch and it may
;	never even return.
;
clock_event:
	pop af
clock_patch:
	jp no_handler

acia_char:
	in a,(1)
	ld h,a
	ld l,0		; bits 7-4: event 3-0: device
			; 0: character input of H
			; 1: carrier raise
			; 2: carrier drop
char_patch:
	call no_op
no_op:
	ret


		cseg
;
;	Used so we can tell the Fuzix kernel where the BIOS ends
;
bios_banked_end:
