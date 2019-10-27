#ifndef _I2C_H
#define _I2C_H

#ifdef CONFIG_DEV_I2C

#include <platform_i2c.h>

#define I2C_WRITE	1
#define I2C_READ	0

/*
 *	I2C support
 */

extern int i2c_receive(uint_fast8_t bus, uint_fast8_t dev, uint8_t *buf, unsigned int len);
extern int i2c_send(uint_fast8_t bus, uint_fast8_t dev, uint8_t *buf, unsigned int len);

/*
 *	Underlying interface that the platform or library supplies if using
 *	the core i2c helpers
 */

extern void i2c_start(uint_fast8_t bus);
extern void i2c_stop(uint_fast8_t bus);
extern uint_fast8_t i2c_read(uint_fast8_t bus);
extern uint_fast8_t i2c_write(uint_fast8_t bus, uint_fast8_t val);
extern int i2c_claim_bus(uint_fast8_t bus);
extern void i2c_release_bus(uint_fast8_t bus);

/*
 *	Helpers provided by the platform if using the bitbang library
 *	for the above (except claim/release).
 */

extern void i2c_set(uint_fast8_t bus, uint_fast8_t bits);
extern void i2c_clear(uint_fast8_t bus, uint_fast8_t bits);
extern uint_fast8_t i2c_sda(uint_fast8_t bus);
extern uint_fast8_t i2c_scl(uint_fast8_t bus);

/*
 *	User device interface
 */
extern int devi2c_write(void);
extern int devi2c_read(void);

#endif
#endif
