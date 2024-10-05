#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#ifdef CONFIG_IBMPC_EMM

/*
 *	Handle EMM memory space using typical implementations of the hardware
 *	with the triple ports (208/9/A or 218/9/A). Assumes only one EMM card
 *
 *	The Intel board uses 0x258 for the port and adds 0x4000 to get higher
 *	decodes and usually maps at C0000. 0x259 controls C000/D000 base
 *	(write 128,0,128,0 for D 128,0,0,0 to 259,4259,8259,C259)
 *
 *	For data write 0x258/4258,8258,C258 with the page number.
 *
 *	TODO:
 *	According to John Elliot the T3100e implements 8MB of EMM 3.2 as if
 *	it was 4 x 2MB boards with intel decode 1 at 208/218/258/268
 *	(and also probes 2A8/2B8/2E8 as alternative EMM ports). We don't at
 *	the moment have a model for multiple EMM cards.
 *
 *	We don't address the case of having two EMM cards with code segments
 *	in one and data the other. It's not clear that is actually very useful
 */

static uint16_t emm[MAX_EMM];
static uint16_t ems_seg;
static uint16_t emm_ptr;
static uint16_t emm_max;
static uint16_t emm_over;	/* EMM we couldn't use for processes but is
                                   a RAMDISC candidate */                            

static uint8_t emsport;		/* Port range - usually 208/9/A */
static uint8_t emsctrl;		/* The bits we set for autoinc, base etc */
static uint16_t emsstep;	/* Step for Intel style ports */
static uint16_t emsoff;		/* Offset to add to emm */

uint16_t emm_alloc_bank(struct proc_mmu *m)
{
    if (emm_ptr == 0) {
        m->emm = 0;
        return ENOMEM;
    }
    m->emm = emm[emm_ptr--];
    m->dseg = ems_seg;
    return 0;
}

void emm_free_bank(struct proc_mmu *m)
{
    emm[++emm_ptr] = m->emm;
}

/*
 *	Select routines. We may need to move these to asm for neatness
 *	with C wrappers for the C invoked cases. Note that the emm can be
 *	any emm code the driver tries to add and isn't limited to MAX_EMM
 *	as the space that didn't fit (if any) will become EMM ramdisc.
 */
/*
 *	Used for things like the Opti chipset support
 */
static void emm_select_1(uint16_t emm)
{
    /* Select first 16K slot, with autoincrement etc */
    outb(emsctrl, emsport + 2);
    for (i = 0; i < 4; i++) {
        outb(0x80 |(emm >> 8), emsport + 2);	/* A22/A23 + Enable */
        outb(emm, emsport + 1);	/* A14 - A 21 , autoincrements bank */
    }
}

/*
 *	Intel AboveBoard, 
 */
static void emm_select_2(uint16_t emm)
{
    unt16_t port = emsport;
    for (i = 0; i < 4; i++) {
        outb(emm + emmoff, port);
        port += emsstep;
    }
}

void emm_select(uint16_emm)
{
#ifdef CONFIG_EMM_BIOS
    if (emm_type == 3)
        emm_select_bios(emm);
    else
#endif
    if (emm_type == 1)
        emm_select_1(emm);
    else
        emm_select_2(emm);
            
}

uint16_t emm_space_used(void)
{
    return (emm_max - emm_ptr) * 64;
}

/* Discard for these eventually */

void emm_new_bank(struct proc_mmu *m)
{
    if (emm_max == MAX_EMM) {
        emm_over++;
        return;
    }
    emm[++emm_ptr] = m->emm;
    emm_max++;
    ramsize += 64;
}

/* FIXME: always needs to run with IRQ off */
uint8_t opti_r(uint8_t c)
{
    outb(c, 0x22);
    return inb(0x23);
}
    
/* Need a probing version of this */
static void do_emm_init(uint16_t base, uint16_t pages)
{
    uint16_t i;
    for (i = 0; i < pages; i++)
        emm_new_bank(base + 4 * i);
}

int opti_emm_init(void)
{
    uint8_t x = opti_r(0x4F);
    
    /* EMS off */
    if ((x & 0xC0) != 0xC0)
        return 0;
        
    if (x & 1)
        emsport = 0x218;
    else
        emsport = 0x208;

    emsctrl = 0x80;	/* D0000, autoinc */
    emstype = 1;
    ems_seg = 0xD000;
    
    /* Add all the memory from 1MB to the top of extended memory as we are
       only using it in EMS mode. Don't get clever and add < 1MB shadow
       space */
    do_ems_init(16, xmm_kb / 64);
    return 1;
}

/* Intel Above Board (original to 2MB) */
int intel_emm_init(uint8_t seg, uint16_t kb)
{
    /* The board lives at 0x258 by default */
    emsport = 0x258;
    emsstep = 0x4000;
    emstype = 2;
    ems_seg = seg;
    emmoff = 0;
    if (seg != 0xC000 && seg != 0xD000)
        return -1;
    /* Program the base address */
    outb(0x80, emsport);
    outb(0x00, emsport + 0x4000);
    outb((seg & 0x1000) >> 9, emsport + 0x8000);
    outb(0x00, emsport + 0xC000);
    /* Set up the pages */
    do_ems_init(0x80, kb / 64);
    return 1;
}

/* Not clear if writing 209 with values sets the high 3 bits for the
   bigger ranges */
int neat_emm_init(uint8_t seg, uint16_t kb)
{
    emsport = 0x208;
    emsstep = 0x4000;
    emstype = 2;
    ems_seg = 0xD000;
    emmoff = 0;
    do_ems_init(0x80, kb / 64);
    return 1;
}

/* Four consecutive write only ports giving each bank. About as simple as
   it gets */
int lotech_emm_init(uint8_t seg, uint16_t kb)
{
    emsport = 0x260;
    emsstep = 1;
    emstype = 2;
    ems_seg = seg;
    emmoff = -1;
    do_ems_init(0x01, kb / 64);
    return 1;
}

#ifdef CONFIG_EMM_BIOS

/* A few emulators don't have any sane EMM interface but do have the int
   hooks in the BIOS environment. That's annoying as **** but we need to
   support this for development work.
   
   We don't behave in a civilised manner we just grab all free pages knowing
   nothing else is running */

static uint16_t handle;

/* This is completely unsafe on a real platform, but it's not a real one
   so it works... */
static void bios_emm_select(uint16_t mm)
{
    uint8_t i;
    for (i = 0; i < 4; i++)
        if (emm_map_memory(i, handle, mm - 1))
            panic("emmfault");
}

int bios_emm_init(void)
{
    int num;
    int handle;
    /* Load the segment from 0x019E and check that :000A contains the ascii
       EMS string */
    if (!emm_detect())
        return 0;
    /* Where does it map ? */
    ems_seg = emm_page_frame_segment();
    if (ems_seg == 0)
        return 0;
    /* How much can we get */
    num = emm_free_pages();
    if (num == 0)
        return 0;
    handle = emm_allocate(num);
    do_ems_init(0x01, num / 4);
    return 1;
}

#endif

#endif
