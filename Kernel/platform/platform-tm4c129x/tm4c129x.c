#include <kernel.h>
#include <kdata.h>
#include "cpu.h"
#include "types.h"
#include "inline-irq.h"
#include "entries.h"
#include "interrupt.h"
#include "systick.h"
#include "serial.h"
#include "ssi.h"
#include "tm4c129x.h"

extern uint8_t _sdata;
extern uint8_t _ebss;

#define KERNEL_END ((uint32_t)((&_ebss) + (CONFIG_IDLETHREAD_STACKSIZE - 4U)))

void isr_svcall(void);

const uint32_t _vectors[] __attribute__((section(".vectors"))) = {
  KERNEL_END,
  (uint32_t)start,
  [2 ... (IRQ_SVC - 1)] = (uint32_t)exception_common,
  [IRQ_SVC] = (uint32_t)isr_svcall,
  [(IRQ_SVC + 1) ... (NR_IRQS - 1)] =
                             (uint32_t)exception_common
};

uint8_t sys_cpu = 0U;
uint8_t sys_cpu_feat = 4U;

uint8_t need_resched = 0U;

#ifdef CONFIG_FLAT

#ifdef ELF32
uint8_t progbase[PROGSIZE];
uaddr_t ramtop = ((uaddr_t)(PROGTOP));
#endif

struct u_data *udata_ptr = NULL;

static u_block udata_blocks[PTABSIZE];

void *get_ustack(void)
{
  if (udata.u_insys) {
    for (;;)
      asm("wfe");
  }
  return ((uint8_t *)((udata_blocks[(udata.u_ptab) - ptab]).u_s))
         + sizeof (udata_blocks[(udata.u_ptab) - ptab]).u_s;
}

uint_fast8_t plt_udata_set(ptptr p)
{
  p->p_udata = &((udata_blocks[p - ptab]).u_d);
  return 0U;
}

struct u_data *set_udata(ptptr p, bool atfork)
{
  int src, dst;
  off_t sp;

  if (atfork) {
    for (src = 0; src < PTABSIZE; src++) {
      if ((&(udata_blocks[src].u_d)) == udata_ptr)
        break;
    }
    if (PTABSIZE == src) {
      for (;;)
        asm("wfe");
    }
    for (dst = 0; dst < PTABSIZE; dst++) {
      if ((&(udata_blocks[dst].u_d)) == (p->p_udata))
        break;
    }
    if (PTABSIZE == dst) {
      for (;;)
        asm("wfe");
    }
    memcpy(&(udata_blocks[dst]),
           &(udata_blocks[src]),
           sizeof(struct u_block));
    sp = (((uint8_t *)(udata_blocks[dst].u_d.u_sp))
          - ((uint8_t *)(udata_blocks[src].u_s)));
    if (0 > sp) {
      for (;;)
        asm("wfe");
    }
    udata_blocks[dst].u_d.u_sp = (((uint8_t *)(udata_blocks[dst].u_s)) + sp);
  }
  return udata_ptr = p->p_udata;
}

#else /* CONFIG_FLAT */
#error Only flat memory currently supported.
#endif /* CONFIG_FLAT */

void tm4c129x_init(void)
{
  unsigned u;
  uintptr_t regaddr;

  udata_ptr = &(udata_blocks[0].u_d);
  ramsize = 256U;
  procmem = 256U;		/* FIXME: should be actual space */

  sysclock_init();
  serial_early_init();
  /* Disable all interrupts */
  for (u = (((inl(NVIC_ICTR)) & NVIC_ICTR_INTLINESNUM_MASK) + 1U),
       regaddr = NVIC_IRQ0_31_CLEAR;
       u;
       u--, regaddr += 4U)
    outl(regaddr, 0xffffffffU);
  /* Attach vector table */
  outl(NVIC_VECTAB, (uint32_t)(_vectors));
  ssi_init(SDCARD_SSI_PORT);
  systick_init();
  serial_late_init();
}

void tm4c129x_modreg(unsigned int adr, uint32_t clr, uint32_t set)
{
  uint32_t regval;
  irqflags_t flags = __hard_di();

  regval = inl(adr);
  regval &= ~clr;
  regval |= set;
  outl(adr, regval);
  __hard_irqrestore(flags);
}

/* Nothing to do for the map on init */
void map_init(void)
{
}

uint_fast8_t plt_param(char *p)
{
  return 0U;
}

void plt_copyright(void)
{
}

void plt_idle(void)
{
  asm("wfe");
}

void plt_discard(void)
{
}

_Noreturn void plt_monitor(void)
{
  for (;;)
    asm("wfe");
}

_Noreturn void plt_reboot(void)
{
  for (;;)
    asm("wfe");
}

void pagemap_init(void)
{
  const size_t gap = (KERNEL_END - ((uint32_t)(&_sdata)));
  unsigned u;

  kmemaddblk(((void *)(_vectors[0])), 262144U - gap);
  for (u = 0U; u < MAX_SWAPS; u++)
    swapmap_init(u);
}

void program_vectors(uint16_t *pageptr)
{
}

void install_vdso(void)
{
}

void copy_blocks(void *to, void *from, size_t len)
{
  if (to != from) {
    for (; len; len--) {
      memcpy(to, from, 512U);
      to = (((uint8_t *)(to)) + 512U);
      from = (((uint8_t *)(from)) + 512U);
    }
  }
}

static uint8_t swap_buf[512U];

void swap_blocks(void *to, void *from, size_t len)
{
  if (to != from) {
    for (; len; len--) {
      memcpy(swap_buf, to, 512U);
      memcpy(to, from, 512U);
      memcpy(from, swap_buf, 512U);
      to = (((uint8_t *)(to)) + 512U);
      from = (((uint8_t *)(from)) + 512U);
    }
  }
}
