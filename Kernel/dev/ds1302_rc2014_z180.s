	.area _CODE

PIN_CE          = 0x10
PIN_DATA_HIZ    = 0x20
PIN_CLK         = 0x40
PIN_DATA_OUT    = 0x80
PIN_DATA_IN     = 0x01

PIN_OTHER	= 0x0C		; Force SPI selects high during RTC
				; so we don't drive both at once in error


	.include "ds1302_common.s"
