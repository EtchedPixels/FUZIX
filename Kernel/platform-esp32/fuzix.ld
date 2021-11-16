/*
 * GNU linker script for Espressif ESP32
 */

/* Default entry point */
ENTRY(__pro_cpu_entry);

/* Specify main memory areas */
MEMORY
{
  /* Use values from the ESP-IDF 'bootloader' component.
  /* TODO: Use human-readable lengths */
  /* TODO: Use the full memory map - this is just a test */
  iram_seg ( RX )       : ORIGIN = 0x400a0000, len = 0x20000
  dram_seg ( RW )       : ORIGIN = 0x3FFae000, len = 72*1024
}

UART0 = 0x3ff40000;
SPI1 = 0x3ff42000;
SPI0 = 0x3ff43000;
GPIO = 0x3ff44000;
SIGMADELTA = 0x3ff44f00;
RTCCNTL = 0x3ff48000;
RTCIO = 0x3ff48400;
SENS = 0x3ff48800;
HINF = 0x3ff4B000;
UHCI1 = 0x3ff4C000;
I2S0 = 0x3ff4F000;
UART1 = 0x3ff50000;
I2C0 = 0x3ff53000;
UHCI0 = 0x3ff54000;
HOST = 0x3ff55000;
RMT = 0x3ff56000;
RMTMEM = 0x3ff56800;
PCNT = 0x3ff57000;
SLC = 0x3ff58000;
LEDC = 0x3ff59000;
MCPWM0 = 0x3ff5E000;
TIMERG0 = 0x3ff5F000;
TIMERG1 = 0x3ff60000;
SPI2 = 0x3ff64000;
SPI3 = 0x3ff65000;
SYSCON = 0x3ff66000;
I2C1 = 0x3ff67000;
SDMMC = 0x3ff68000;
EMAC_DMA = 0x3ff69000;
EMAC_EXT = 0x3ff69800;
EMAC_MAC = 0x3ff6A000;
TWAI = 0x3ff6B000;
CAN = 0x3ff6B000;
MCPWM1 = 0x3ff6C000;
I2S1 = 0x3ff6D000;
UART2 = 0x3ff6E000;

/* Define output sections */
SECTIONS {
  /* The program code and other data goes into Instruction RAM */
  .iram.text :
  {
    . = ALIGN(16);
    KEEP(*(.entry.text))
    *(.text)
    *(.text*)
    KEEP (*(.init))
    KEEP (*(.fini))
    *(.rodata)
    *(.rodata*)

    . = ALIGN(4);
    _etext = .;
  } >iram_seg

  /* Initialized data goes into Data RAM */
  _sidata = .;
  .data : AT(_sidata)
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)

    . = ALIGN(4);
    _edata = .;
  } >dram_seg

  /* Uninitialized data also goes into Data RAM */
  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;
  } >dram_seg

  . = ALIGN(4);
  PROVIDE ( end = . );
  PROVIDE ( _end = . );
}
