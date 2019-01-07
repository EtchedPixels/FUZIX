;
; dispatches vt calls to different screen routines
;

	.module	multihead

	.globl _set_vid_mode
	.globl _set_vc_mode

	.area .videodata

	.globl _curtty
_curtty	.db 0

	.area .video

	.globl _clear_across
_clear_across:
	lda	_curtty
	lbeq	_m6847_clear_across
	deca
	lbeq	_crt9128_clear_across
	jmp	_vc_clear_across

	.globl _clear_lines
_clear_lines:
	lda	_curtty
	lbeq	_m6847_clear_lines
	deca
	lbeq	_crt9128_clear_lines
	jmp	_vc_clear_lines

	.globl _scroll_up
_scroll_up:
	lda	_curtty
	lbeq	_m6847_scroll_up
	deca
	lbeq	_crt9128_scroll_up
	jmp	_vc_scroll_up

	.globl _scroll_down
_scroll_down:
	lda	_curtty
	lbeq	_m6847_scroll_down
	deca
	lbeq	_crt9128_scroll_down
	jmp	_vc_scroll_down

	.globl _plot_char
_plot_char:
	lda	_curtty
	lbeq	_m6847_plot_char
	deca
	lbeq	_crt9128_plot_char
	jmp	_vc_plot_char

	.globl _cursor_off
_cursor_off:
	lda	_curtty
	lbeq	_m6847_cursor_off
	deca
	lbeq	_crt9128_cursor_off
	jmp	_vc_cursor_off

	.globl _cursor_on
_cursor_on:
	lda	_curtty
	lbeq	_m6847_cursor_on
	deca
	lbeq	_crt9128_cursor_on
	jmp	_vc_cursor_on

	.globl _cursor_disable
_cursor_disable:
	rts

	.globl _vtattr_notify
_vtattr_notify:
	lda	_curtty
	lbeq	_m6847_vtattr_notify
	deca
	lbeq	_crt9128_vtattr_notify
	jmp	_vc_vtattr_notify

	.globl _video_cmd
_video_cmd:
	tst	_curtty
	lbeq	_m6847_video_cmd
	rts

	.globl _video_read
_video_read:
	tst	_curtty
	lbeq	_m6847_video_read
	rts

	.globl _video_write
_video_write:
	tst	_curtty
	lbeq	_m6847_video_write
	rts

