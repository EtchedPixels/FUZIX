#include <kernel.h>
#include <kdata.h>
#include <printf.h>


int uget(const void* user, void* dest, usize_t count)
{
	memcpy(dest, user, count);
	return 0;
}

int uput(const void* source, void* user, usize_t count)
{
	memcpy(user, source, count);
	return 0;
}

int16_t ugetc(const void* user)
{
	return *(uint8_t*)user;
}

uint16_t ugetw(const void* user)
{
	uint16_t tmp;
	uget(user, &tmp, 2);
	return tmp;
}

uint32_t ugetl(void* user, int* err)
{
	uint32_t tmp;
	uget(user, &tmp, 4);
	err = 0;
	return tmp;
}

int uputc(uint16_t value, void* user)
{
	*(uint8_t*)user = value;
	return 0;
}

int uputw(uint16_t value, void* user)
{
	memcpy(user, &value, 2);
	return 0;
}

int uputl(uint32_t value, void* user)
{
	memcpy(user, &value, 4);
	return 0;
}

int uzero(void* user, usize_t count)
{
	memset(user, 0, count);
	return 0;
}

