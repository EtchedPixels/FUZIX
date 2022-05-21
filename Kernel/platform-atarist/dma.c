#include <kernel.h>
#include <timer.h>
#include <dma.h>

static uint8_t locked;

/*
 *	Set the 24bit DMA address for a transfer. The transfer is not
 *	cache coherent and must be word aigned
 */
void set_dma_addr(uint8_t *ptr)
{
	uint32_t p = (uint32_t) ptr;
	if (p & 1)
		panic("odd dma");
	DMA->addr_low = p;
	DMA->addr_med = p >> 8;
	DMA->addr_high = p >> 16;
}

/*
 *	Report how the DMA did
 */
uint16_t get_dma_status(void)
{
	DMA->control = 0x90;
	return DMA->control;
}

/*
 *	Wait for a DMA devices to complete by watching a GPIO
 */
int dma_wait(uint16_t wait)
{
	timer_t x = set_timer_duration(wait);
	while (!timer_expired(x)) {
		uint8_t status = *(volatile uint8_t *)0xFFFA01;
		if (!(status & 0x20))
			return 0;
		plt_idle();
	}
	return -1;
}

void dma_lock(void)
{
	locked = 1;
}

void dma_unlock(void)
{
	locked = 0;
}

uint8_t dma_is_locked(void)
{
	return locked;
}
