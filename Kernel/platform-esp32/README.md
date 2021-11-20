## Processor

The ESP32 has two LX6 processor cores with a huge number of architecture
options. You can find the full list here:

https://github.com/espressif/esp-idf/blob/1cb31e50943bb757966ca91ed7f4852692a5b0ed/components/xtensa/esp32/include/xtensa/config/core-isa.h

## Memory map

The ESP32 is semi-Harvard architecture, with everything above 0x40000000 being
instruction memory and everything below being data memory. Instruction memory
can only be read or written in 32-bit words. Some memory blocks are mapped in
both places; but the word order is very different, so you should pick one
method of access and stick to it.

Data:

0x34f0_0000  0x3f7f_ffff  4MB      External flash
0x3f80_0000  0x3fbf_ffff  4MB      External SRAM
0x3ff0_0000  0x3ff7_ffff  512kB    I/O space
0x3ff8_0000  0x3ff8_1fff  8kB      RTC FAST memory
0x3ff9_0000  0x3ff9_ffff  64kB     ROM1 (also readable via instruction bus)
0x3ffa_e000  0x3ffb_ffff  72kB     SRAM2 (static)
0x3ffc_0000  0x3ffd_ffff  128kB    SRAM2 (MMU)
0x3ffe_0000  0x3fff_ffff  128kB    SRAM1

Instructions:

0x4000_0000  0x4005_ffff  384kB    ROM0
0x4007_0000  0x4007_ffff  64kB     SRAM0 (static) / XIP cache
0x4008_0000  0x4009_ffff  128kB    SRAM0 (MMU)
0x400a_0000  0x400b_ffff  128kB    SRAM1
0x400c_0000  0x400c_1fff  8kB      RTC FAST memory
0x400c_2000  0x40bf_ffff  11512kB  External flash

The MMU is very simple and splits the areas up into 16x8kB pages (smaller pages
are possible but then you don't get access to the entire memory space). There's
then a 16-slot lookup table which maps from virtual page to physical page. This
is repeated for both SRAM0 and SRAM2.



