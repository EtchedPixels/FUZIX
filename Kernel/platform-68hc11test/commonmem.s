;
;	udata is a bit special we have to copy it on a task switch as we've
;	got almost no common memory space on the simple board design
;
        .file "commonmem"
	.mode mshort

        ; exported symbols
        .globl _ub
        .globl udata
        .globl kstack_top
        .globl istack_top
        .globl istack_switched_sp

        .sect .udata

	.comm udata,512,1
	.comm kstack_top,0,1
	.comm istack_base,254,1
	.comm istack_top,2,1
	.comm istack_switched_sp,2,1
