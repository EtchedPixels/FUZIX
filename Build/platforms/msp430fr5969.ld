
ENTRY(_start)

MEMORY
{
        USER : ORIGIN = 0x8000, LENGTH = 0x7f00
}

SECTIONS
{
        .image : {
                *(.header*)
                *(.text*)
                __data_start = .;
                *(.data*)
                *(.rodata*)
				*(.eh_frame*)
                __data_end = .;
                __data_len = __data_end - __data_start;
        } > USER

        .bss : {
                __bss_start = .;
                *(.bss* COMMON)
                __bss_end = .;
                __bss_len = __bss_end - __bss_start;
        } > USER
}

