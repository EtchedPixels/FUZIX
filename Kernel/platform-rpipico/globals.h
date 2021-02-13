#ifndef GLOBALS_H
#define GLOBALS_H

#define FLASH_OFFSET (128*1024)

extern void flash_dev_init(void);

struct exception_frame
{
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t pc;
	uint32_t psr;
};

#endif

