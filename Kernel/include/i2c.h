#ifndef _I2C_H
#define _I2C_H

#ifdef CONFIG_DEV_I2C

#include <platform_i2c.h>

#define I2C_WR		1
#define I2C_RD		0

struct i2c_msg {
    uint8_t bus;		/* So we can have multiple i2c busses */
    uint8_t addr;		/* Including r/w bit */
    uint8_t len;
    uint8_t *data;
};

#define I2C_MSG		0x0540

/*
 *	User device interface
 */
extern int devi2c_ioctl(uarg_t request, char *data);
/*
 *	Platform provided
 */
extern int plt_i2c_msg(struct i2c_msg *msg, uint8_t *kbuf);
#endif
#endif
