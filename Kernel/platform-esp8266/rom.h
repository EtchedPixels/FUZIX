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
extern void Wait_SPI_Idle(void* flash);

extern void* sdk_flashchip;

struct __exception_frame
{
	uint32_t epc;
	uint32_t ps;
	uint32_t sar;
	uint32_t sp;
	uint32_t a0;
	// note: no a1 here!
	uint32_t a2;
	uint32_t a3;
	uint32_t a4;
	uint32_t a5;
	uint32_t a6;
	uint32_t a7;
	uint32_t a8;
	uint32_t a9;
	uint32_t a10;
	uint32_t a11;
	uint32_t a12;
	uint32_t a13;
	uint32_t a14;
	uint32_t a15;
	uint32_t cause;
};

typedef void (*fn_c_exception_handler_t)(struct __exception_frame *ef, int cause);
extern fn_c_exception_handler_t _xtos_set_exception_handler(int cause, fn_c_exception_handler_t fn);

#endif

