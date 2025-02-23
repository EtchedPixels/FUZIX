#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <blkdev.h>

uint8_t plt_param(char *p)
{
	return 0;
}

/* Nothing to do for the map of init */
void map_init(void)
{
#if defined EXTENDED_RAM_512 || defined EXTENDED_RAM_1024	
	uint_fast8_t i;
	for (i = 0; i < MAX_SWAPS; i++)
		swapmap_init(i);
#endif
}

void plt_copyright(void)
{
	kprintf("Amstrad CPC6128 platform\nCopyright (c) 2024-2025 Antonio J. Casado Alias\n");
}

#if defined CONFIG_USIFAC_SERIAL
void usifac_serial_init()
{
	char c;
	kprintf("Configuring Usifac serial port\n");
	usifctrl = USIFAC_RESET_COMMAND;
	usifctrl = USIFAC_CLEAR_RECEIVE_BUFFER_COMMAND;
	usifctrl = USIFAC_DISABLE_BURST_MODE_COMMAND;
	usifctrl = USIFAC_DISABLE_DIRECT_MODE_COMMAND;
	usifctrl = USIFAC_SET_115200B_COMMAND;
	while (usifctrl == 0xff)
		c=usifdata; /*flush transmit buffer*/
	c=usifspr; /*read baudrate*/
	if (c == USIFAC_SET_115200B_COMMAND)
	{
		kprintf("Usifac serial port configured at 115200 baud\n");
	/*	usifac_present = 1;*/
	}
}
	#endif