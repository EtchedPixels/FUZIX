
		.code
		.setcpu 8085

start:
		call	_main
		push	h
		call	_exit
