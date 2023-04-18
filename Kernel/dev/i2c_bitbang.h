#ifdef CONFIG_I2C_BITBANG

/*
 *	I2C support
 */

extern int i2c_bb_receive(uint_fast8_t bus, uint_fast8_t dev, uint8_t *buf, unsigned int len);
extern int i2c_bb_send(uint_fast8_t bus, uint_fast8_t dev, uint8_t *buf, unsigned int len);

/*
 *	Helpers provided by the platform if using the bitbang library
 *	for the above (except claim/release).
 */

extern void i2c_set(uint_fast8_t bus, uint_fast8_t bits);
extern void i2c_clear(uint_fast8_t bus, uint_fast8_t bits);
extern uint_fast8_t i2c_sda(uint_fast8_t bus);
extern uint_fast8_t i2c_scl(uint_fast8_t bus);

extern int i2c_claim_bus(uint_fast8_t bus);
extern void i2c_release_bus(uint_fast8_t bus);

#endif
