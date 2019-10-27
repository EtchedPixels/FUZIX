extern uint8_t i2c_busy;

/* Bit numbers for i2c bitbang to match our port bits for simplicity */
#define I2C_SCL		0x01
#define I2C_RX_SCL	0x01	/* Dummy - not actually readable */
#define I2C_SDA		0x80
#define I2C_RX_SDA	0x80
