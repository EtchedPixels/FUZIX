;;from https://cpctech.cpc-live.com/
;; Load data by talking directly to the NEC765 floppy disc controller (FDC)
;; https://cpctech.cpcwiki.de/source/fdcload.html from Kevin Thacker site
;; Code assumes there is a drive 0 and there is a disc in it and the disc is formatted
;; to SYSTEM format.
.area BOOT (ABS)
.org #0xfdff

;;ROM's & interrupts off

di
ld b, #0x7f
ld c,#0b10101101
out (c),c
;;from default boot org to the needed one
ld hl,#0x100
ld de,#0xfdff
ld bc,#512
ldir

jp start

start:
;;clean room
ld hl,#0
ld de ,#1
ld (#0),hl
ld bc,#0xfdfe
ldir
;;patch interrupt vector
LD HL,#0XC9FB		
LD (#0X0038),HL
;; turn on disc motor
ld bc,#0xfa7e
ld a,#1
out (c),a
;; the motor on all connected drives will be turned on
;; and the motor will start to speed up.
;;
;; The drive must be "Ready" to accept commands from the FDC.
;; A drive will be ready if:
;; * the drive motor has reached a stable speed
;; * there is a disc in the drive
;;
;; The following code is a delay which will ait enough time for the motor 
;; to reach a stable speed. (i.e. the motor speed is not increasing or decreasing)
;;
;; All drives are not the same, some 3" drives take longer to reach a stable
;; speed so we need a longer delay to be compatible with these.
;;
;; At this point interrupts must be enabled.
ei
ld b,#30 ;; 30/6 = 5 frames or 5/50 of a second.
w1:
;; there are 6 CPC interrupts per frame. This waits for one of them
halt
djnz w1
di
;; this is the drive we want to use
;; the code uses this variable.
ld a,#0
ld (#drive),a

;; recalibrate means to move the selected drive to track 0.
;;
;; track 0 is a physical signal from the drive that indicates when
;; the read/write head is at track 0 position.
;;
;; The drive itself doesn't know which track the read/write head is positioned over.
;; The FDC has an internal variable for each drive which holds the current track number.
;; This value is reset when the drive indicates the read/write head is over track 0.
;; The number is increased/decreased as the FDC issues step pulses to the drive to move the head
;; to the track we want.
;;
;; once a recalibrate has been done, both drive and fdc agree on the track.
;;
call fdc_recalibrate

;; now the drive is at a known position and is ready the fdc knows it is at a known position
;; we can read data..
;call read_file

read_file:
;; set variable for starting sector for our data (#0xC1 is first sector ID for
;; SYSTEM format. Sector IDs are #0x41, #0x42, #0x43, #0x44, #0x45, #0x46, #0x47, #0x48 and #0x49.
;; This bootloader is in track 0 sector #0x41 to load with |cpm command from basic

ld a,#0x42
ld (#sector),a

;; set variable for starting track for our data
;; Tracks are numbered 0 to 39 for 40 track drives and 0 to 79 for 80 track drives.
;; Some 3" drives can allow up to 42 tracks (0-41), some 80 track drives can allow up 
;; to 83 tracks (0-82).
;;
;; Not all drives are the same however. The maximum that is compatible with all 3" drives
;; is 41 tracks.
ld a,#0
ld (#track),a

;; memory address to write data to (start)
ld de,#0x100
ld (#data_ptr),de

;; number of complete sectors to read for our data
;; 30 sectors, 512 bytes per sector. Total data to read is 30*512 = 15360 bytes.
ld a,#126
ld (#sector_count),a

read_sectors_new_track:
;; perform a seek (this means to move read/write head to track we want).
;; track is defined by the "track" variable.
;;
;; a recalibrate must be done on the drive before a seek is done.
;; 
;; the fdc uses it's internal track value for the chosen drive to decide to seek up/down to
;; reach the desired track. The FDC issues "step pulses" which makes the read/write head move
;; 1 track at a time at the rate defined by the FDC specify command.
;;
;; e.g. if fdc thinks we are on track 10, and we ask it to move to track 5, it will step back 5 times
;; updating it's internal track number each time.
call fdc_seek

read_sectors:
;; Send Read data command to FDC to read 1 sector.

;; A track is layed out as follows:
;;
;; id field
;; data field
;;
;; id field
;; data field
;;
;; id field
;; data field
;; etc.
;;
;; we tell the FDC the values of the ID field we want. Once it finds a match it will then read
;; the data. If the ID field we want is not found, it will report an error.


ld a,#0b01000110				;; read data command (mfm=double density reading mode)
                                  ;; not multi-track. See FDC data sheet for list of commands and the 
                                  ;; number of bytes they need.
call fdc_write_command
ld a,(#drive)            ;; physical drive and side
                            ;; bits 1,0 define drive, bit 2 defines side
call fdc_write_command
ld a,(#track)					;; C value from id field of sector we want to read
call fdc_write_command
ld a,#0						;; H value from id field of sector we want to read
call fdc_write_command
ld a,(#sector)					;; R value from id field of sector we want to read
call fdc_write_command
ld a,#2					;; N value from id field of sector we want to read
                  ;; this also determines the amount of data in the sector.
                  ;; 2 = 512 byte sector
call fdc_write_command
ld a,(#sector)					;; EOT = Last sector ID to read. This is the same as the first to read 1 sector.
call fdc_write_command
ld a,#0x2a          ;; Gap Length for read. Not important.
call fdc_write_command
ld a,#0xff          ;; DTL = Data length. Only valid when N is 0 it seems
call fdc_write_command

;; There will be a delay here before the first byte of a sector is ready and 
;;  interrupts can be active.
;; 
;; The FDC is reading from the track. It is searching for an ID field that
;; matches the values we have sent in the command.
;; 
;; When it finds the ID field, there is furthur time before the data field 
;; of the sector is found and it starts to read.
;; 
;; Once it has found the data, we must read it all and quickly.
;;


;; interrupts must be off now for data to be read successfully.
;;
;; The CPU constantly asks the FDC if there is data ready, if there is
;; it reads it from the FDC and stores it in RAM. There is a timing
;; constraint, the FDC gives the CPU a byte every 32microseconds.
;; If the CPU fails to read one of the bytes in time, the FDC will report
;; an overrun error and stop data transfer.


;; current address to write data too.
ld de,(#data_ptr)

;; this is the main loop
;; which reads the data
;; The FDC will give us a byte every 32us (double density disc format).
;;
;; We must read it within this time.

fdc_data_read: 
in a,(c)				          ;; FDC has data and the direction is from FDC to CPU
jp p,fdc_data_read		;; 
and #0x20					;; "Execution phase" i.e. indicates reading of sector data
jp z,fdc_read_end 		

inc c					;; BC = I/O address for FDC data register
in a,(c)				;; read from FDC data register
ld (de),a				;; write to memory
dec c					;; BC = I/O address for FDC main status register
inc de					;; increment memory pointer
jp fdc_data_read

fdc_read_end:
;; Interrupts can be enabled now we have completed the data transfer



;; we will get here if we successfully read all the sector's data
;; OR if there was an error.

;; read the result 
call fdc_result

;; check result
ld ix,#result_data
ld c,#0x54
ld a,0(ix)
cp #0x40
jp z,nerr
ld a,1(ix)
cp #0x80
jp z,nerr
ld c,#0x40
nerr:

;; decrease number of sectors transferred
ld a,(#sector_count)
dec a
jp z,read_done
ld (#sector_count),a

;; update ram pointer for next sector
ld hl,(#data_ptr)
ld bc,#512
add hl,bc
ld (#data_ptr),hl

;; update sector id (loops #0x41-#0x49).
ld a,(#sector)
inc a
ld (#sector),a
cp #0x4a        ;; #0x49+1 (last sector id on the track+1)
jp nz,read_sectors
;; we read sector #0x49, the last on the track.
;; Update track variable so we seek to the next track before
;; reading the next sector
ld a,(#track)
inc a
ld (#track),a

ld a,#0x41      ;; #0x41 = first sector id on the track
ld (#sector),a
jp read_sectors_new_track

read_done:
ld bc,#0xfa7e
ld a,#0
out (c),a ;stop the motor
jp 0x100 ;start FUZIX

;;===============================================
;; send command to fdc
;;

fdc_write_command:


ld bc,#0xfb7e					;; I/O address for FDC main status register
push af						;;
fwc1: in a,(c)				;; 
add a,a						;; 
jr nc,fwc1					;; 
add a,a						;; 
jr nc,fwc2					;; 
pop af						;; 
ret							

fwc2: 
pop af				;; 

inc c						;; 
out (c),a					;; write command byte 
dec c						;; 

;; some FDC documents say there must be a delay between each
;; command byte, but in practice it seems this isn't needed on CPC.
;; Here for compatiblity.
ld a,#5				;;
fwc3: dec a			;; 
jr nz,fwc3			;; 
ret							;; 

;;===============================================
;; get result phase of command
;;
;; timing is not important here

fdc_result:

ld hl,#result_data 
ld bc,#0xfb7e
fr1:
in a,(c)
cp #0xc0 
jr c,fr1
 
inc c 
in a,(c) 
dec c 
ld (hl),a 
inc hl 

ld a,#5 
fr2: 
dec a 
jr nz,fr2
in a,(c) 
and #0x10 
jr nz,fr1


ret 

;;===============================================

;; physical drive 
;; bit 1,0 are drive, bit 2 is side.
drive:
.db 0

;; physical track (updated during read)
track:
.db 0

;; id of sector we want to read (updated during read)
sector:
.db 0

;; number of sectors to read (updated during read)
sector_count:
.db 2 ;; enough for now

;; address to write data to (updated during read)
data_ptr:
.ds 2

;;===============================================

fdc_seek:
ld a,#0b00001111    ;; seek command
call fdc_write_command
ld a,(#drive)
call fdc_write_command
ld a,(#track)
call fdc_write_command

call fdc_seek_or_recalibrate
jp nz,fdc_seek
ret

;;===============================================

fdc_recalibrate:

;; seek to track 0
ld a,#0b111						;; recalibrate
call fdc_write_command
ld a,(#drive)					;; drive
call fdc_write_command

call fdc_seek_or_recalibrate
jp nz,fdc_recalibrate
ret

;;===============================================
;; NZ result means to retry seek/recalibrate.

fdc_seek_or_recalibrate:
ld a,#0b1000						;; sense interrupt status 
call fdc_write_command
call fdc_result

;; recalibrate completed?
ld ix,#result_data
bit 5,0(ix)            ;; Bit 5 of Status register 0 is "Seek complete"
jr z,fdc_seek_or_recalibrate
bit 4,0(ix)          ;; Bit 4 of Status register 0 is "recalibrate/seek failed"
;;
;; Some FDCs will seek a maximum of 77 tracks at one time. This is a legacy/historical
;; thing when drives only had 77 tracks. 3.5" drives have 80 tracks.
;;
;; If the drive was at track 80 before the recalibrate/seek, then one recalibrate/seek 
;; would not be enough to reach track 0 and the fdc will then report an error (meaning
;; it had seeked 77 tracks and failed to reach the track we wanted).
;; We repeat the recalibrate/seek to finish the movement of the read/write head.
;;
ret

;;===============================================

file_buffer:
.dw 0x100
result_data:
.ds 8
end: