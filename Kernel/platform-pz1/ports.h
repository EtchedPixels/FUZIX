#define port_serial_flags           (*(volatile uint8_t *)0xFE10)
#define port_serial_in              (*(volatile uint8_t *)0xFE11)
#define port_serial_out             (*(volatile uint8_t *)0xFE12)
#define SERIAL_FLAGS_OUT_FULL       128
#define SERIAL_FLAGS_IN_AVAIL       64

#define port_fs_cmd                 (*(volatile uint8_t *)0xFE60)
#define port_fs_prm0                (*(volatile uint8_t *)0xFE61)
#define port_fs_prm1                (*(volatile uint8_t *)0xFE62)
#define port_fs_data                (*(volatile uint8_t *)0xFE63)
#define port_fs_status              (*(volatile uint8_t *)0xFE64)
#define FS_CMD_SELECT               0
#define FS_CMD_SEEK                 1
#define FS_STATUS_OK                0
#define FS_STATUS_NOK               1

#define port_irq_timer_target       (*(volatile uint8_t *)0xFE80)
#define port_irq_timer_reset        (*(volatile uint8_t *)0xFE82)
