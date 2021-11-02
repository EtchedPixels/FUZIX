#include <kernel.h>

/*
 * Memory alignment friendly user copiers. Unfortunately gcc doesn't
 * appear capable of just doing the right thing when told to always inline
 * these so we can't just inline them as we'd like.
 */

int16_t _ugetc(const uint8_t *p) 
{
    return *p;
}

uint16_t _ugetw(const uint16_t *pv)
{
	const uint8_t *p = (const uint8_t *)pv;
	return (*p << 8) | p[1];
}

uint32_t _ugetl(const uint32_t *pv)
{
	const uint8_t *p = (const uint8_t *)pv;
	return (*p << 24) | (p[1] << 16) | (p[2] << 8) | p [3];
}

int _uputc(uint16_t v, uint8_t *p)
{
	*p = v;
	return 0;
}

int _uputw(uint16_t v, uint16_t *pv)
{
	uint8_t *p = (uint8_t *)pv;
	*p++ = v >> 8;
	*p = v;
	return 0;
}

int _uputl(uint32_t v, uint32_t *pv)
{
	uint8_t *p = (uint8_t *)pv;
	*p++ = v >> 24;
	*p++ = v >> 16;
	*p++ = v >> 8;
	*p = v;
	return 0;
}

int _uzero(uint8_t *p, usize_t len)
{
	memset(p, 0, len);
	return 0;
}

int _uget(const uint8_t *user, uint8_t *kernel, usize_t count)
{
	memcpy(kernel, user, count);
	return 0;
}

int _uput(const uint8_t *kernel, uint8_t *user, usize_t count)
{
	memcpy(user, kernel, count);
	return 0;
}
