/* CH375 Commands */
#define CMD_RESET_ALL           0x05
#define CMD_SET_USB_MODE        0x15
#define CMD_GET_STATUS          0x22
#define CMD_RD_USB_DATA         0x28
#define CMD_WR_USB_DATA         0x2B
#define CMD_DISK_INIT           0x51
#define CMD_DISK_SIZE           0x53
#define CMD_DISK_READ           0x54
#define CMD_DISK_RD_GO          0x55
#define CMD_DISK_WRITE          0x56
#define CMD_DISK_WR_GO          0x57
#define CMD_DISK_READY          0x59

/* CH375 Status Results */
#define USB_INT_SUCCESS         0x14
#define USB_INT_CONNECT         0x15
#define USB_INT_DISCONNECT      0x16
#define USB_INT_DISK_READ       0x1D
#define USB_INT_DISK_WRITE      0x1E
