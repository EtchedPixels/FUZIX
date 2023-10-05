#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <bios.h>

/*
 *	Everything in this file ends up in discard which means the moment
 *	we try and execute init it gets blown away. That includes any
 *	variables declared here so beware!
 */

void init_hardware_c(void)
{
	uint8_t i;
	bufptr p = bufpool;
	biosinfo = fuzixbios_getinfo();
	ramsize = biosinfo->ram_kb;
	/* Assumes a zero base */
	procmem = ramsize - (biosinfo->common_base >> 10);
	/* Memory allocator base */
	alloc_base = (uint8_t *)biosinfo->bios_top;
	/* Now set up the swap and RAM information */
	swap_size = biosinfo->common_base >> 9;
	ramtop = biosinfo->common_base;
	udata_stash = ramtop - sizeof(u_data);
	if (biosinfo->features & FEATURE_TIMER)
		plt_tick_present = 1;
	if (biosinfo->features & FEATURE_RTC_SLOW)
		rtc_delay = 100;
	/* Get the tty up */
	biostty_init();
	/* Allocate the initial buffers */
	for (i = 0; i < NBUFS; i++) {
		if (buffer_alloc(p) == NULL)
			panic("bufmem");
		p++;
	}
}

/*
 *	We get passed each kernel command line argument. if we return 1 then
 *	we claim it, if not it gets passed to init. It's perfectly acceptable
 *	to act on a match and return to also pass it to init if you need to.
 */
uint8_t plt_param(unsigned char *p)
{
	return fuzixbios_param(p);
}

/*
 *	Set up our memory mappings. This is not needed for simple banked memory
 *	only more complex setups such as 16K paging.
 */
void map_init(void)
{
}

/*
 *	Add all the available pages to the list of pages we an use. If this
 *	is runtime dynamic check to make sure you don't add more than MAX_MAPS
 *	of them. On some machines with a lot of RAM the implementation turns
 *	the excess into a RAM disc
 *
 *	The mapping for FuzixBIOS should be logical numbers 1-n. The
 *	kernel is bank 0.
 */

static struct fuzixbios_callbacks fcb = {
	callback_tick,
	callback_timer,
	callback_tty,
	callback_disk,
	callback_kprintf,
	&int_disabled
};

void pagemap_init(void)
{
	uint8_t i;
	for (i = 1; i < biosinfo->num_banks; i++)
		pagemap_add(i);
	fuzixbios_set_callbacks(&fcb);
}

/*
 *	TTY masks - define which bits can be changed for each port
 *	eventually...
 */

void biostty_init(void)
{
	int n = biosinfo->num_serial;
	uint8_t *pt;
	uint8_t i;
	struct s_queue *q = &ttyinq[1];
	tcflag_t *m = &termios_mask[1];
	struct fuzixbios_ttyparam *p;

	if (n > NUM_DEV_TTY) {
		kprintf("Only %d serial devices will be available.\n",
			n);
		n = NUM_DEV_TTY;
	}
	for (i = 1; i < n; i++) {
		/* Set the termios mask for the port */
		*m++ = p->termios_mask | _CSYS;
		/* Set the initial height and width */
		ttydata[i].winsize.ws_col = p->width;
		ttydata[i].winsize.ws_row = p->height;
		/* If a mode is passed then set this so the port can force
		   parameters it know are relevant - eg boot monitor port
		   configurations */
		if (p->initial_mode)
			ttydata[i].termios.c_cflag = p->initial_mode;
		pt = init_alloc(TTYSIZ);
		if (pt == NULL)
			panic("ttybuf");
		q->q_base = q->q_head = q->q_tail = pt;
		q->q_size = TTYSIZ;
		q->q_count = 0;
		q->q_wakeup = TTYSIZ/2;
		q++;
	}
}

/*
 *	Called after interrupts are enabled in order to enumerate and set up
 *	drives.
 */

void device_init(void)
{
	buffer_init();
	vd_init();
}
