#ifndef _V99xx_H_
#define _V99xx_H_

#include <kernel.h>

#define V99xx_NREGS		    0x1B

/* mode registers */
#define V99xx_REG_MODE0		    0x00
#define V99xx_REG_MODE1		    0x01

#define V99xx_REG_MODE1_BL_MASK	    (1 << 6)
#define V99xx_REG_MODE1_IE0	    (1 << 5)

#define V99xx_REG_MODE2		    0x08
#define V99xx_REG_MODE3		    0x09

/* table base address registers */
#define V99xx_REG_PTRN_LAYOUT_BASE  0x02
#define V99xx_REG_COLOR_BASE_H	    0x03
#define V99xx_REG_COLOR_BASE_L	    0x0A
#define V99xx_REG_PATRN_GEN_BASE    0x04
#define V99xx_REG_SPRITE_ATTR_L	    0x05
#define V99xx_REG_SPRITE_ATTR_H	    0x0B
#define V99xx_REG_SPRITE_PATRN	    0x06

/* color registers */
#define V99xx_REG_COLOR1	    0x07
#define V99xx_REG_COLOR2	    0x0C
#define V99xx_REG_BLINK_PERIOD	    0x0D
#define V99xx_REG_COLOR_BURST1	    0x14
#define V99xx_REG_COLOR_BURST2	    0x15
#define V99xx_REG_COLOR_BURST3	    0x16

/* display registers */
#define V99xx_REG_DISP_CENTER	    0x12
#define V99xx_REG_VERT_OFFSET	    0x17
#define V99xx_REG_INT_LINE	    0x13

/* access registers */
#define V99xx_SET_VRAM_PAGE	    0x0E
#define V99xx_SET_STATUS_REG	    0x0F
#define V99xx_SET_PALETTE_REG	    0x10
#define V99xx_SET_CONTROL_REG	    0x11

/* command registers */
#define V99xx_REG_CMD_SRC_XL	    0x20
#define V99xx_REG_CMD_SRC_XH	    0x21
#define V99xx_REG_CMD_SRC_YL	    0x22
#define V99xx_REG_CMD_SRC_YH	    0x23
#define V99xx_REG_CMD_DST_XL	    0x24
#define V99xx_REG_CMD_DST_XH	    0x25
#define V99xx_REG_CMD_DST_YL	    0x26
#define V99xx_REG_CMD_DST_YH	    0x27
#define V99xx_REG_CMD_SIZE_XL	    0x28
#define V99xx_REG_CMD_SIZE_XH	    0x29
#define V99xx_REG_CMD_SIZE_YL	    0x2A
#define V99xx_REG_CMD_SIZE_YH	    0x2B
#define V99xx_REG_CMD_COLOR	    0x2C
#define V99xx_REG_CMD_ARG	    0x2D
#define V99xx_REG_CMD_OP	    0x2E

/* extended registers V9958 */
#define V99xx_REG_MODE4		    0x19
#define V99xx_REG_HOR_OFFSET1	    0x1A
#define V99xx_REG_HOR_OFFSET2	    0x1B

#define MODE_EXTEND_CMD	    0x40

#define V99xx_VRAM_PAGE_SIZE        0x4000

/* vram access flags */
#define REG_VRAM_ADDR_MASK  1
#define REG_VRAM_WRITE_FLAG  0x40
#define REG_VRAM_READ_FLAG  0x3F

/* mode flags */
#define MODE_MASK_MODE0	1

/* M1 and M2 are bits 4 and 3 */
#define MODE_MASK_REG_MODE1(x)	    (((x) & 0x1C) >> 1)
#define MODE_MASK_REG_MODE0(x)	    (((x) & 0x7) << 1)

#define MODE_TEXT1	    0x01
#define MODE_TEXT2	    0x09
#define MODE_G1		    0x00
#define MODE_G2		    0x04
#define MODE_G3		    0x08
#define MODE_G4		    0x0C
#define MODE_G5		    0x10
#define MODE_G6		    0x14
#define MODE_G7		    0x1C

/* commands */
#define CMD_HMMC	    0xF
#define CMD_YMMM	    0xE
#define CMD_HMMM	    0xD
#define CMD_HMMV	    0xC
#define CMD_LMMC	    0xB
#define CMD_LMCM	    0xA
#define CMD_LMMM	    0x9
#define CMD_LMMV	    0x8
#define CMD_LINE	    0x7
#define CMD_SRCH	    0x6
#define CMD_PSET	    0x5
#define CMD_POINT	    0x4
#define CMD_STOP	    0x0

/* comand logic ops */
#define CMD_IMP		    0x0
#define CMD_AND		    0x1
#define CMD_OR		    0x2
#define CMD_XOR		    0x3
#define CMD_NOT		    0x4
#define CMD_TIMP	    0x8
#define CMD_TAND	    0x9
#define CMD_TOR		    0xA
#define CMD_TXOR	    0xB
#define CMD_TNOT	    0xC

void v99xx_write_reg(uint8_t reg, uint8_t val);
void v99xx_set_vram_page(uint8_t page);
void v99xx_write_vram(uint16_t addr, uint8_t val);
void v99xx_memset_vram(uint16_t addr, uint8_t value, uint16_t size);
void v99xx_copy_to_vram(uint16_t vaddr, uint8_t *src, uint16_t size);
void v99xx_copy_from_vram(uint8_t *dst, uint16_t vaddr, uint16_t size);
uint8_t v99xx_read_vram(uint16_t addr);
void v99xx_set_mode(uint8_t mode);
void v99xx_set_color(uint8_t fg, uint8_t bg);
void v99xx_set_blink_color(uint8_t fg, uint8_t bg);
void v99xx_set_blink_period(uint8_t fg, uint8_t bg);

#endif
