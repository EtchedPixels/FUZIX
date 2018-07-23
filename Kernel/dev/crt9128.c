/* SMC CRT9128 Video Terminal Logic Controller driver */
/* 2016-10-29 Tormod Volden */

#include <kernel.h>
#include "crt9128.h"
#include <vt.h>

/* constants for now, otherwise must be shadowed */
static const uint8_t tosadd_tim = 0; /* CRT9128_TOSADD_TIM for 50 Hz */
static const uint8_t curhi_sle = 0; /* non-scrolling status line enable */

/* these are mask-programmed in the chip */
static const uint8_t data_row_length = 80;
static const uint8_t display_lines = 24;
static const uint8_t vram_lines = 25;

/* keep track of current scan-out offset */
static uint8_t vram_start_line;
/* shadow register */
static uint8_t attdat;

static void crt9128_write_reg(uint8_t reg, uint8_t value)
{
	*((volatile uint8_t *)CRT9128_ADDRESS_REG) = reg;
	*((volatile uint8_t *)CRT9128_DATA_REG) = value;
}

static uint8_t crt9128_read_char(void)
{
	*((volatile uint8_t *)CRT9128_ADDRESS_REG) = CRT9128_REG_CHARACTER;
	*((volatile uint8_t *)CRT9128_DATA_REG);
	int timeout = 0xA000;
	while (!crt9128_done() && --timeout);
	if (timeout == 0)
		crt9128_write_reg(CRT9128_REG_CHIP_RESET, 0);
	return *((volatile uint8_t *)CRT9128_DATA_REG);
}

static void crt9128_fill(uint16_t end_addr, uint8_t character)
{
	crt9128_write_reg(CRT9128_REG_FILADD, end_addr >> 4);
	int timeout = 0xA000;
	while (!crt9128_done() && --timeout);
	if (timeout == 0)
		crt9128_write_reg(CRT9128_REG_CHIP_RESET, 0);
	crt9128_write_reg(CRT9128_REG_CHARACTER, character);
}

static void crt9128_set_cursor_address(uint16_t addr)
{
	int timeout = 0xA000;
	while (!crt9128_done() && --timeout);
	if (timeout == 0)
		crt9128_write_reg(CRT9128_REG_CHIP_RESET, 0);
	crt9128_write_reg(CRT9128_REG_CURLO, addr & 0xff);
	crt9128_write_reg(CRT9128_REG_CURHI, addr >> 8 | curhi_sle);
}

/* the scan-out offset register is used for fast scrolling */
static void crt9128_set_tos_line(uint8_t vram_line)
{
	crt9128_write_reg(CRT9128_REG_TOSADD, tosadd_tim | vram_line * (data_row_length >> 4));
}

void crt9128_init(void)
{
	vram_start_line = 0;
	attdat = CRT9128_ATTDAT_TAG_REVERSE_VIDEO | CRT9128_ATTDAT_SCREEN;

	crt9128_write_reg(CRT9128_REG_CHIP_RESET, 0);
	crt9128_write_reg(CRT9128_REG_MODE_REGISTER, 0);
	crt9128_write_reg(CRT9128_REG_ATTDAT, attdat);
	crt9128_set_tos_line(vram_start_line);
	crt9128_clear_lines(0, display_lines);
}

/* return cursor address as a bonus */
static uint16_t crt9128_set_cursor(uint8_t newy, uint8_t newx)
{
	uint8_t vram_line;
	uint16_t cur_addr;

	vram_line = vram_start_line + newy;
	if (vram_line >= vram_lines)
		vram_line -= vram_lines;
	cur_addr = vram_line * data_row_length + newx;
	crt9128_set_cursor_address(cur_addr);
	return cur_addr;
}

static void crt9128_blank_hidden_line(void)
{
	uint16_t start_addr;
	uint16_t end_addr;

	end_addr = vram_start_line * data_row_length;
	if (end_addr == 0)
		start_addr = (vram_lines - 1) * data_row_length;
	else
		start_addr = end_addr - data_row_length;
	crt9128_set_cursor_address(start_addr);
	crt9128_fill(end_addr, ' ');
}

/* interface to Kernel/vt.c VT52 emulation */

void crt9128_clear_across(int8_t y, int8_t x, int16_t num)
{
	uint16_t end_addr;

	end_addr = crt9128_set_cursor(y, x) + (num & 0xff);
	crt9128_fill(end_addr, ' ');
}

/* also sets cursor to first cleared line */
void crt9128_clear_lines(int8_t y, int8_t num)
{
	uint16_t end_addr;

	end_addr = crt9128_set_cursor(y, 0) + num * data_row_length;
	if (end_addr >= vram_lines * data_row_length)
		end_addr -= vram_lines * data_row_length;
	crt9128_fill(end_addr, ' ');
}

void crt9128_scroll_up(void)
{
	crt9128_blank_hidden_line();
	if (vram_start_line == vram_lines - 1)
		vram_start_line = 0;
	else
		vram_start_line++;
	crt9128_set_tos_line(vram_start_line);
}

void crt9128_scroll_down(void)
{
	crt9128_blank_hidden_line();
	if (vram_start_line == 0)
		vram_start_line = vram_lines - 1;
	else
		vram_start_line--;
	crt9128_set_tos_line(vram_start_line);
}

void crt9128_plot_char(int8_t y, int8_t x, uint16_t c)
{
	crt9128_set_cursor(y, x);
	crt9128_write_reg(CRT9128_REG_CHARACTER, c & 0x7f);
}

void crt9128_cursor_off(void)
{
	attdat |= CRT9128_ATTDAT_CURSOR_SUPRESS;
	crt9128_write_reg(CRT9128_REG_ATTDAT, attdat);
}

void crt9128_cursor_on(int8_t newy, int8_t newx)
{
	crt9128_set_cursor(newy, newx);

	attdat &= ~CRT9128_ATTDAT_CURSOR_SUPRESS;
	crt9128_write_reg(CRT9128_REG_ATTDAT, attdat);
}

void crt9128_vtattr_notify(void)
{}

void crt9128_video_cmd(uint8_t *ptr)
{}

void crt9128_video_read(uint8_t *ptr)
{}

void crt9128_video_write(uint8_t *ptr)
{}
