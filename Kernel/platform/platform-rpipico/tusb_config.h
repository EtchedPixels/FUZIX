#ifndef TUSB_CONFIG_H
#define TUSB_CONFIG_H
#include "stdint.h"
#include "config.h"
#define CFG_TUSB_RHPORT0_MODE   (OPT_MODE_DEVICE)

#define CFG_TUD_CDC             (NUM_DEV_TTY_USB)
#define CFG_TUD_CDC_RX_BUFSIZE  (64)
#define CFG_TUD_CDC_TX_BUFSIZE  (64)

#endif

