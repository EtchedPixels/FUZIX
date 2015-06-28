
ENTRY(_start)

MEMORY
{
        USER : ORIGIN = 0x8000, LENGTH = 0x7e00
}

SECTIONS
{
        .image : {
                *(.header*)
                *(.text*)
				. = ALIGN(2);
                __data_start = .;
                *(.data*)
                *(.rodata*)
				*(.eh_frame*)
                __data_end = .;
        } > USER
		__data_len = (__data_end - __data_start);

        .bss : {
				. = ALIGN(2);
                __bss_start = .;
                *(.bss* COMMON)
                __bss_end = .;
        } > USER
		__bss_len = (__bss_end - __bss_start);
}

