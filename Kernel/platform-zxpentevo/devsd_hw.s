            .module devsd_hw

            ; exported symbols

SDCTL	.equ 0xFF57
SDDATA	.equ 0x7F57


            .area _CODE
_sd_probe:
            ret
