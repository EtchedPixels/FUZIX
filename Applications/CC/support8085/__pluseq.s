;
;		TOS = lval of object HL = amount
;
		.export __pluseq

		.setcpu 8085
		.code
__pluseq:
		xchg			; amount into D
		pop	h		; return
		xthl			; swap with lval
		xchg			; get lval into D
		push	d		; save lval
		push	h		; save value to add
		lhlx			; load it into HL
		pop	d		; get value back
		dad	d		; add __tmp to it
		pop	d		; get the TOS address
		shlx			; store it back
		ret
