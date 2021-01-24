#ifndef ROM_H
#define ROM_H

extern void ets_putc(char c);
extern void ets_uart_printf(const char* format, ...);
extern void ets_update_cpu_frequency(int mhz);
extern void uart_div_modify(uint8_t uart_no, uint32_t divlatch);
extern void Cache_Read_Enable(uint8_t odd_even, uint8_t mb_count, uint8_t no_idea);
extern void Cache_Read_Disable(void);
extern void SpiWrite(uint32_t address, const uint8_t* buffer, uint32_t length);
extern void SpiRead(uint32_t address, uint8_t* buffer, uint32_t length);
extern void SpiUnlock(void);
extern void SpiEraseSector(uint32_t sector);

#endif

