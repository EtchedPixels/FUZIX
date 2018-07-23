;
; dispatches vt calls to different screen routines
;

	.module	multihead

	.area .data

	.globl _curtty
_curtty	.db 0

	.area .text

	.globl _clear_across
_clear_across:
	tst	_curtty
	lbeq	_m6847_clear_across
	jmp	_crt9128_clear_across

	.globl _clear_lines
_clear_lines:
	tst	_curtty
	lbeq	_m6847_clear_lines
	jmp	_crt9128_clear_lines

	.globl _scroll_up
_scroll_up:
	tst	_curtty
	lbeq	_m6847_scroll_up
	jmp	_crt9128_scroll_up

	.globl _scroll_down
_scroll_down:
	tst	_curtty
	lbeq	_m6847_scroll_down
	jmp	_crt9128_scroll_down

	.globl _plot_char
_plot_char:
	tst	_curtty
	lbeq	_m6847_plot_char
	jmp	_crt9128_plot_char

	.globl _cursor_off
_cursor_off:
	tst	_curtty
	lbeq	_m6847_cursor_off
	jmp	_crt9128_cursor_off

	.globl _cursor_on
_cursor_on:
	tst	_curtty
	lbeq	_m6847_cursor_on
	jmp	_crt9128_cursor_on

	.globl _cursor_disable
_cursor_disable:
	rts

	.globl _vtattr_notify
_vtattr_notify:
	tst	_curtty
	lbeq	_m6847_vtattr_notify
	jmp	_crt9128_vtattr_notify

	.globl _video_cmd
_video_cmd:
	tst	_curtty
	lbeq	_m6847_video_cmd
	jmp	_crt9128_video_cmd

	.globl _video_read
_video_read:
	tst	_curtty
	lbeq	_m6847_video_read
	jmp	_crt9128_video_read

	.globl _video_write
_video_write:
	tst	_curtty
	lbeq	_m6847_video_write
	jmp	_crt9128_video_write
