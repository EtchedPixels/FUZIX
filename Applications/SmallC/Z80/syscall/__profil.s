	.code

	.export __profil

__profil:
	ld hl, 56
	jp __syscall
