PROVIDE (ets_putc = 0x40002be8);
PROVIDE (uart_div_modify = 0x400039d8);
PROVIDE (ets_update_cpu_frequency = 0x40002f04);
PROVIDE (ets_uart_printf = 0x40002544);
PROVIDE (Cache_Read_Disable = 0x400047f0);
PROVIDE (Cache_Read_Enable = 0x40004678);
PROVIDE (SpiUnlock = 0x40004878);
PROVIDE (SpiEraseSector = 0x40004a00);
PROVIDE (SpiWrite = 0x40004a4c);
PROVIDE (SpiRead = 0x40004b1c);
PROVIDE (Wait_SPI_Idle = 0x4000448c);
PROVIDE (_xtos_set_exception_handler = 0x40000454);
PROVIDE (sdk_flashchip = 0x3fffc714);

PROVIDE (__udivsi3 = 0x4000e21c);
PROVIDE (__umodsi3 = 0x4000e268);

