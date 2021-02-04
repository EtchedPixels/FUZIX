PROVIDE (Cache_Read_Disable = 0x400047f0);
PROVIDE (Cache_Read_Enable = 0x40004678);
PROVIDE (SpiEraseSector = 0x40004a00);
PROVIDE (SpiFlashCnfig = 0x40004568);
PROVIDE (SpiReadModeCnfig = 0x400048ec);
PROVIDE (SpiRead = 0x40004b1c);
PROVIDE (SpiUnlock = 0x40004878);
PROVIDE (SpiWrite = 0x40004a4c);
PROVIDE (Wait_SPI_Idle = 0x4000448c);
PROVIDE (_xtos_set_exception_handler = 0x40000454);
PROVIDE (ets_putc = 0x40002be8);
PROVIDE (ets_uart_printf = 0x40002544);
PROVIDE (ets_update_cpu_frequency = 0x40002f04);
PROVIDE (rom_i2c_writeReg = 0x400072d8);
PROVIDE (sdk_flashchip = 0x3fffc714);
PROVIDE (uart_div_modify = 0x400039d8);
PROVIDE (ets_isr_attach = 0x40000f88);
PROVIDE (ets_isr_unmask = 0x40000fa8);
PROVIDE (ets_isr_mask = 0x40000f98);

PROVIDE (__udivsi3 = 0x4000e21c);
PROVIDE (__umodsi3 = 0x4000e268);

