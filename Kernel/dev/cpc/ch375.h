#ifndef _CH375_H
#define _CH375_H

#include "plt_ch375.h"

#define CH375_CMD_GET_IC_VER            0x01
#define CH375_CMD_RESET_ALL             0x05
#define CH375_CMD_CHECK_EXIST           0x06
#define CH375_CMD_SET_USB_MODE          0x15
#define CH375_CMD_GET_STATUS            0x22
#define CH376_CMD_RD_USB_DATA           0x27
#define CH375_CMD_RD_USB_DATA           0x28
#define CH375_CMD_WR_USB_DATA7          0x2B
#define CH376_CMD_WR_HOST_DATA          0x2C
#define CH375_CMD_DISK_INIT             0x51
#define CH375_CMD_DISK_READ             0x54
#define CH375_CMD_DISK_RD_GO            0x55
#define CH375_CMD_DISK_WRITE            0x56
#define CH375_CMD_DISK_WR_GO            0x57

#define CH375_USB_INT_SUCCESS           0x14
#define CH375_USB_INT_CONNECT           0x15
#define CH375_USB_INT_DISK_READ         0x1D
#define CH375_USB_INT_DISK_WRITE        0x1E

/* TODO: 2 us delay should be implemented by platform */

#ifndef nap2()
#define nap2() nap20()
#endif

uint_fast8_t ch375_probe(void);
uint8_t ch375_rpoll(void);
int ch375_xfer(uint_fast8_t dev, bool is_read, uint32_t lba, uint8_t *dptr);

#endif