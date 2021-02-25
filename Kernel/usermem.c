/*
 *	Implement the usermem services. Can then use either C or asm code for
 *	the block transfers. This code can be banked as part of the kernel if
 *	using the asm helpers but must be in commonmem if not
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#if !defined(CONFIG_LEVEL_0)

/* Flat mode has to use its own valaddr: tidy this */
#if !defined(CONFIG_FLAT) && !defined(CONFIG_VMMU) && !defined(CONFIG_CUSTOM_VALADDR)

/* This checks to see if a user-supplied address is legitimate */
usize_t valaddr(const uint8_t *base, usize_t size)
{
	if (base + size < base)
		size = MAXUSIZE - (usize_t)base + 1;
	if (!base || base < (const uint8_t *)PROGBASE ||
		base > (const uint8_t *)(size_t)udata.u_ptab->p_top)
		size = 0;
	else if (base + size > (const uint8_t *)(size_t)udata.u_ptab->p_top)
		size = (uint8_t *)(size_t)udata.u_ptab->p_top - base;
	if (size == 0)
		udata.u_error = EFAULT;
	return size;
}
#endif


int uget(const void *user, void *dst, usize_t count)
{
	if (!valaddr(user,count))
		return -1;
	return _uget(user,dst,count);
}

int16_t ugetc(const void *user)
{
	if (!valaddr(user, 1))
		return -1;
	return _ugetc(user);
}

uint16_t ugetw(const void *user)
{
	if (!valaddr(user, 2))
		return -1;
#ifdef MISALIGNED
	if (MISALIGNED(user, 2)) }
		ssig(udata.u_proc, SIGBUS);
		return -1;
	}
#endif
	return _ugetw(user);
}

int uput(const void *source,   void *user, usize_t count)
{
	if (!valaddr(user, count))
		return -1;
	return _uput(source,user,count);
}

int uputc(uint16_t value,  void *user)
{
	if (!valaddr(user, 1))
		return -1;
	/* u16_t so we don't get wacky 8bit stack games on SDCC */
	return _uputc(value,user);
}

int uputw(uint16_t value, void *user)
{
	if (!valaddr(user, 2))
		return -1;
#ifdef MISALIGNED
	if (MISALIGNED(user, 2)) }
		ssig(udata.u_proc, SIGBUS);
		return -1;
	}
#endif
	return _uputw(value,user);
}

int uzero(void *user, usize_t count)
{
	if (!valaddr(user, count))
		return -1;
	return _uzero(user,count);
}

#ifdef CONFIG_32BIT

uint32_t ugetl(void *uaddr, int *err)
{
	if (!valaddr(uaddr, 4)) {
		if (err)
			*err = -1;
		return -1;
	}
#ifdef MISALIGNED
	if (MISALIGNED(user, 4)) }
		ssig(udata.u_proc, SIGBUS);
		*err = -1;
		return -1;
	}
#endif
	if (err)
		*err = 0;
	return _ugetl(uaddr);
}

int uputl(uint32_t val, void *uaddr)
{
	if (!valaddr(uaddr, 4))
		return -1;
#ifdef MISALIGNED
	if (MISALIGNED(user, 2)) }
		ssig(udata.u_proc, SIGBUS);
		return -1;
	}
#endif
	return _uputl(val, uaddr);
}

#endif

/*
 *	Optional C language implementation for porting to new processors
 *	or where asm isn't needed
 */
#ifdef CONFIG_USERMEM_C

usize_t _uget(const uint8_t *user, uint8_t *dest, usize_t count)
{
	uint8_t tmp;
	while(count--) {
		BANK_PROCESS;
		tmp = *user++;
		BANK_KERNEL;
		*dest++ = tmp;
	}
	return 0;
}

int16_t _ugetc(const uint8_t *user)
{
	uint8_t tmp;
	BANK_PROCESS;
	tmp = *user;
	BANK_KERNEL;
	return tmp;
}

uint16_t _ugetw(const uint16_t *user)
{
	uint16_t tmp;
	BANK_PROCESS;
	tmp = *user;
	BANK_KERNEL;
	return tmp;
}

int _uput(const uint8_t *source, uint8_t *user, usize_t count)
{
	uint8_t tmp;
	while(count--) {
		tmp = *source++;
		BANK_PROCESS;
		*user++ = tmp;
		BANK_KERNEL;
	}
	return 0;

}

int _uputc(uint16_t value,  uint8_t *user)
{
	BANK_PROCESS;
	*user = value;
	BANK_KERNEL;
	return 0;
}

int _uputw(uint16_t value,  uint16_t *user)
{
	BANK_PROCESS;
	*user = value;
	BANK_KERNEL;
	return 0;
}

int _uzero(uint8_t *user, usize_t count)
{
	BANK_KERNEL;
	while(count--)
		*user++=0;
	BANK_PROCESS;
	return 0;
}

#ifdef CONFIG_32BIT

uint32_t _ugetl(void *uaddr)
{
	uint32_t v;
	BANK_PROCESS;
	v = *(uint32_t *)uaddr;
	BANK_KERNEL;
	return v;
}

int _uputl(uint32_t val, void *uaddr)
{
	BANK_PROCESS;
	*(uint32_t *)uaddr = val;
	BANK_KERNEL;
	return 0;
}

#endif
#endif

#ifdef CONFIG_USERMEM_DIRECT

/* Systems where all memory is always mapped for live processes and kernel */

usize_t _uget(const uint8_t *user, uint8_t *dest, usize_t count)
{
	memcpy(dest, user, count);
	return 0;
}

int _uput(const uint8_t *source, uint8_t *user, usize_t count)
{
	memcpy(user, source, count);
	return 0;
}

int _uzero(uint8_t *user, usize_t count)
{
	memset(user, 0, count);
	return 0;
}

#ifdef CONFIG_32BIT

uint32_t _ugetl(void *uaddr)
{
	return *(uint32_t *)uaddr;
}

int _uputl(uint32_t val, void *uaddr)
{
	*(uint32_t *)uaddr = val;
	return 0;
}

#endif
#endif
#endif
