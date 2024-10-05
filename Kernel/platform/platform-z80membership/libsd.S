;
;	Detect the SD card
;
spiprobe:
	.ascii "probing.."
	.byte 0
spicmd0:
	.ascii "CMD0.."
	.byte 0
spicmd8:
	.ascii "CMD8.."
	.byte 0
spidone:
	.ascii " OK", 13, 10
	.byte 0
spitype:
	.ascii "SPI type "
	.byte 0
spisdhc:
	.ascii "SDHC.."
	.byte 0
spicdi4:
	.ascii "INFO.."
	.byte 0
spiidle:
	.ascii "IDLE.."
	.byte 0
spicm58:
	.ascii "CMD58.."
	.byte 0

sd_spi_init:
	ld hl, spiprobe
	call strout
	xor a                      ; if the timer fires we will return 0
	                           ; and also switch the mmu to bank 0
	ld b, 11	           ; 10 seconds (trim this!)
	call sd_settimer
	ld a, 0xff
	out (SD_DIVISOR), a        ; Low speed for detect
	dec a
	out (SD_CHIPSELECT), a     ; 0xfe - chip select
	xor a
	out (SD_SPIMODE), a

	ld b, 20		   ; now wait 160 clocks
sd_spi_init_pre:
	ld a, 0xFF
	out (SD_TX), a
	in a, (SD_RX)
	djnz sd_spi_init_pre
	ld hl, spicmd0
	call strout
	ld b, SD_CMD0              ; probe the device
	call sd_cmd_nodata
	cp 1
	jr nz, sd_spi_fail         ; Nothing sane
	ld hl, spicmd8
	call strout
	ld b, SD_CMD8              ; Get device info
	ld hl, 0x01AA
	call sd_cmd_data16
	cp 1
	jp nz, sd_notsdhc          ; Something but not SDHC TODO jr
	push hl
	ld hl, spisdhc
	call strout
	pop hl
	call sd_cdinfo4            ; HL now points past the data
	push hl
	ld hl, spicdi4
	call strout
	pop hl
	dec hl
	ld a, (hl)                 ; check if we are 3.3v
	cp 0xaa
	jr nz, sd_spi_fail         ; Not 3.3v capable
	dec hl
	ld a, (hl)
	cp 0x01
	jr nz, sd_spi_fail         ; Not 3.3v capable
	ld hl, 0
	ld de, 0x4000
sd_spi_idleout:
	push hl
	ld hl, spiidle
	call strout
	pop hl
	ld b, SD_ACMD41            ; Leave idle, HCS
	call sd_cmd
	jr nz, sd_spi_idleout
	push hl
	ld hl, spicm58
	call strout
	pop hl
	ld b, SD_CMD58             ; nearly done
	call sd_cmd_nodata
	jr nz, sd_spi_fail
	; Success - we have an SDHC card
	call sd_cdinfo4
	ld a, 2
	out (SD_DIVISOR), a
	ld c, CT_SD2
	ld a, (sd_cdinfobuf)
	bit 6, a                   ; check type
	jr z, sd_spi_done
	ld c, CT_BLOCK_SD2
	jr sd_spi_done
sd_spi_fail:ld c, 0                    ; 0 is our error return
sd_spi_done:ld a, 0xff
	out (SD_CHIPSELECT), a     ; deselect the chip
	out (SD_TX), a
	in a, (SD_RX)              ; mop up
	ld a, c                    ; return code into a
	cp CT_MMC
	ld a, 2
	jr nz, sd_clock2
	inc a                      ; SD is slower
sd_clock2:  out (SD_DIVISOR), a
	call sd_stoptimer
	ld a, c
	ld (sd_cardtype), a
	ld hl, spitype
	call strout
	ld a, (sd_cardtype)
	call outcharhex
	ld hl, spidone
	call strout
	or a
	ret

sd_doacmd41:  .ascii "ACMD41..",0
sd_acmd41_loop: .ascii ".",0
sd_docmd_1:  .ascii "CMD1..",0
sd_docmd_16:  .ascii "CMD16..",0

sd_notsdhc: ; SDSC or MMC ?
	ld hl, sd_doacmd41
	call strout
	ld b, SD_ACMD41
	call sd_cmd_nodata
	cp 2
	jr nc, sd_mmc3              ; SD or MMC ?
sd_sd1_idle:
	ld hl, sd_acmd41_loop
	call strout
	ld b, SD_ACMD41            ; wait for ready
	call sd_cmd_nodata
	jr nz, sd_sd1_idle
	ld c, CT_SD1               ; set type
sd_blocklen:
	push bc                    ; save return code
	ld hl, sd_docmd_16
	ld b, SD_CMD16
	ld hl, 512
	call sd_cmd_data16
	pop bc
	jr nz, sd_spi_fail
	jr sd_spi_done             ; successful detection

sd_mmc3:    ld hl, sd_docmd_1
	ld b, SD_CMD1              ; MMC card
	call sd_cmd_nodata
	jr nz, sd_mmc3             ; wait for the card
	ld c, CT_MMC
	jr sd_blocklen

;
;	Get size data, return != 0 on error
;
sd_size:
	ld b, SD_CMD9
        call sd_cmd_nodata
        ret nz
        ld bc, 16
        call sd_cdinfo
        or a
        ret nz
        ld a, (sd_cdinfobuf)
        bit 6, a
        jr nz, sd_size_sdc2
        ;
        ;   Messy type
        ;
         ;
        xor a 
        ret
sd_size_sdc2:
        ;   Saner block descriptions
        ;  9 + 8 << 8 + 1   Kilosectors
        xor a
        ret

; clock out four bytes into the buffer sd_cdinfobuf
sd_cdinfo4:
	ld b, 4
; entry point for different sizes (16 max)
sd_cdinfo:
	ld hl, sd_cdinfobuf
; entry point for straight copy to HL
sd_spi_d0:
	ld a, 0xff                 ; Copy the response
	out (SD_TX), a
	in a, (SD_RX)
	ld (hl), a
	inc hl
	djnz sd_spi_d0
	ret

;
;  Wait for the SD to become ready - receive until we stop getting 0xFF
;  back from the card. On entry c always holds SD_TX
;
sd_wait_ready:
	; Preserve all but A
	ld a, 0xff
	out (c), a
	in a, (SD_RX)
sd_wait_ready2:
	ld a, 0xff
	out (c), a
	in a, (SD_RX)
	cp 0xff
	jr nz, sd_wait_ready2
	ret

;
; Wait to receive a block, on entry C is alway SD_RX
; Must preserve B
;
sd_wait_rx:
	ld a, 0xff
	out (SD_TX), a
	in a, (c)
;	call outcharhex
	cp 0xff
	jr z, sd_wait_rx
	xor 0xfe        ; return 0 if 0xfe
	ret z
	ld a, 1         ; CP/M physical error
	ret
	
;
;   Main command processor. This is slightly complicated by the need to
;   juggle a 32 bit argument and a command in registers, as well as the
;   fact that we need to consider that some commands are sent with CMD55
;   first as sort of 'shift key'. We also fake CRC values on a couple of
;   commands as needed by the protocol rather than doing real CRC.
;
;   d,e,h,l holds the argument and b holds the command
;
sd_cmd_nodata:
	ld hl, 0        ; low 16bits of arg in HL
sd_cmd_data16:
	ld de, 0        ; high bits in de
sd_cmd:     ; command b with 0 arg and not an ACMD 
	bit 7, b
	; ACMD ?
	jr z, sd_notacmd
	; Send CMD 55, 0 first
	; use the alternate register bank as we don't save it for IRQs
	; or stuff like that
	exx
	ld b, SD_CMD55
	call sd_cmd_nodata
	exx
	res 7, b
sd_notacmd:
	ld a, 0xff               ; deselect
	ld c, SD_TX
	out (SD_CHIPSELECT), a
	dec a                    ; reselect (0xfe)
	out (SD_CHIPSELECT), a
	call sd_wait_ready       ; wait for the card to respond
	out (c), b               ; send the command and data
	out (c), d
	out (c), e
	out (c), h
	out (c), l
	ld a, b                  ; check for the CRC cases
	ld e, 0x01               ; no CRC
	cp SD_CMD0
	jr nz, sd_notcmd0
	ld e, 0x95
	jr sd_crc_tx
sd_notcmd0: cp SD_CMD8
	jr nz, sd_crc_tx
	ld e, 0x87
sd_crc_tx:  out (c), e               ; send the correct fudged CRC
	cp SD_CMD12
	jr nz, sd_noskip         ; might be better to just offset bump read?
	ld a, 0xff
	out (c), a
	in a, (SD_RX)       ; Meh - we should fix the hw so rx and tx
				; are the same offset!
sd_noskip:  ld b, 10
sd_cmd_rx:  ld a, 0xff
	out (c), a
	in a, (SD_RX)
	bit 7, a
	jr z, sd_cmd_out
	djnz sd_cmd_rx
sd_cmd_out: or a                ; Many callers want to check v 0
	ret            



;
;   Block offset into d,e,h,l
;
sd_make_arg:
	ld a, (sd_cardtype)
	cp CT_BLOCK_SD2
	jr z, sd_make_block
	; 16384 * 128 byte records / track
	; deblocked by CP/M so we see 512 byte size request/offset
	; ie 4096 blocks / track
	; 000TTTTT TTTSSSSS SSSSSSS0 00000000
	ld bc, (cursector)
	sla c
	rl b
	ld a, (curtrack)
	ld e, 0
	ld l, e    ; l in the final result is fixed at 0
	rrca
	rr e
	rrca
	rr e
	rrca
	rr e
	and 0x1F
	ld d, a    ; High bits
	ld a, b
	and 0x1f
	or e
	ld e, a    ; bits 24-16
	ld h, c    ; bits 15-8
	ret


sd_make_block:
;
;   Block addressed
;   00000000 0000TTTTTTTTSSSS SSSSSSSS
;
;
;	ld hl, mkblk
;	call strout
	ld a, (curtrack)
;	call outcharhex
	rlca                  ; Swap the nibbles over
	rlca
	rlca
	rlca
	ld d, a               ; Save a copy
	and 0x0f              ; top nibble
	ld e, a               ; save in result e
	ld hl, (cursector)    ; sector forms the low 12 bits
	ld a, d               ; high bits from swapped track nibble
	and 0xf0              ; drop the low bits
	or h                  ; add sector
	ld h, a               ; return in h
	ld d, 0               ; top of address is 0, l was set earlier
;	ld a, d
;	call outcharhex
;       ld a, e
;       call outcharhex
;       ld a, h
;       call outcharhex
;       ld a, l
;       call outcharhex
	ret			  ; done
;mkblk:	.ascii 13,10,'MB',0


sd_read_banked:
;
;	Set up for a read. 
;       On return nz is true if an error (and the code is set)
;                 z is true on no error (and the xfer is ready to begin)
;
	call sd_make_arg
	ld b, SD_CMD17
	call sd_cmd
	ld hl, (curdmaaddr)
	ld bc, SD_RX    ; b=0, c=port
	jp z, sd_wait_rx
sd_hw_err:
	ld a, 1
	ret
;
;	Similar for write
;
sd_write_banked:
	call sd_make_arg
	ld b, SD_CMD24
	call sd_cmd
	 ld hl, (curdmaaddr)
	jr nz, sd_hw_err
	ld bc, SD_TX
	call sd_wait_ready
	xor a
	ret

sd_cdinfobuf .ds 16
sd_cardtype .ascii 0
