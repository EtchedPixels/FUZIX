/*
 *	We just glue the PCF8584 to the system
 */

#ifdef CONFIG_RC2014_EXTREME
__sfr __banked __at 0x06B8 i2c_s0;
__sfr __banked __at 0x07B8 i2c_s1;
#else
__sfr __at 0x06 i2c_s0;
__sfr __at 0x07 i2c_s1;
#endif
