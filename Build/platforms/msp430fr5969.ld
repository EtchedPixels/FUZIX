
ENTRY(_start)

MEMORY
{
        USER : ORIGIN = 0x9f00, LENGTH = (0xff80 - 0x9f00)
}

SECTIONS
{
        .image : {
				__image_start = .;
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
		__user_page = (__image_start / 0x100);

        .bss : {
				. = ALIGN(2);
                __bss_start = .;
                *(.bss* COMMON)
                __bss_end = .;
        } > USER
		__bss_len = (__bss_end - __bss_start);
}

