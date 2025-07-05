#ifndef CH375_H
#define CH375_H

#include "plt_ch375.h"

#define CH375_CMD_GET_IC_VER            0x01
#define CH375_CMD_SET_BAUDRATE          0x02
#define CH375_CMD_RESET_ALL             0x05
#define CH375_CMD_CHECK_EXIST           0x06
#define CH375_CMD_SET_USB_MODE          0x15
#define CH375_CMD_TEST_CONNECT          0x16
#define CH375_CMD_GET_STATUS            0x22
#define CH375_CMD_DIRTY_BUFFER          0x25
#define CH376_CMD_RD_USB_DATA           0x27
#define CH375_CMD_RD_USB_DATA           0x28
#define CH375_CMD_WR_USB_DATA7          0x2B
#define CH376_CMD_WR_HOST_DATA          0x2C
#define CH375_CMD_DISK_CONNECT          0x30
#define CH375_CMD_DISK_MOUNT            0x31
#define CH375_CMD_CLR_STALL             0x41
#define CH375_CMD_DISK_INIT             0x51
#define CH375_CMD_DISK_RESET            0x52
#define CH375_CMD_DISK_READ             0x54
#define CH375_CMD_DISK_RD_GO            0x55
#define CH375_CMD_DISK_WRITE            0x56
#define CH375_CMD_DISK_WR_GO            0x57
#define CH375_CMD_DISK_R_SENSE          0x5A

#define CH375_USB_INT_SUCCESS           0x14
#define CH375_USB_INT_CONNECT           0x15
#define CH375_USB_INT_DISK_READ         0x1D
#define CH375_USB_INT_DISK_WRITE        0x1E

#ifndef nap2()
#define nap2() nap20()
#endif

extern uint_fast8_t ch375_probe(void);
extern void* td_io_data_reg;
extern uint8_t td_io_data_count;
extern void ch375_wcmd(uint8_t dev, uint8_t cmd);
extern void ch375_wdata(uint8_t dev, uint8_t data);
extern uint8_t ch375_rdata(uint8_t dev);
extern uint8_t ch375_rpoll(uint_fast8_t dev);
extern int ch375_xfer(uint_fast8_t dev, bool is_read, uint32_t lba, uint8_t *dptr);
#endif