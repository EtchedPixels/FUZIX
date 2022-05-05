;
;	No CPU4/6 detection yet
;
		.export _identify_cpu

		.setcpu 6
		.code

_identify_cpu:
		lda	6
		rsr
