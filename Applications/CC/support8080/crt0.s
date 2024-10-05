
		.code
		.setcpu 8080

start:
		call	_main
		push	h
		call	_exit
