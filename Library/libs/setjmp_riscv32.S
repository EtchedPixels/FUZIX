/*
	From MUSL

musl as a whole is licensed under the following standard MIT license:

----------------------------------------------------------------------
Copyright © 2005-2020 Rich Felker, et al.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
----------------------------------------------------------------------

*/

.global __setjmp
.global _setjmp
.global setjmp
.type __setjmp, %function
.type _setjmp,  %function
.type setjmp,   %function
__setjmp:
_setjmp:
setjmp:
	sw s0,    0(a0)
	sw s1,    4(a0)
	sw s2,    8(a0)
	sw s3,    12(a0)
	sw s4,    16(a0)
	sw s5,    20(a0)
	sw s6,    24(a0)
	sw s7,    28(a0)
	sw s8,    32(a0)
	sw s9,    36(a0)
	sw s10,   40(a0)
	sw s11,   44(a0)
	sw sp,    48(a0)
	sw ra,    52(a0)

#ifndef __riscv_float_abi_soft
	fsd fs0,  56(a0)
	fsd fs1,  64(a0)
	fsd fs2,  72(a0)
	fsd fs3,  80(a0)
	fsd fs4,  88(a0)
	fsd fs5,  96(a0)
	fsd fs6,  104(a0)
	fsd fs7,  112(a0)
	fsd fs8,  120(a0)
	fsd fs9,  128(a0)
	fsd fs10, 136(a0)
	fsd fs11, 144(a0)
#endif

	li a0, 0
	ret
.global __longjmp
.global _longjmp
.global longjmp
.type __longjmp, %function
.type _longjmp,  %function
.type longjmp,   %function
__longjmp:
_longjmp:
longjmp:
	lw s0,    0(a0)
	lw s1,    4(a0)
	lw s2,    8(a0)
	lw s3,    12(a0)
	lw s4,    16(a0)
	lw s5,    20(a0)
	lw s6,    24(a0)
	lw s7,    28(a0)
	lw s8,    32(a0)
	lw s9,    36(a0)
	lw s10,   40(a0)
	lw s11,   44(a0)
	lw sp,    48(a0)
	lw ra,    52(a0)

#ifndef __riscv_float_abi_soft
	fld fs0,  56(a0)
	fld fs1,  64(a0)
	fld fs2,  72(a0)
	fld fs3,  80(a0)
	fld fs4,  88(a0)
	fld fs5,  96(a0)
	fld fs6,  104(a0)
	fld fs7,  112(a0)
	fld fs8,  120(a0)
	fld fs9,  128(a0)
	fld fs10, 136(a0)
	fld fs11, 144(a0)
#endif

	seqz a0, a1
	add a0, a0, a1
	ret
