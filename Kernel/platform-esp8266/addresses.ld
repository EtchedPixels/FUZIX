PROVIDE (Cache_Read_Disable = 0x400047f0);
PROVIDE (Cache_Read_Enable = 0x40004678);
PROVIDE (SpiEraseSector = 0x40004a00);
PROVIDE (SpiFlashCnfig = 0x40004568);
PROVIDE (SpiReadModeCnfig = 0x400048ec);
PROVIDE (SpiRead = 0x40004b1c);
PROVIDE (SpiUnlock = 0x40004878);
PROVIDE (SpiWrite = 0x40004a4c);
PROVIDE (Wait_SPI_Idle = 0x4000448c);
PROVIDE (ets_update_cpu_frequency = 0x40002f04);
PROVIDE (sdk_flashchip = 0x3fffc714);
PROVIDE (uart_div_modify = 0x400039d8);

PROVIDE (__udivsi3 = 0x4000e21c);
PROVIDE (__umodsi3 = 0x4000e268);
PROVIDE (__divsi3 = 0x4000dc88);
PROVIDE (abs = 0x400060c0);

PROVIDE (system_restart = 0x400000a4);
