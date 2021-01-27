.global udata
.comm udata, 512

.global platform_switchout
.text
switchout_s:
	.string "platform_switchout"
.align 4
platform_switchout:
	movi a2, switchout_s
	j panic

.global switchin
.text
switchin_s:
	.string "platform_switchin"
.align 4
switchin:
	movi a2, switchin_s
	j panic

.global dofork
.text
dofork_s:
	.string "dofork"
.align 4
dofork:
	movi a2, dofork_s
	j panic

.global platform_idle
.text
.align 4
platform_idle:
	ret

.global program_vectors
.text
.align 4
program_vectors:
	ret

