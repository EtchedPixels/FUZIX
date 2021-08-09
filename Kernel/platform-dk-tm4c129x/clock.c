/*
 *     Configure the memory and system timing. Hard coded for 120MHz
 */

#include <kernel.h>
#include "tm4c129x.h"

#define SYSCON_ALTCLKCFG        0x400fe138U
#define SYSCON_MEMTIM0          0x400fe0c0U
#define SYSCON_MOSCCTL          0x400fe07cU
#define SYSCON_PLLFREQ0         0x400fe160U
#define SYSCON_PLLFREQ1         0x400fe164U
#define SYSCON_PLLSTAT          0x400fe168U
#define SYSCON_RSCLKCFG         0x400fe0b0U

#define ALTCLKCFG_PIOSC         0U

#define MOSCCTL_OSCRNG          0x10U
#define MOSCCTL_PWRDN           0x08U
#define MOSCCTL_NOXTAL          0x04U

#define MEMTIM0(sysclk) (((sysclk) <=  16000000U) ?  3145776U : \
                         ((sysclk) <=  40000000U) ?  9502865U : \
                         ((sysclk) <=  60000000U) ? 13762770U : \
                         ((sysclk) <=  80000000U) ? 18022675U : \
                         ((sysclk) <= 100000000U) ? 22282580U : \
                         ((sysclk) <= 120000000U) ? 26542485U : 0U)

#define PLL_MINT   96U
#define PLL_MFRAC   0U
#define PLL_N       5U
#define PLL_Q       1U
#define PLL_SYSDIV  4U

#define PLLFREQ0_PLLPWR         (1U << 23U)
#define PLLFREQ0_MINT_SHIFT     0U
#define PLLFREQ0_MINT_MASK      (0x3ffU << PLLFREQ0_MINT_SHIFT)
#define PLLFREQ0_MFRAC_SHIFT    10U
#define PLLFREQ0_MFRAC_MASK     (0x3ffU << PLLFREQ0_MFRAC_SHIFT)
#define PLLFREQ0(mint, mfrac) \
  (((uint32_t)((mint) << PLLFREQ0_MINT_SHIFT)) | \
   ((uint32_t)((mfrac) << PLLFREQ0_MFRAC_SHIFT)))

#define PLLFREQ1_N_SHIFT        0U
#define PLLFREQ1_N_MASK         (31U << PLLFREQ1_N_SHIFT)
#define PLLFREQ1_Q_SHIFT        8U
#define PLLFREQ1_Q_MASK         (31U << PLLFREQ1_Q_SHIFT)
#define PLLFREQ1(q,n) \
  (((uint32_t)(((n) - 1U) << PLLFREQ1_N_SHIFT)) | \
   ((uint32_t)(((q) - 1U) << PLLFREQ1_Q_SHIFT)))

#define PLLSTAT_LOCK            (1U << 0U)

#define RSCLKCFG_PSYSDIV_SHIFT  0U
#define RSCLKCFG_PSYSDIV_MASK   (0x3ffU << RSCLKCFG_PSYSDIV_SHIFT)
#define RSCLKCFG_PSYSDIV(n)     (((uint32_t)(n)) << RSCLKCFG_PSYSDIV_SHIFT)
#define RSCLKCFG_NEWFREQ        (1U << 30U)
#define RSCLKCFG_OSCSRC_MASK    (15U << 20U)
#define RSCLKCFG_OSCSRC_MOSC    (3U << 20U)
#define RSCLKCFG_PLLSRC_MASK    (15U << 24U)
#define RSCLKCFG_PLLSRC_MOSC    (3U << 24U)
#define RSCLKCFG_USEPLL         (1U << 28U)
#define RSCLKCFG_MEMTIMU        (1U << 31U)

static inline uint32_t vco_freq(uint32_t pll0, uint32_t pll1)
{
  uint64_t fvcob10;
  uint32_t mint;
  uint32_t mfrac;
  uint32_t q;
  uint32_t n;
  uint32_t mdivb10;

  mfrac   = (pll0 & PLLFREQ0_MFRAC_MASK) >> PLLFREQ0_MFRAC_SHIFT;
  mint    = (pll0 & PLLFREQ0_MINT_MASK) >> PLLFREQ0_MINT_SHIFT;
  q       = ((pll1 & PLLFREQ1_Q_MASK) >> PLLFREQ1_Q_SHIFT) + 1U;
  n       = ((pll1 & PLLFREQ1_N_MASK) >> PLLFREQ1_N_SHIFT) + 1U;
  mdivb10 = (mint << 10U) + mfrac;
  fvcob10 = (mdivb10 * ((uint64_t)(25000000U))) / (q * n);
  return (uint32_t)(fvcob10 >> 10U);
}

void sysclock_init(void)
{
  uint32_t pll0 = PLLFREQ0(PLL_MINT, PLL_MFRAC);
  uint32_t pll1 = PLLFREQ1(PLL_Q, PLL_N);
  uint32_t r, new_pll;

  r = inl(SYSCON_MOSCCTL);
  r &= ~(MOSCCTL_OSCRNG | MOSCCTL_PWRDN | MOSCCTL_NOXTAL);
  r |= MOSCCTL_OSCRNG;
  outl(SYSCON_MOSCCTL, r);
  r = MEMTIM0(25000000U);
  outl(SYSCON_MEMTIM0, r);

  r = inl(SYSCON_RSCLKCFG);
  r &= ~(RSCLKCFG_PSYSDIV_MASK | RSCLKCFG_OSCSRC_MASK |
         RSCLKCFG_PLLSRC_MASK | RSCLKCFG_USEPLL);
  r |= RSCLKCFG_MEMTIMU;
  outl(SYSCON_RSCLKCFG, r);

  new_pll = ((inl(SYSCON_PLLFREQ1) != pll1) ||
             (inl(SYSCON_PLLFREQ0) != pll0));
  if (new_pll) {
    r = inl(SYSCON_RSCLKCFG);
    r |= (RSCLKCFG_OSCSRC_MOSC | RSCLKCFG_PLLSRC_MOSC);
    outl(SYSCON_RSCLKCFG, r);
    outl(SYSCON_PLLFREQ1, pll1);
    r = inl(SYSCON_PLLFREQ0);
    r &= PLLFREQ0_PLLPWR;
    pll0 |= r;
    outl(SYSCON_PLLFREQ0, pll0);
  }
  r = MEMTIM0(vco_freq(pll0, pll1) / PLL_SYSDIV);
  outl(SYSCON_MEMTIM0, r);

  if (inl(SYSCON_PLLFREQ0) & PLLFREQ0_PLLPWR) {
    if (new_pll) {
      r = inl(SYSCON_RSCLKCFG);
      r |= RSCLKCFG_NEWFREQ;
      outl(SYSCON_RSCLKCFG, r);
    }
  } else {
    r = inl(SYSCON_PLLFREQ0);
    r |= PLLFREQ0_PLLPWR;
    outl(SYSCON_PLLFREQ0, r);
  }

  for (;;) {
    if (inl(SYSCON_PLLSTAT) & PLLSTAT_LOCK) {
      r = inl(SYSCON_RSCLKCFG);
      r |= (RSCLKCFG_PSYSDIV(PLL_SYSDIV - 1U) |
            RSCLKCFG_OSCSRC_MOSC |
            RSCLKCFG_PLLSRC_MOSC |
            RSCLKCFG_USEPLL |
            RSCLKCFG_MEMTIMU);
      outl(SYSCON_RSCLKCFG, r);
      break;
    }
  }

  outl(SYSCON_ALTCLKCFG, ALTCLKCFG_PIOSC);
}
