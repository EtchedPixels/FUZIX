;
;	6801/6303 detection. For now we don't try and detect 6800 or HC11
;
		.export _identify_cpu

		.setcpu 6303
		.code

_identify_cpu:
		ldab #1
		ldx #3
		xgdx		; no-op on the 6803
		rts
		
