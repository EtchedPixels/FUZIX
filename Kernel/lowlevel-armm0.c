#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdarg.h>

/* Called on return from system calls while on the user stack, immediately
 * before returning into the user program. A signal may longjmp out, so this
 * may not return. */
void deliver_signals(void)
{
	for (;;)
	{
		uint8_t cursig = udata.u_cursig;
		if (!cursig)
			break;

		typedef int (*sigvec_fn)(int);
		sigvec_fn fn = udata.u_sigvec[cursig];

		/* Semantics for now: signal delivery clears handler. */
		udata.u_sigvec[cursig] = 0;
		udata.u_cursig = 0;

		fn(cursig);
	}
}

void doexec(uaddr_t entrypoint)
{
	asm volatile ("mov sp, %0\nisb\nbx %1"
		:: "r" (udata.u_sp), "r" (entrypoint));
    __builtin_unreachable();
}

void swap_blocks(void* p1, void* p2, size_t len)
{
	uint32_t* u1 = p1;
	uint32_t* u2 = p2;

	while (len != 0)
    {
        uint32_t t = *u1;
        *u1 = *u2;
        *u2 = t;
        u1++;
        u2++;
		len -= 4;
    }
}


