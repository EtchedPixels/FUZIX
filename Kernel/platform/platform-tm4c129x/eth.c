#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include "cpu.h"
#include "inline-irq.h"
#include "interrupt.h"
#include "tm4c129x.h"
#include "gpio.h"

#include "eth.h"

#define ETH_PKTSIZE 590U
#define OPTIMAL_EMAC_BUFSIZE ((ETH_PKTSIZE + 4U + 15U) & (~15U))

#define FLASH_USERREG0 0x400fe1e0
#define FLASH_USERREG1 0x400fe1e4

#define SYSCON_SREPHY     0x400fe530
#define SYSCON_SREMAC     0x400fe59c
#define SYSCON_RCGCEPHY   0x400fe630
#define SYSCON_RCGCEMAC   0x400fe69c
#define SYSCON_PCEPHY     0x400fe930
#define SYSCON_PCEMAC     0x400fe99c
#define SYSCON_PREPHY     0x400fea30
#define SYSCON_PREMAC     0x400fea9c

#define SREPHY_R0   1U
#define SREMAC_R0   1U
#define RCGCEPHY_R0 1U
#define RCGCEMAC_R0 1U
#define PCEPHY_P0   1U
#define PCEMAC_P0   1U
#define PREPHY_R0   1U
#define PREMAC_R0   1U

#define ephy_periphrdy() ((bool)(((inl(SYSCON_PREPHY)) & (PREPHY_R0))))
#define emac_periphrdy() ((bool)(((inl(SYSCON_PREMAC)) & (PREMAC_R0))))

#define PHY_RESET_DELAY   65U
#define PHY_CONFIG_DELAY  1000U
#define PHY_READ_TIMEOUT  0x4ffffU
#define PHY_WRITE_TIMEOUT 0x4ffffU
#define PHY_RETRY_TIMEOUT 0x4ffffU

#define INTERNAL_PHY_PHYADDR 0U

#define EPHY_BMCR         0x00U
#define EPHY_BMSR         0x01U
#define EPHY_ID1          0x02U
#define EPHY_ID2          0x03U
#define EPHY_ANA          0x04U
#define EPHY_ANLPA        0x05U
#define EPHY_ANER         0x06U
#define EPHY_ANNPTR       0x07U
#define EPHY_ANLNPTR      0x08U
#define EPHY_CFG1         0x09U
#define EPHY_CFG2         0x0aU
#define EPHY_CFG3         0x0bU
#define EPHY_REGCTL       0x0dU
#define EPHY_ADDAR        0x0eU
#define EPHY_STS          0x10U
#define EPHY_SCR          0x11U
#define EPHY_MISR1        0x12U
#define EPHY_MISR2        0x13U
#define EPHY_FCSCR        0x14U
#define EPHY_RXERCNT      0x15U
#define EPHY_BISTCR       0x16U
#define EPHY_LEDCR        0x18U
#define EPHY_CTL          0x19U
#define EPHY_10BTSC       0x1aU
#define EPHY_BICSR1       0x1bU
#define EPHY_BICSR2       0x1cU
#define EPHY_CDCR         0x1eU
#define EPHY_RCR          0x1fU
#define EPHY_LEDCFG       0x25U

#define EMAC_NRXDESC      8U
#define EMAC_NTXDESC      4U
#define EMAC_NFREEBUFFERS (EMAC_NRXDESC + 1U)

#define EMAC_CFG          0x400ec000
#define EMAC_FRAMEFLTR    0x400ec004
#define EMAC_HASHTBLH     0x400ec008
#define EMAC_HASHTBLL     0x400ec00c
#define EMAC_MIIADDR      0x400ec010
#define EMAC_MIIDATA      0x400ec014
#define EMAC_FLOWCTL      0x400ec018
#define EMAC_VLANTG       0x400ec01c
#define EMAC_IM           0x400ec03c
#define EMAC_ADDR0H       0x400ec040
#define EMAC_ADDR0L       0x400ec044
#define EMAC_MMCRXIM      0x400ec10c
#define EMAC_MMCTXIM      0x400ec110
#define EMAC_DMABUSMOD    0x400ecc00
#define EMAC_TXPOLLD      0x400ecc04
#define EMAC_RXPOLLD      0x400ecc08
#define EMAC_RXDLADDR     0x400ecc0c
#define EMAC_TXDLADDR     0x400ecc10
#define EMAC_DMARIS       0x400ecc14
#define EMAC_DMAOPMODE    0x400ecc18
#define EMAC_DMAIM        0x400ecc1c
#define EMAC_PC           0x400ecfc4
#define EMAC_CC           0x400ecfc8

#define EMAC_IM_PMT       (1U <<  3U)
#define EMAC_IM_TSI       (1U <<  9U)
#define EMAC_IM_ALLINTS   (EMAC_IM_PMT | EMAC_IM_TSI)

#define EMAC_DMAINT_TI    (1U <<  0U)
#define EMAC_DMAINT_TPSI  (1U <<  1U)
#define EMAC_DMAINT_TBUI  (1U <<  2U)
#define EMAC_DMAINT_TJTI  (1U <<  3U)
#define EMAC_DMAINT_OVFI  (1U <<  4U)
#define EMAC_DMAINT_UNFI  (1U <<  5U)
#define EMAC_DMAINT_RI    (1U <<  6U)
#define EMAC_DMAINT_RBUI  (1U <<  7U)
#define EMAC_DMAINT_RPSI  (1U <<  8U)
#define EMAC_DMAINT_RWTI  (1U <<  9U)
#define EMAC_DMAINT_ETI   (1U << 10U)
#define EMAC_DMAINT_FBEI  (1U << 13U)
#define EMAC_DMAINT_ERI   (1U << 14U)
#define EMAC_DMAINT_AIS   (1U << 15U)
#define EMAC_DMAINT_NIS   (1U << 16U)

#define EMAC_DMAINT_RECV_ENABLE  (EMAC_DMAINT_NIS | EMAC_DMAINT_RI)
#define EMAC_DMAINT_XMIT_ENABLE  (EMAC_DMAINT_NIS | EMAC_DMAINT_TI)
#define EMAC_DMAINT_XMIT_DISABLE EMAC_DMAINT_TI
#define EMAC_DMAINT_ERROR_ENABLE 0U
#define EMAC_DMAINT_NORMAL \
  (EMAC_DMAINT_TI | EMAC_DMAINT_TBUI | EMAC_DMAINT_RI | EMAC_DMAINT_ERI)

#define EMAC_DMABUSMOD_SWR      (1U <<  0U)
#define EMAC_DMABUSMOD_DA       (1U <<  1U)
#define EMAC_DMABUSMOD_ATDS     (1U <<  7U)
#define EMAC_DMABUSMOD_FB       (1U << 16U)
#define EMAC_DMABUSMOD_USP      (1U << 23U)
#define EMAC_DMABUSMOD_8XPBL    (1U << 24U)
#define EMAC_DMABUSMOD_AAL      (1U << 25U)
#define EMAC_DMABUSMOD_MB       (1U << 26U)
#define EMAC_DMABUSMOD_TXPR     (1U << 27U)
#define EMAC_DMABUSMOD_RIB      (1U << 31U)

#define EMAC_DMABUSMOD_DSL_MASK (31U << 2U)
#define EMAC_DMABUSMOD_PBL_MASK (0x3fU << 8U)
#define EMAC_DMABUSMOD_PR_MASK  (3U << 14U)
#define EMAC_DMABUSMOD_RPBL_MASK (0x3fU << 17U)

#define EMAC_DMABUSMOD_DSL(n)   (((uint32_t)(n)) << 2U)
#define EMAC_DMABUSMOD_PBL(n)   (((uint32_t)(n)) << 8U)
#define EMAC_DMABUSMOD_RPBL(n)  (((uint32_t)(n)) << 17U)

#define EMAC_DMA_RXBURST 4
#define EMAC_DMA_TXBURST 4

#if (EMAC_DMA_RXBURST > 32) || (EMAC_DMA_TXBURST > 32)
#define __EMAC_DMABUSMOD_8XPBL 0U
#define __EMAC_DMA_RXBURST     EMAC_DMA_RXBURST
#define __EMAC_DMA_TXBURST     EMAC_DMA_TXBURST
#else
#define __EMAC_DMABUSMOD_8XPBL EMAC_DMABUSMOD_8XPBL
#define __EMAC_DMA_RXBURST     (EMAC_DMA_RXBURST >> 3U)
#define __EMAC_DMA_TXBURST     (EMAC_DMA_TXBURST >> 3U)
#endif

#define __EMAC_DMABUSMOD_PBL   (EMAC_DMABUSMOD_PBL(__EMAC_DMA_RXBURST))

#if __EMAC_DMA_RXBURST == __EMAC_DMA_TXBURST
#define __EMAC_DMABUSMOD_USP   0U
#define __EMAC_DMABUSMOD_RPBL  0U
#else
#define __EMAC_DMABUSMOD_USP   EMAC_DMABUSMOD_USP
#define __EMAC_DMABUSMOD_RPBL  EMAC_DMABUSMOD_RPBL(__EMAC_DMA_TXBURST)
#endif

#ifdef EMAC_ENHANCEDDESC
#define __EMAC_DMABUSMOD_ATDS  EMAC_DMABUSMOD_ATDS
#else
#define __EMAC_DMABUSMOD_ATDS  0U
#endif

#define EMAC_PC_PHYHOLD         (1U <<  0U)
#define EMAC_PC_ANEN            (1U <<  3U)
#define EMAC_PC_FASTANEN        (1U <<  6U)
#define EMAC_PC_EXTFD           (1U <<  7U)
#define EMAC_PC_FASTLUPD        (1U <<  8U)
#define EMAC_PC_FASTRXDV        (1U <<  9U)
#define EMAC_PC_MDIXEN          (1U << 10U)
#define EMAC_PC_FASTMDIX        (1U << 11U)
#define EMAC_PC_RBSTMDIX        (1U << 12U)
#define EMAC_PC_MDISWAP         (1U << 13U)
#define EMAC_PC_POLSWAP         (1U << 14U)
#define EMAC_PC_TDRRUN          (1U << 20U)
#define EMAC_PC_LRR             (1U << 21U)
#define EMAC_PC_ISOMIILL        (1U << 22U)
#define EMAC_PC_RXERIDLE        (1U << 23U)
#define EMAC_PC_NIBDETDIS       (1U << 24U)
#define EMAC_PC_DIGRESTART      (1U << 25U)
#define EMAC_PC_PHYEXT          (1U << 31U)

#define EMAC_PC_ANMODE_10HD     0U
#define EMAC_PC_ANMODE_10FD     2U
#define EMAC_PC_ANMODE_100HD    4U
#define EMAC_PC_ANMODE_100FD    6U

#define EMAC_PC_PINTFS_MII      (0U << 28U)
#define EMAC_PC_PINTFS_RMII     (4U << 28U)

#define EMAC_CC_CLKEN     (1U << 16U)
#define EMAC_CC_POL       (1U << 17U)
#define EMAC_CC_PTPCEN    (1U << 18U)

#define EMAC_RDES0_ESA    (1U <<  0U)
#define EMAC_RDES0_CE     (1U <<  1U)
#define EMAC_RDES0_DBE    (1U <<  2U)
#define EMAC_RDES0_RE     (1U <<  3U)
#define EMAC_RDES0_RWT    (1U <<  4U)
#define EMAC_RDES0_FT     (1U <<  5U)
#define EMAC_RDES0_LCO    (1U <<  6U)
#define EMAC_RDES0_TSV    (1U <<  7U)
#define EMAC_RDES0_GIANT  (1U <<  7U)
#define EMAC_RDES0_LS     (1U <<  8U)
#define EMAC_RDES0_FS     (1U <<  9U)
#define EMAC_RDES0_VLAN   (1U << 10U)
#define EMAC_RDES0_OE     (1U << 11U)
#define EMAC_RDES0_LE     (1U << 12U)
#define EMAC_RDES0_SAF    (1U << 13U)
#define EMAC_RDES0_DE     (1U << 14U)
#define EMAC_RDES0_ES     (1U << 15U)
#define EMAC_RDES0_AFM    (1U << 30U)
#define EMAC_RDES0_OWN    (1U << 31U)

#define EMAC_RDES0_FL_SHIFT     16U
#define EMAC_RDES0_FL_MASK      (0x3fffU << EMAC_RDES0_FL_SHIFT)

#define EMAC_RDES1_RCH    (1U << 14U)
#define EMAC_RDES1_RER    (1U << 15U)
#define EMAC_RDES1_DIC    (1U << 31U)

#define EMAC_TDES0_DB     (1U <<  0U)
#define EMAC_TDES0_UF     (1U <<  1U)
#define EMAC_TDES0_ED     (1U <<  2U)
#define EMAC_TDES0_VF     (1U <<  7U)
#define EMAC_TDES0_EC     (1U <<  8U)
#define EMAC_TDES0_LCO    (1U <<  9U)
#define EMAC_TDES0_NC     (1U << 10U)
#define EMAC_TDES0_LCA    (1U << 11U)
#define EMAC_TDES0_IPE    (1U << 12U)
#define EMAC_TDES0_FF     (1U << 13U)
#define EMAC_TDES0_JT     (1U << 14U)
#define EMAC_TDES0_ES     (1U << 15U)
#define EMAC_TDES0_IHE    (1U << 16U)
#define EMAC_TDES0_TTSS   (1U << 17U)
#define EMAC_TDES0_TCH    (1U << 20U)
#define EMAC_TDES0_TER    (1U << 21U)
#define EMAC_TDES0_CRCR   (1U << 24U)
#define EMAC_TDES0_TTSE   (1U << 25U)
#define EMAC_TDES0_DP     (1U << 26U)
#define EMAC_TDES0_DC     (1U << 27U)
#define EMAC_TDES0_FS     (1U << 28U)
#define EMAC_TDES0_LS     (1U << 29U)
#define EMAC_TDES0_IC     (1U << 30U)
#define EMAC_TDES0_OWN    (1U << 31U)

#define EMAC_MIIADDR_MIIB 1U
#define EMAC_MIIADDR_MIIW 2U

#define EMAC_MIIADDR_CR_MASK 60U
#if (SYS_CLOCK >= 20000000) && (SYS_CLOCK < 35000000)
#define EMAC_MIIADDR_CR 8U
#elif (SYS_CLOCK >= 35000000) && (SYS_CLOCK <= 64000000)
#define EMAC_MIIADDR_CR 12U
#elif (SYS_CLOCK >= 60000000) && (SYS_CLOCK <= 104000000)
#define EMAC_MIIADDR_CR 0U
#elif (SYS_CLOCK >= 100000000) && (SYS_CLOCK <= 150000000)
#define EMAC_MIIADDR_CR 4U
#elif (SYS_CLOCK >= 150000000) && (SYS_CLOCK <= 168000000)
#define EMAC_MIIADDR_CR 16U
#else
#error Unsupported SYS_CLOCK.
#endif

#define EMAC_MIIADDR_PLA_SHIFT  11U
#define EMAC_MIIADDR_PLA_MASK   (31U << EMAC_MIIADDR_PLA_SHIFT)
#define EMAC_MIIADDR_MII_SHIFT  6U
#define EMAC_MIIADDR_MII_MASK   (31U << EMAC_MIIADDR_MII_SHIFT)

#define EMAC_CFG_RE       (1U <<  2U)
#define EMAC_CFG_TE       (1U <<  3U)
#define EMAC_CFG_DC       (1U <<  4U)
#define EMAC_CFG_ACS      (1U <<  7U)
#define EMAC_CFG_DR       (1U <<  9U)
#define EMAC_CFG_IPC      (1U << 10U)
#define EMAC_CFG_DUPM     (1U << 11U)
#define EMAC_CFG_LOOPBM   (1U << 12U)
#define EMAC_CFG_DRO      (1U << 13U)
#define EMAC_CFG_FES      (1U << 14U)
#define EMAC_CFG_PS       (1U << 15U)
#define EMAC_CFG_DISCRS   (1U << 16U)
#define EMAC_CFG_JFEN     (1U << 20U)
#define EMAC_CFG_JD       (1U << 22U)
#define EMAC_CFG_WDDIS    (1U << 23U)
#define EMAC_CFG_CST      (1U << 25U)
#define EMAC_CFG_TWOKPEN  (1U << 27U)

#define EMAC_CFG_BL_MASK  (3U <<  5U)

#define EMAC_CFG_BL_10    (0U <<  5U)
#define EMAC_CFG_BL_8     (1U <<  5U)
#define EMAC_CFG_BL_4     (2U <<  5U)
#define EMAC_CFG_BL_1     (3U <<  5U)

#define EMAC_CFG_IFG_MASK (7U << 17U)

#define EMAC_CFG_IFG_96   (0U << 17U)
#define EMAC_CFG_IFG_88   (1U << 17U)
#define EMAC_CFG_IFG_80   (2U << 17U)
#define EMAC_CFG_IFG_72   (3U << 17U)
#define EMAC_CFG_IFG_64   (4U << 17U)
#define EMAC_CFG_IFG_56   (5U << 17U)
#define EMAC_CFG_IFG_48   (6U << 17U)
#define EMAC_CFG_IFG_40   (7U << 17U)

#define MACCR_CLEAR_BITS                                             \
  (EMAC_CFG_RE | EMAC_CFG_TE | EMAC_CFG_DC | EMAC_CFG_BL_MASK |      \
   EMAC_CFG_ACS | EMAC_CFG_DR | EMAC_CFG_IPC | EMAC_CFG_DUPM |       \
   EMAC_CFG_LOOPBM | EMAC_CFG_DRO | EMAC_CFG_FES | EMAC_CFG_DISCRS | \
   EMAC_CFG_IFG_MASK | EMAC_CFG_JD | EMAC_CFG_WDDIS | EMAC_CFG_CST)

#ifdef EMAC_HWCHECKSUM
#define MACCR_SET_BITS \
  (EMAC_CFG_BL_10 | EMAC_CFG_DR | EMAC_CFG_IPC | EMAC_CFG_IFG_96)
#else
#define MACCR_SET_BITS \
  (EMAC_CFG_BL_10 | EMAC_CFG_DR | EMAC_CFG_IFG_96)
#endif

#define EMAC_FRAMEFLTR_PR       (1U <<  0U)
#define EMAC_FRAMEFLTR_HUC      (1U <<  1U)
#define EMAC_FRAMEFLTR_HMC      (1U <<  2U)
#define EMAC_FRAMEFLTR_DAIF     (1U <<  3U)
#define EMAC_FRAMEFLTR_PM       (1U <<  4U)
#define EMAC_FRAMEFLTR_DBF      (1U <<  5U)
#define EMAC_FRAMEFLTR_SAIF     (1U <<  8U)
#define EMAC_FRAMEFLTR_SAF      (1U <<  9U)
#define EMAC_FRAMEFLTR_HPF      (1U << 10U)
#define EMAC_FRAMEFLTR_VTFE     (1U << 16U)
#define EMAC_FRAMEFLTR_RA       (1U << 31U)

#define EMAC_FRAMEFLTR_PCF_MASK     (3U << 6U)

#define EMAC_FRAMEFLTR_PCF_NONE     (0U << 6U)
#define EMAC_FRAMEFLTR_PCF_PAUSE    (1U << 6U)
#define EMAC_FRAMEFLTR_PCF_ALL      (2U << 6U)
#define EMAC_FRAMEFLTR_PCF_FILTER   (3U << 6U)

#define FRAMEFLTR_CLEAR_BITS                                            \
  (EMAC_FRAMEFLTR_PR | EMAC_FRAMEFLTR_HUC | EMAC_FRAMEFLTR_HMC |        \
   EMAC_FRAMEFLTR_DAIF | EMAC_FRAMEFLTR_PM | EMAC_FRAMEFLTR_DBF |       \
   EMAC_FRAMEFLTR_PCF_MASK | EMAC_FRAMEFLTR_SAIF | EMAC_FRAMEFLTR_SAF | \
   EMAC_FRAMEFLTR_HPF | EMAC_FRAMEFLTR_RA)

#define FRAMEFLTR_SET_BITS EMAC_FRAMEFLTR_PCF_PAUSE

#define EMAC_FLOWCTL_FCBBPA     (1U << 0U)
#define EMAC_FLOWCTL_TFE        (1U << 1U)
#define EMAC_FLOWCTL_RFE        (1U << 2U)
#define EMAC_FLOWCTL_UP         (1U << 3U)
#define EMAC_FLOWCTL_DZQP       (1U << 7U)

#define EMAC_FLOWCTL_PLT_MASK   (3U << 4U)

#define EMAC_FLOWCTL_PLT_M4     (0U << 4U)
#define EMAC_FLOWCTL_PLT_M28    (1U << 4U)
#define EMAC_FLOWCTL_PLT_M144   (2U << 4U)
#define EMAC_FLOWCTL_PLT_M256   (3U << 4U)

#define EMAC_FLOWCTL_PT_MASK    (0xffffU << 16U)

#define FLOWCTL_CLEAR_MASK                                       \
  (EMAC_FLOWCTL_FCBBPA | EMAC_FLOWCTL_TFE | EMAC_FLOWCTL_RFE |   \
   EMAC_FLOWCTL_UP | EMAC_FLOWCTL_PLT_MASK | EMAC_FLOWCTL_DZQP | \
   EMAC_FLOWCTL_PT_MASK)

#define FLOWCTL_SET_MASK (EMAC_FLOWCTL_PLT_M4 | EMAC_FLOWCTL_DZQP)

#define EMAC_DMAOPMODE_SR       (1U <<  1U)
#define EMAC_DMAOPMODE_OSF      (1U <<  2U)
#define EMAC_DMAOPMODE_DGF      (1U <<  5U)
#define EMAC_DMAOPMODE_FUF      (1U <<  6U)
#define EMAC_DMAOPMODE_FEF      (1U <<  7U)
#define EMAC_DMAOPMODE_ST       (1U << 13U)
#define EMAC_DMAOPMODE_FTF      (1U << 20U)
#define EMAC_DMAOPMODE_TSF      (1U << 21U)
#define EMAC_DMAOPMODE_DFF      (1U << 24U)
#define EMAC_DMAOPMODE_RSF      (1U << 25U)
#define EMAC_DMAOPMODE_DT       (1U << 26U)

#define EMAC_DMAOPMODE_RTC_MASK (3U << 3U)

#define EMAC_DMAOPMODE_RTC_64   (0U << 3U)
#define EMAC_DMAOPMODE_RTC_32   (1U << 3U)
#define EMAC_DMAOPMODE_RTC_96   (2U << 3U)
#define EMAC_DMAOPMODE_RTC_128  (3U << 3U)

#define EMAC_DMAOPMODE_TTC_MASK (7U << 14U)

#define EMAC_DMAOPMODE_TTC_64   (0U << 14U)
#define EMAC_DMAOPMODE_TTC_128  (1U << 14U)
#define EMAC_DMAOPMODE_TTC_192  (2U << 14U)
#define EMAC_DMAOPMODE_TTC_256  (3U << 14U)
#define EMAC_DMAOPMODE_TTC_40   (4U << 14U)
#define EMAC_DMAOPMODE_TTC_32   (5U << 14U)
#define EMAC_DMAOPMODE_TTC_24   (6U << 14U)
#define EMAC_DMAOPMODE_TTC_16   (7U << 14U)

#define DMAOPMODE_CLEAR_MASK \
  (EMAC_DMAOPMODE_SR | EMAC_DMAOPMODE_OSF | EMAC_DMAOPMODE_RTC_MASK | \
   EMAC_DMAOPMODE_DGF | EMAC_DMAOPMODE_FUF | EMAC_DMAOPMODE_FEF |     \
   EMAC_DMAOPMODE_ST | EMAC_DMAOPMODE_TTC_MASK | EMAC_DMAOPMODE_FTF | \
   EMAC_DMAOPMODE_TSF | EMAC_DMAOPMODE_DFF | EMAC_DMAOPMODE_RSF |     \
   EMAC_DMAOPMODE_DT)

#ifdef EMAC_HWCHECKSUM
#define DMAOPMODE_SET_MASK                                              \
  (EMAC_DMAOPMODE_OSF | EMAC_DMAOPMODE_RTC_64 | EMAC_DMAOPMODE_TTC_64 | \
   EMAC_DMAOPMODE_TSF | EMAC_DMAOPMODE_RSF)
#else
#define DMAOPMODE_SET_MASK                                              \
  (EMAC_DMAOPMODE_OSF | EMAC_DMAOPMODE_RTC_64 | EMAC_DMAOPMODE_TTC_64 | \
   EMAC_DMAOPMODE_DT)
#endif

#define DMABUSMOD_CLEAR_MASK                                                \
  (EMAC_DMABUSMOD_SWR | EMAC_DMABUSMOD_DA | EMAC_DMABUSMOD_DSL_MASK |       \
   EMAC_DMABUSMOD_ATDS | EMAC_DMABUSMOD_PBL_MASK | EMAC_DMABUSMOD_PR_MASK | \
   EMAC_DMABUSMOD_FB | EMAC_DMABUSMOD_RPBL_MASK | EMAC_DMABUSMOD_USP |      \
   EMAC_DMABUSMOD_8XPBL | EMAC_DMABUSMOD_AAL | EMAC_DMABUSMOD_MB |          \
   EMAC_DMABUSMOD_TXPR | EMAC_DMABUSMOD_RIB)

#define DMABUSMOD_SET_MASK                                               \
  (EMAC_DMABUSMOD_DA | EMAC_DMABUSMOD_DSL(0) | __EMAC_DMABUSMOD_ATDS |   \
   __EMAC_DMABUSMOD_PBL | __EMAC_DMABUSMOD_RPBL | __EMAC_DMABUSMOD_USP | \
   __EMAC_DMABUSMOD_8XPBL | EMAC_DMABUSMOD_MB)

#define EPHY_STS_LINK     (1U <<  0U)
#define EPHY_STS_SPEED    (1U <<  1U)
#define EPHY_STS_DUPLEX   (1U <<  2U)
#define EPHY_STS_MIILB    (1U <<  3U)
#define EPHY_STS_ANS      (1U <<  4U)
#define EPHY_STS_JD       (1U <<  5U)
#define EPHY_STS_RF       (1U <<  6U)
#define EPHY_STS_MIIREQ   (1U <<  7U)
#define EPHY_STS_PAGERX   (1U <<  8U)
#define EPHY_STS_DL       (1U <<  9U)
#define EPHY_STS_SD       (1U << 10U)
#define EPHY_STS_FCSL     (1U << 11U)
#define EPHY_STS_POLSTAT  (1U << 12U)
#define EPHY_STS_RXLERR   (1U << 13U)
#define EPHY_STS_MDIXM    (1U << 14U)

#define MII_MCR           0x00U
#define MII_MSR           0x01U
#define MII_PHYID1        0x02U
#define MII_PHYID2        0x03U
#define MII_ADVERTISE     0x04U
#define MII_LPA           0x05U
#define MII_EXPANSION     0x06U
#define MII_NEXTPAGE      0x07U
#define MII_LPANEXTPAGE   0x08U
#define MII_MSCONTROL     0x09U
#define MII_MSSTATUS      0x0aU
#define MII_PSECONTROL    0x0bU
#define MII_PSESTATUS     0x0cU
#define MII_MMDCONTROL    0x0dU
#define MII_ESTATUS       0x0fU

#define MII_MCR_UNIDIR    (1U <<  5U)
#define MII_MCR_SPEED1000 (1U <<  6U)
#define MII_MCR_CTST      (1U <<  7U)
#define MII_MCR_FULLDPLX  (1U <<  8U)
#define MII_MCR_ANRESTART (1U <<  9U)
#define MII_MCR_ISOLATE   (1U << 10U)
#define MII_MCR_PDOWN     (1U << 11U)
#define MII_MCR_ANENABLE  (1U << 12U)
#define MII_MCR_SPEED100  (1U << 13U)
#define MII_MCR_LOOPBACK  (1U << 14U)
#define MII_MCR_RESET     (1U << 15U)

#define MII_MSR_EXTCAP          (1U <<  0U)
#define MII_MSR_JABBERDETECT    (1U <<  1U)
#define MII_MSR_LINKSTATUS      (1U <<  2U)
#define MII_MSR_ANEGABLE        (1U <<  3U)
#define MII_MSR_RFAULT          (1U <<  4U)
#define MII_MSR_ANEGCOMPLETE    (1U <<  5U)
#define MII_MSR_MFRAMESUPPRESS  (1U <<  6U)
#define MII_MSR_UNIDIR          (1U <<  7U)
#define MII_MSR_ESTATEN         (1U <<  8U)
#define MII_MSR_100BASET2FULL   (1U <<  9U)
#define MII_MSR_100BASET2HALF   (1U << 10U)
#define MII_MSR_10BASETXHALF    (1U << 11U)
#define MII_MSR_10BASETXFULL    (1U << 12U)
#define MII_MSR_100BASETXHALF   (1U << 13U)
#define MII_MSR_100BASETXFULL   (1U << 14U)
#define MII_MSR_100BASET4       (1U << 15U)

struct emac_txdesc_s
{
  uint32_t tdes0;
  uint32_t tdes1;
  uint32_t tdes2;
  uint32_t tdes3;
};

struct emac_rxdesc_s
{
  uint32_t rdes0;
  uint32_t rdes1;
  uint32_t rdes2;
  uint32_t rdes3;
};

static uint8_t rxbuffer[EMAC_NRXDESC * OPTIMAL_EMAC_BUFSIZE];
static struct emac_rxdesc_s rxtable[EMAC_NRXDESC];
static struct emac_txdesc_s txtable[EMAC_NTXDESC];
static struct emac_rxdesc_s *rxhead = NULL;
static struct emac_rxdesc_s *rxcurr = NULL;
static struct emac_txdesc_s *txhead = NULL;
static struct emac_txdesc_s *txtail = NULL;
static unsigned char recv_queue_base[EMAC_NFREEBUFFERS];
static unsigned char send_queue_base[EMAC_NFREEBUFFERS];

static struct s_queue recv_q = {
  .q_base = recv_queue_base,
  .q_head = recv_queue_base,
  .q_tail = recv_queue_base,
  .q_size = EMAC_NFREEBUFFERS,
  .q_count = 0,
  .q_wakeup = EMAC_NFREEBUFFERS
};

static struct s_queue send_q = {
  .q_base = send_queue_base,
  .q_head = send_queue_base,
  .q_tail = send_queue_base,
  .q_size = EMAC_NFREEBUFFERS,
  .q_count = 0,
  .q_wakeup = EMAC_NFREEBUFFERS
};

static struct buff_s {
  uint8_t buff[OPTIMAL_EMAC_BUFSIZE];
  size_t len;
  uint_fast8_t used;
} buffers[EMAC_NFREEBUFFERS];

static size_t rxsegments = 0U;
static size_t txinflight = 0U;
static uint8_t mac_addr[6U] = { 0U, 0U, 0U, 0U, 0U, 0U };

static int eth_phyread(uint16_t phydevaddr,
                       uint16_t phyregaddr, uint16_t *value)
{
  volatile uint32_t timeout;
  uint32_t regval;

  regval = inl(EMAC_MIIADDR);
  regval &= EMAC_MIIADDR_CR_MASK;
  regval |= (phydevaddr << EMAC_MIIADDR_PLA_SHIFT) & EMAC_MIIADDR_PLA_MASK;
  regval |= (phyregaddr << EMAC_MIIADDR_MII_SHIFT) & EMAC_MIIADDR_MII_MASK;
  regval |= EMAC_MIIADDR_MIIB;
  outl(EMAC_MIIADDR, regval);
  for (timeout = 0U; timeout < PHY_READ_TIMEOUT; timeout++) {
    if (!((inl(EMAC_MIIADDR)) & EMAC_MIIADDR_MIIB)) {
      *value = ((uint16_t)(inl(EMAC_MIIDATA)));
      return 0;
    }
  }
  return -ETIMEDOUT;
}

static int eth_phywrite(uint16_t phydevaddr,
                        uint16_t phyregaddr, uint16_t value)
{
  volatile uint32_t timeout;
  uint32_t regval;

  regval = inl(EMAC_MIIADDR);
  regval &= EMAC_MIIADDR_CR_MASK;
  regval |= (phydevaddr << EMAC_MIIADDR_PLA_SHIFT) & EMAC_MIIADDR_PLA_MASK;
  regval |= (phyregaddr << EMAC_MIIADDR_MII_SHIFT) & EMAC_MIIADDR_MII_MASK;
  regval |= (EMAC_MIIADDR_MIIB | EMAC_MIIADDR_MIIW);
  outl(EMAC_MIIDATA, value);
  outl(EMAC_MIIADDR, regval);
  for (timeout = 0U; timeout < PHY_WRITE_TIMEOUT; timeout++) {
    if (!((inl(EMAC_MIIADDR)) & EMAC_MIIADDR_MIIB))
      return 0;
  }
  return -ETIMEDOUT;
}

static void eth_initbuffers(void)
{
  unsigned u;

  for (u = 0U; u < EMAC_NFREEBUFFERS; u++)
    buffers[u].used = 0U;
}

static uint8_t *eth_allocbuffer(size_t len)
{
  unsigned u;

  for (u = 0U; u < EMAC_NFREEBUFFERS; u++) {
    if (!(buffers[u].used)) {
      buffers[u].len = len;
      buffers[u].used = (u + 1U);
      return buffers[u].buff;
    }
  }
  return NULL;
}

static void eth_freebuffer(uint8_t *buffer)
{
  struct buff_s *buf = ((struct buff_s *)
                        (buffer - offsetof(struct buff_s, buff)));

  buf->len = 0U;
  buf->used = 0U;
}

static uint8_t *eth_getbuffer(uint_fast8_t tok)
{
  return (tok == (buffers[tok - 1U].used)) ? buffers[tok - 1U].buff : NULL;
}

static uint_fast8_t eth_usedbuffer(const uint8_t *buffer)
{
  struct buff_s *buf = ((struct buff_s *)
                        (buffer - offsetof(struct buff_s, buff)));

  return buf->used;
}

static size_t eth_sizebuffer(const uint8_t *buffer)
{
  struct buff_s *buf = ((struct buff_s *)
                        (buffer - offsetof(struct buff_s, buff)));

  return buf->len;
}

static void eth_freesegment(struct emac_rxdesc_s *rxfirst, size_t segments)
{
  struct emac_rxdesc_s *rxdesc;
  unsigned u;

  rxdesc = rxfirst;
  for (u = 0U; u < segments; u++) {
    rxdesc->rdes0 = EMAC_RDES0_OWN;
    rxdesc = ((struct emac_rxdesc_s *)(rxdesc->rdes3));
  }
  rxcurr = NULL;
  rxsegments = 0U;
  if (inl(EMAC_DMARIS) & EMAC_DMAINT_RBUI) {
    outl(EMAC_DMARIS, EMAC_DMAINT_RBUI);
    outl(EMAC_RXPOLLD, 0U);
  }
}

static void eth_enableint(uint32_t ierbit)
{
  uint32_t regval;

  regval = inl(EMAC_DMAIM);
  regval |= (EMAC_DMAINT_NIS | ierbit);
  outl(EMAC_DMAIM, regval);
}

static void eth_disableint(uint32_t ierbit)
{
  uint32_t regval;

  regval = inl(EMAC_DMAIM);
  regval &= (~ierbit);
  if (!(regval & EMAC_DMAINT_NORMAL))
    regval &= (~EMAC_DMAINT_NIS);
  outl(EMAC_DMAIM, regval);
}

static void eth_transmit(void)
{
  irqflags_t fl;
  const void *buffer;
  size_t pkt_len;
  struct emac_txdesc_s *txdesc = txhead;
  struct emac_txdesc_s *txfirst = txdesc;
  uint_fast8_t tok = 0U;

  if (!txdesc) {
    kprintf("No TX descriptor available!\n");
    return;
  }
  fl = __hard_di();
  if (!(txdesc->tdes2)) {
    if (remq(&send_q, &tok)) {
      buffer = eth_getbuffer(tok);
      if (!buffer) {
        __hard_irqrestore(fl);
        kprintf("Transmit buffer not found!\n");
        return;
      }
      pkt_len = eth_sizebuffer(buffer);
      if (pkt_len > OPTIMAL_EMAC_BUFSIZE) {
        __hard_irqrestore(fl);
        kprintf("Invalid transmit buffer size: %d\n", ((int)(pkt_len)));
        return;
      }
      txdesc->tdes0 |= (EMAC_TDES0_FS | EMAC_TDES0_LS | EMAC_TDES0_IC);
      txdesc->tdes1 = pkt_len;
      txdesc->tdes2 = ((uint32_t)(buffer));
      txdesc->tdes0 |= EMAC_TDES0_OWN;
      txdesc = ((struct emac_txdesc_s *)(txdesc->tdes3));
      txhead = txdesc;
      if (!txtail)
        txtail = txfirst;
      txinflight++;
      if (txinflight >= EMAC_NTXDESC)
        eth_disableint(EMAC_DMAINT_RI);
      if ((inl(EMAC_DMARIS)) & EMAC_DMAINT_TBUI) {
        outl(EMAC_DMARIS, EMAC_DMAINT_TBUI);
        outl(EMAC_TXPOLLD, 0U);
      }
      eth_enableint(EMAC_DMAINT_TI);
    }
  }
  __hard_irqrestore(fl);
}

static bool eth_recvframe(void)
{
  struct emac_rxdesc_s *rxdesc;
  struct emac_rxdesc_s *rxcur;
  uint8_t *buffer;
  unsigned u;
  size_t pkt_len;

  rxdesc = rxhead;
  for (u = 0U;
       ((!(rxdesc->rdes0 & EMAC_RDES0_OWN)) &&
        (u < EMAC_NRXDESC) &&
        (txinflight < EMAC_NTXDESC));
       u++) {
    if ((rxdesc->rdes0 & EMAC_RDES0_FS) && (!(rxdesc->rdes0 & EMAC_RDES0_LS))) {
      rxcurr = rxdesc;
      rxsegments = 1U;
    } else if ((!(rxdesc->rdes0 & EMAC_RDES0_LS)) &&
               (!(rxdesc->rdes0 & EMAC_RDES0_FS))) {
      rxsegments++;
    } else {
      rxsegments++;
      rxcur = (1U == rxsegments) ? rxdesc : rxcurr;
      if (!(rxdesc->rdes0 & EMAC_RDES0_ES)) {
        pkt_len = ((rxdesc->rdes0 & EMAC_RDES0_FL_MASK) >> EMAC_RDES0_FL_SHIFT);
        if (pkt_len <= OPTIMAL_EMAC_BUFSIZE) {
          buffer = eth_allocbuffer(pkt_len);
          if (!buffer)
            return false;
          if (!insq(&recv_q, eth_usedbuffer(buffer))) {
            eth_freebuffer(buffer);
            return false;
          }
          memcpy(buffer, ((const void *)(rxcur->rdes2)), pkt_len);
        }
        rxhead = ((struct emac_rxdesc_s *)(rxdesc->rdes3));
        eth_freesegment(rxcur, rxsegments);
        return true;
      } else {
        eth_freesegment(rxcur, rxsegments);
      }
    }
    rxdesc = ((struct emac_rxdesc_s *)(rxdesc->rdes3));
  }
  rxhead = rxdesc;
  return false;
}

static __attribute__((noinline)) void eth_mdelay(unsigned ms)
{
  volatile unsigned i, j;

  for (i = 0U; i < ms; i++) {
    for (j = 0U; j < BOARD_LOOPSPERMSEC; j++)
      asm volatile("");
  }
}

static void eth_freeframe(struct emac_txdesc_s *txdesc)
{
  if (txdesc) {
    if (!txinflight)
      kprintf("Nothing in flight!\n");
    while (!(txdesc->tdes0 & EMAC_TDES0_OWN)) {
      if (txdesc->tdes0 & EMAC_TDES0_FS)
        eth_freebuffer((uint8_t *)(txdesc->tdes2));
      txdesc->tdes2 = 0U;
      if (txdesc->tdes0 & EMAC_TDES0_LS) {
        if (txinflight)
          txinflight--;
        eth_enableint(EMAC_DMAINT_RI);
        if (!txinflight) {
          txtail = NULL;
          return;
        }
      }
      txdesc = ((struct emac_txdesc_s *)(txdesc->tdes3));
    }
    txtail = txdesc;
  }
}

static irqreturn_t eth_isr(unsigned int irq, void *dev_id, uint32_t *regs)
{
  uint32_t regval;

  regval = inl(EMAC_DMARIS);
  if (regval) {
    disable_irq(IRQ_ETHCON);
    regval = inl(EMAC_DMARIS);
    regval &= inl(EMAC_DMAIM);
    if (regval & EMAC_DMAINT_NIS) {
      if (regval & EMAC_DMAINT_RI) {
        outl(EMAC_DMARIS, EMAC_DMAINT_RI);
        while (eth_recvframe()) { }
      }
      if (regval & EMAC_DMAINT_TI) {
        outl(EMAC_DMARIS, EMAC_DMAINT_TI);
        eth_freeframe(txtail);
        if (!txinflight)
          eth_disableint(EMAC_DMAINT_TI);
      }
      outl(EMAC_DMARIS, EMAC_DMAINT_NIS);
    }
    enable_irq(IRQ_ETHCON);
  }
  return IRQ_HANDLED;
}

static __attribute__((noinline)) void eth_reset(void)
{
  uint32_t regval;

  regval  = inl(EMAC_DMABUSMOD);
  regval |= EMAC_DMABUSMOD_SWR;
  outl(EMAC_DMABUSMOD, regval);
  while ((inl(EMAC_DMABUSMOD)) & EMAC_DMABUSMOD_SWR) { }
  eth_mdelay(1U);
  regval = EMAC_PC_MDIXEN | EMAC_PC_ANMODE_100FD | EMAC_PC_ANEN |
           EMAC_PC_PINTFS_MII;
  outl(EMAC_PC, regval);
  regval = inl(SYSCON_SREPHY);
  regval |= SREPHY_R0;
  outl(SYSCON_SREPHY, regval);
  regval &= ~SREPHY_R0;
  outl(SYSCON_SREPHY, regval);
  while (!ephy_periphrdy()) { }
  eth_mdelay(1U);
  outl(EMAC_MMCRXIM, 0xffffffffU);
  outl(EMAC_MMCTXIM, 0xffffffffU);
  regval = inl(EMAC_CC);
  regval &= ~EMAC_CC_CLKEN;
  outl(EMAC_CC, regval);
}

static __attribute__((noinline)) void eth_ifup(void)
{
  uint16_t phyval;
  uint32_t regval;
  unsigned u;
  volatile uint32_t timeout;

  eth_reset();
  regval = inl(EMAC_MIIADDR);
  regval &= ~EMAC_MIIADDR_CR_MASK;
  regval |= EMAC_MIIADDR_CR;
  outl(EMAC_MIIADDR, regval);
  if ((eth_phywrite(INTERNAL_PHY_PHYADDR, MII_MCR, MII_MCR_RESET)) < 0) {
    for (;;)
      asm("wfe");
  }
  eth_mdelay(PHY_RESET_DELAY);
  for (timeout = 0U; timeout < PHY_RETRY_TIMEOUT; timeout++) {
    phyval = 0U;
    if ((eth_phyread(INTERNAL_PHY_PHYADDR, MII_MSR, &phyval)) < 0) {
      for (;;)
        asm("wfe");
    }
    if (phyval & MII_MSR_LINKSTATUS)
      break;
  }
  if (timeout >= PHY_RETRY_TIMEOUT) {
    for (;;)
      asm("wfe");
  }
  if ((eth_phywrite(INTERNAL_PHY_PHYADDR, MII_MCR, MII_MCR_ANENABLE)) < 0) {
    for (;;)
      asm("wfe");
  }
  for (timeout = 0U; timeout < PHY_RETRY_TIMEOUT; timeout++) {
    if ((eth_phyread(INTERNAL_PHY_PHYADDR, MII_MSR, &phyval)) < 0) {
      for (;;)
        asm("wfe");
    }
    if (phyval & MII_MSR_ANEGCOMPLETE)
      break;
  }
  if (timeout >= PHY_RETRY_TIMEOUT) {
    for (;;)
      asm("wfe");
  }
  if ((eth_phyread(INTERNAL_PHY_PHYADDR, EPHY_STS, &phyval)) < 0) {
    for (;;)
      asm("wfe");
  }
  regval = inl(EMAC_CFG);
  regval &= ~MACCR_CLEAR_BITS;
  regval |= MACCR_SET_BITS;
  if (EPHY_STS_DUPLEX == (phyval & EPHY_STS_DUPLEX))
    regval |= EMAC_CFG_DUPM;
  if (!(phyval & EPHY_STS_SPEED))
    regval |= EMAC_CFG_FES;
  outl(EMAC_CFG, regval);
  regval = inl(EMAC_FRAMEFLTR);
  regval &= ~FRAMEFLTR_CLEAR_BITS;
  regval |= FRAMEFLTR_SET_BITS;
  outl(EMAC_FRAMEFLTR, regval);
  outl(EMAC_HASHTBLH, 0U);
  outl(EMAC_HASHTBLL, 0U);
  regval = inl(EMAC_FLOWCTL);
  regval &= ~FLOWCTL_CLEAR_MASK;
  regval |= FLOWCTL_SET_MASK;
  outl(EMAC_FLOWCTL, regval);
  outl(EMAC_VLANTG, 0U);
  regval = inl(EMAC_DMAOPMODE);
  regval &= ~DMAOPMODE_CLEAR_MASK;
  regval |= DMAOPMODE_SET_MASK;
  outl(EMAC_DMAOPMODE, regval);
  regval = inl(EMAC_DMABUSMOD);
  regval &= ~DMABUSMOD_CLEAR_MASK;
  regval |= DMABUSMOD_SET_MASK;
  outl(EMAC_DMABUSMOD, regval);
  eth_initbuffers();
  txhead = txtable;
  txtail = NULL;
  txinflight = 0U;
  for (u = 0U; u < EMAC_NTXDESC; u++) {
    txtable[u].tdes0 = EMAC_TDES0_TCH;
    txtable[u].tdes1 = 0U;
    txtable[u].tdes2 = 0U;
    txtable[u].tdes3 = ((u < (EMAC_NTXDESC - 1U)) ?
                         ((uint32_t)(&txtable[u + 1U])) :
                         ((uint32_t)(txtable)));
  }
  outl(EMAC_TXDLADDR, ((uint32_t)(txtable)));
  rxhead = rxtable;
  rxcurr = NULL;
  rxsegments = 0U;
  for (u = 0U; u < EMAC_NRXDESC; u++) {
    rxtable[u].rdes0 = EMAC_RDES0_OWN;
    rxtable[u].rdes1 = (EMAC_RDES1_RCH | ((uint32_t)(OPTIMAL_EMAC_BUFSIZE)));
    rxtable[u].rdes2 = ((uint32_t)(&rxbuffer[u * OPTIMAL_EMAC_BUFSIZE]));
    rxtable[u].rdes3 = ((u < (EMAC_NRXDESC - 1U)) ?
                         ((uint32_t)(&rxtable[u + 1U])) :
                         ((uint32_t)(rxtable)));
  }
  outl(EMAC_RXDLADDR, ((uint32_t)(rxtable)));
  regval = ((((uint32_t)(mac_addr[5])) <<  8U) |
             ((uint32_t)(mac_addr[4])));
  outl(EMAC_ADDR0H, regval);
  regval = ((((uint32_t)(mac_addr[3])) << 24U) |
            (((uint32_t)(mac_addr[2])) << 16U) |
            (((uint32_t)(mac_addr[1])) <<  8U) |
             ((uint32_t)(mac_addr[0])));
  outl(EMAC_ADDR0L, regval);
  regval = inl(EMAC_CFG);
  regval |= EMAC_CFG_TE;
  outl(EMAC_CFG, regval);
  regval = inl(EMAC_DMAOPMODE);
  regval |= EMAC_DMAOPMODE_FTF;
  outl(EMAC_DMAOPMODE, regval);
  regval = inl(EMAC_CFG);
  regval |= EMAC_CFG_RE;
  outl(EMAC_CFG, regval);
  regval = inl(EMAC_DMAOPMODE);
  regval |= EMAC_DMAOPMODE_ST;
  outl(EMAC_DMAOPMODE, regval);
  regval = inl(EMAC_DMAOPMODE);
  regval |= EMAC_DMAOPMODE_SR;
  outl(EMAC_DMAOPMODE, regval);
  outl(EMAC_IM, EMAC_IM_ALLINTS);
  outl(EMAC_DMAIM, EMAC_DMAINT_RECV_ENABLE | EMAC_DMAINT_ERROR_ENABLE);
}

static void eth_init(void)
{
  tm4c129x_modreg(SYSCON_PCEMAC, 0U, PCEMAC_P0);
  tm4c129x_modreg(SYSCON_RCGCEMAC, 0U, RCGCEMAC_R0);
  while (!emac_periphrdy()) { }
  eth_mdelay(1U);
  tm4c129x_modreg(SYSCON_RCGCEPHY, 0U, RCGCEPHY_R0);
  while (!ephy_periphrdy()) { }
  eth_mdelay(1U);
  tm4c129x_modreg(SYSCON_PCEPHY, 0U, PCEPHY_P0);
  while (!ephy_periphrdy()) { }
  eth_mdelay(1U);
  gpio_setup_pin(GPIO_PORT('K'), GPIO_PINFUN_PFIO, 4U,
                 GPIO_PAD_STD, 0U, 5U, 0U);
  gpio_setup_pin(GPIO_PORT('K'), GPIO_PINFUN_PFIO, 6U,
                 GPIO_PAD_STD, 0U, 5U, 0U);
  gpio_setup_pin(GPIO_PORT('F'), GPIO_PINFUN_PFIO, 1U,
                 GPIO_PAD_STD, 0U, 5U, 0U);
}

int eth_open(uint_fast8_t minor, uint16_t flag)
{
  if (minor) {
    udata.u_error = ENXIO;
    return -1;
  }
  return 0;
}

int eth_close(uint_fast8_t minor)
{
  if (minor) {
    udata.u_error = ENXIO;
    return -1;
  }
  return 0;
}

int eth_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
  irqflags_t fl;
  void *buffer;
  size_t pkt_len;
  bool anything;
  uint_fast8_t tok = 0U;

  if (minor) {
    udata.u_error = ENXIO;
    return -1;
  }
  eth_transmit();
  if (udata.u_done > udata.u_count) {
    udata.u_error = EINVAL;
    return -1;
  }
  fl = __hard_di();
  anything = remq(&recv_q, &tok);
  __hard_irqrestore(fl);
  if (anything) {
    buffer = eth_getbuffer(tok);
    if (!buffer) {
      udata.u_error = EFAULT;
      return -1;
    }
    pkt_len = eth_sizebuffer(buffer);
    if (pkt_len > (udata.u_count - udata.u_done)) {
      udata.u_error = ERANGE;
      return -1;
    }
    if (uput(buffer, udata.u_base, pkt_len)) {
      eth_freebuffer(buffer);
      udata.u_error = EIO;
      return -1;
    }
    eth_freebuffer(buffer);
    udata.u_done += pkt_len;
    udata.u_base += pkt_len;
  }
  return udata.u_done;
}

int eth_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)
{
  uint8_t *buffer;
  size_t pkt_len;

  if (minor) {
    udata.u_error = ENXIO;
    return -1;
  }
  eth_transmit();
  if (udata.u_done > udata.u_count) {
    udata.u_error = EINVAL;
    return -1;
  }
  if ((udata.u_count - udata.u_done) > OPTIMAL_EMAC_BUFSIZE) {
    udata.u_error = EMSGSIZE;
    return -1;
  }
  pkt_len = (udata.u_count - udata.u_done);
  buffer = eth_allocbuffer(pkt_len);
  if (!buffer) {
    udata.u_error = ENOMEM;
    return -1;
  }
  if (uget(udata.u_base, buffer, pkt_len)) {
    eth_freebuffer(buffer);
    udata.u_error = EIO;
    return -1;
  }
  if (!insq(&send_q, eth_usedbuffer(buffer))) {
    eth_freebuffer(buffer);
    udata.u_error = EAGAIN;
    return -1;
  }
  udata.u_done += pkt_len;
  udata.u_base += pkt_len;
  eth_transmit();
  return udata.u_done;
}

int eth_ioctl(uint_fast8_t minor, uarg_t request, char *data)
{
  char mac[sizeof mac_addr];

  if (minor) {
    udata.u_error = ENXIO;
    return -1;
  }
  if (uget(data, mac, sizeof mac))
    return -1;
  switch (request) {
  case 0:
    memcpy(mac, mac_addr, sizeof mac);
    break;
  default:
    udata.u_error = EINVAL;
    return -1;
  }
  return uput(mac, data, sizeof mac);
}

void ethdev_init(void)
{
  uint32_t user0;
  uint32_t user1;
  irqflags_t fl = __hard_di();

  user0 = inl(FLASH_USERREG0);
  user1 = inl(FLASH_USERREG1);
  mac_addr[0] = ((user0 >>  0U) & 0xffU);
  mac_addr[1] = ((user0 >>  8U) & 0xffU);
  mac_addr[2] = ((user0 >> 16U) & 0xffU);
  mac_addr[3] = ((user1 >>  0U) & 0xffU);
  mac_addr[4] = ((user1 >>  8U) & 0xffU);
  mac_addr[5] = ((user1 >> 16U) & 0xffU);
  eth_init();
  while ((inl(EMAC_DMABUSMOD)) & EMAC_DMABUSMOD_SWR) { }
  eth_mdelay(1U);
  eth_ifup();
  request_irq(IRQ_ETHCON, eth_isr, NULL);
  __hard_irqrestore(fl);
}
