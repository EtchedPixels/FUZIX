#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <blkdev.h>
#include <devfhd.h>

char spambuf[93];
struct dpb *fhd_dpb;
uint8_t fhd_drive;
uint8_t fhd_op;			/* must follow fhd_drive directly */
uint16_t fhd_sector;
uint16_t fhd_track;

uint8_t probe_fidhd(void) __naked
{
__asm
	xor a
	.db 0xed
	.db 0xfe
	ld l,a
	or a
	ret z
	ld l,h
	ret
__endasm;
}

/*
 *	Install a drive using the given dpb, with spambuf as a buffer
 *	for (currrntly) unused messages on error [93 byte]
 *
 *	Caller sets driveno, returns volume number to use or FF on fail
 */

uint8_t install_fidhd(void) __naked
{
__asm
	push ix
	push iy
	ld bc,#0x00fe		; probably FF for MYZ80 only ?
	ld ix, (_fhd_dpb)
	ld hl, #_spambuf
	ld de, (_fhd_drive)	; d = op e = drive
	ld a,#7
	.db 0xed
	.db 0xfe
	cp #2
	ld l,#0xFF
	jr nz, instbad
	ld l,b
instbad:
	pop iy
	pop ix
	ret
__endasm;
}

/*
 *	Returns 0 for ok, 1/2/FF on error as others
 */
uint8_t flush_fidhd(void) __naked
{
__asm
	push ix
	push iy
	ld ix, (_fhd_dpb)
	ld a, (_fhd_drive)
	ld b,a
	ld a,#6
	.byte 0xed
	.byte 0xfe
	pop iy
	pop ix
	ld h,#0
	ld l,a
	ret z
	ret c
	ld h,b
	ret
__endasm;
}
/*
 *	Returns 0 for OK and if ok also writes the 17 byte DPB for this
 *	drive to ix. In theory the DPB we get handed back could be for a
 * 	whole tonload of formats but the main one we care about is the myz80
 *	8MB image with 16k/track (32 sectors), 512 tracks.
 *
 *	Currently we must do the login at boot time and we can't tolerate
 *	a media change of size. (see read/write case for why).
 */
uint8_t login_fidhd(void) __naked
{
__asm
	push ix
	push iy
	ld bc, (_fhd_drive-1)
	ld ix, (_fhd_dpb)
	ld a, #3
	.byte 0xed
	.byte 0xfe
	pop iy
	pop ix
	ld h,#0
	ld l,a
	ret
__endasm;
}

COMMON_MEMORY

/*
 *	Low level I/O has to live in common as we switch banks around in
 *	order to read into user space or into a paging bank.
 *	
 *	Warning: because our common is copied per instance our variables
 *	in common are map specific.
 * 
 * 	Read a sector. On return 0 = ok, 1 = error FF = mediach
 * 	if recoverable error high byte = error code else 0
 */

uint8_t rw_fidhd(void) __naked
{
__asm
	ld a,(_fhd_op)
	push ix
	push iy
	; We must fetch these while in kernel bank. dataptr will point into
	; the bank we switch to but the dpb is in kernel bank.
	;
	; We rely upon the fact we scan all the drives before userspace
	; runs thus the dpb has been copied into each common copy and is
	; static.
	;
	ld ix, (_fhd_dpb)
	ld iy, (_blk_op+BLKPARAM_ADDR_OFFSET)
	ld de, (_fhd_sector)
	ld hl, (_fhd_track)
	ld bc, (_fhd_drive-1)
	push af
	ld a, (_blk_op+BLKPARAM_IS_USER_OFFSET)
	or a
	jr z, k_map
	cp #1
	call z, map_process_always
	ld a, (_blk_op+BLKPARAM_SWAP_PAGE)
	call nz, map_for_swap
k_map:
	pop af
	.byte 0xed
	.byte 0xfe
	pop iy
	pop ix
	call map_kernel
	ld h,#0
	ld l,a
	ret z
	ret c
	ld h,b
	ret
__endasm;
}
