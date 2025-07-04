#include <kernel.h>
#include <devtty.h>
#include <printf.h>
#include "plt_ch375.h"

extern int8_t n_valid_maps;
extern uint8_t valid_maps_array[MAX_MAPS];

/* TODO: probe banks */
void pagemap_init(void)
{
 /* 0xC2 is kernel, valid maps are validated and stored in cpcsme.s*/
	for (int8_t i = n_valid_maps - 1; i >= 0 ; i--) /*We go backwards as lower banks are faster to map*/
		pagemap_add(valid_maps_array[i]);
}

/* Nothing to do for the map of init but we do set our vectors up here */
void map_init(void)
{

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

#if (defined CONFIG_USIFAC_SERIAL || defined CONFIG_USIFAC_CH376)
void usifac_flush(){
	char c;
	while ((usifctrl == 0xff)){
		c=usifdata; /*flush transmit buffer*/
		/*kprintf("%2x:",c);*/
	}
	/*kprintf("\n");*/
}
void usifac_init()
{
	kprintf("Configuring Usifac\n");
	if (usifexists == 255){
		kprintf("Usifac not present\n");
		return;
	}
#if (defined CONFIG_USIFAC_SERIAL && !(defined CONFIG_USIFAC_CH376))
	usifctrl = USIFAC_SET_115200B_COMMAND;
	usifac_flush();
#endif
#if (!(defined CONFIG_USIFAC_SERIAL) && (defined CONFIG_USIFAC_CH376))
	usifctrl = USIFAC_SET_9600B_COMMAND;
	usifac_flush();
	usifdata = 0x57;
	usifdata = 0xAB;
	usifdata = 0x02; /*CH375_CMD_SET_BAUDRATE*/
	usifdata = 0x03;
	usifdata = 0xFA;
	usifctrl = USIFAC_SET_1MBPS_COMMAND;
	usifac_flush();
#endif
	switch (usifgetbaud){
	case USIFAC_SET_1MBPS_COMMAND:
		kprintf("Usifac CH376 module serial comunication configured at 1MBPS\n");
		break;
	case USIFAC_SET_115200B_COMMAND:
		kprintf("Usifac serial port configured at 115200 baud\n");
		break;
	default:
		kprintf("Error configuring Usifac, baudcode:%u\n",usifgetbaud);
	}
}
#endif

void device_init(void)
{
#if (defined CONFIG_USIFAC_SERIAL || defined CONFIG_USIFAC_CH376)
	usifac_init();
#endif
#if (defined CONFIG_ALBIREO || defined CONFIG_USIFAC_CH376)
	ch375_probe();
#endif

#ifdef CONFIG_TD_IDE
	ide_probe();
#endif
#ifdef CONFIG_NET
	sock_init();
#endif
}