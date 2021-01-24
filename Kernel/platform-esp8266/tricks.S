.global udata
.comm udata, 512

.global platform_switchout
.text
.align 4
platform_switchout:
	ret

.global switchin
.text
.align 4
switchin:
	ret

.global dofork
.text
.align 4
dofork:
	ret

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

