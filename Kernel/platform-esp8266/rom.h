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
extern void rom_i2c_writeReg(int reg, int hosid, int par, int val); 
extern void SpiFlashCnfig(uint32_t spi_interface, uint32_t spi_freq);
extern int SpiReadModeCnfig(uint32_t mode);

extern void* sdk_flashchip;

//static volatile uint32_t* DPORT_BASEADDR = (volatile uint32_t*)0x3ff00000;

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

typedef void fn_c_exception_handler_t(struct __exception_frame *ef, int cause);
extern fn_c_exception_handler_t* _xtos_set_exception_handler(int cause, fn_c_exception_handler_t* fn);

/* Interrupt handling */

#define ETS_SLC_INUM        1
#define ETS_SDIO_INUM       1
#define ETS_SPI_INUM        2
#define ETS_GPIO_INUM       4
#define ETS_UART_INUM       5
#define ETS_UART1_INUM      5
#define ETS_CCOMPARE0_INUM  6
#define ETS_SOFT_INUM       7
#define ETS_WDT_INUM        8
#define ETS_FRC_TIMER1_INUM 9  /* use edge */

typedef void (*int_handler_t)(void*, struct __exception_frame*);
extern void ets_isr_attach(int intr, int_handler_t handler, void* arg);
extern void ets_isr_mask(int intr);
extern void ets_isr_unmask(int intr);

#endif

