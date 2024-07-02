/*
 *	This is intended for the NC100. It assumes 24 hour clock mode, no
 *	timer or alarm running. It also for now assumes a 1990 base as the
 *	NC100 does.
 *
 *	This code breaks in 2089.
 */

#include <kernel.h>
#include <kdata.h>
#include <stdbool.h>
#include <printf.h>
#include <rtc.h>

#define CLOCK_PORT	0xD0

static uint8_t rtc_buf[7];

static void read_clock(void) __naked
{
    __asm
        ; select the time page (we should do this at init) ?
        xor a
        out (CLOCK_PORT+13),a

retry:
        ; ini also decrements b so twice count
        ld bc,#(6*256 + CLOCK_PORT+12)
        ld hl,#_rtc_buf

        ld d,#2		; loop count

        in a,(CLOCK_PORT)
        ld e,a		; seconds on read start
        
        ; We do two passes of half of the data because there is a week digit
        ; stuck in the middle we dont want

l1:     ini		; high bits of field in low bits of (hl)
        dec c
        in a,(c)
        dec hl
        rld		; I have always wanted an excuse to use RLD 8)
        inc hl
        dec c
        djnz l1
        ld b,#6		; twice count
        dec c		; Skip day of week
        dec d		; Loop count
        jr nz, l1
        ; check if seconds changed (we read them last)
        dec hl		; back to seconds
        ld a,(hl)	; HL still points at the seconds
        xor e
        and #0x0f	; check low 4 bits stable
        jr nz, retry
        ret
    __endasm;
}


uint_fast8_t plt_rtc_secs(void) __naked
{
    __asm
        in a, (CLOCK_PORT)
        ld l,a
        ret
    __endasm;
}

/* Full RTC support (for read - no write yet) */
int plt_rtc_read(void)
{
	uint16_t len = sizeof(struct cmos_rtc);
	uint16_t y;
	struct cmos_rtc cmos;
	uint8_t *p = cmos.data.bytes;

	if (udata.u_count < len)
		len = udata.u_count;

        /* Places 7 BCD pairs into rtc_buf */
	read_clock();

	y = *rtc_buf;
	/* 1990 based year , if its 0x10 or more we are in 2000-2089 and
	   need to adjust the clock up by 2000 and back by 10. If not well
	   then its 1990-1999 so just needs 0x1990 adding to the 0-9 we have */
	if (y >= 0x10)
	    y += 0x19F0;
        else
            y += 0x1990;
	*p++ = y >> 8;
	*p++ = y;
	*p++ = rtc_buf[1];	/* month */
	*p++ = rtc_buf[2];	/* day */
	*p++ = rtc_buf[3];	/* Hour */
	*p++ = rtc_buf[4];	/* Minute */
	*p = rtc_buf[5];	/* Second */
	cmos.type = CMOS_RTC_BCD;
	if (uput(&cmos, udata.u_base, len) == -1)
		return -1;
	return len;
}

int plt_rtc_write(void)
{
	udata.u_error = -EOPNOTSUPP;
	return -1;
}
