.code 16

.text

.align 4
.thumb_func
.globl set_cpu_type
set_cpu_type:
  bx lr

.align 4
.thumb_func
# On entry:
#   r0: entrypoint
#   r1: sp
.global _doexec
_doexec:
  mov sp, r1
  isb
  bx r0
