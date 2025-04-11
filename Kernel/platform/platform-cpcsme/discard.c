#include <kernel.h>
#include <devtty.h>
#include <printf.h>


/* TODO: probe banks */
void pagemap_init(void)
{
 /* 0xC2 is kernel */
 pagemap_add(0xCA);
 pagemap_add(0xD2);
 pagemap_add(0xDA);
 pagemap_add(0xE2);
 pagemap_add(0xEA);
 pagemap_add(0xF2);
 pagemap_add(0xFA);
}

/* Nothing to do for the map of init but we do set our vectors up here */
void map_init(void)
{
 /*if (request_irq(0xE7, tuart0_rx_ring) |
 request_irq(0xEF, tuart0_txd) |
 request_irq(0xF7, tuart0_timer4))
  panic("irqset");*/
 /* We need to claim these in case we set one off as they are at odd vectors
    as the base tu_uart is strapped for 8080 mode */
 /*if (
  request_irq(0xC7, spurious) |
  request_irq(0xCF, spurious) |
  request_irq(0xD7, spurious) |
  request_irq(0xDF, spurious) |
  request_irq(0xFF, spurious)
  )
  panic("irqset2");*/
  /* FIXME: request vectors for uart1 and 2 */
}

uint8_t plt_param(char *p)
{
 used(p);
 return 0;
}

void plt_copyright(void)
{
	kprintf("Amstrad CPC with standard memory expansion platform\nCopyright (c) 2024-2025 Antonio J. Casado Alias\n");
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