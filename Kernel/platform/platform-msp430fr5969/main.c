#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>
#include "externs.h"

uaddr_t ramtop;
uint8_t need_resched;
uint8_t last_interrupt;

struct overlay
{
	uint16_t start;
	uint16_t stop;
};

#define DECLARE_OVERLAY(n) \
	extern char __load_start_##n##_overlay; \
	extern char __load_stop_##n##_overlay

#define DEFINE_OVERLAY(n) \
	{ \
		(uint16_t) &__load_start_##n##_overlay, \
		(uint16_t) &__load_stop_##n##_overlay, \
	}

DECLARE_OVERLAY(syscall_exec16);
DECLARE_OVERLAY(syscall_fs);
DECLARE_OVERLAY(syscall_fs2);
DECLARE_OVERLAY(syscall_fs3);
DECLARE_OVERLAY(syscall_other);
DECLARE_OVERLAY(syscall_proc);

/* Do not change the order here without also changing the list in
 * build.mk. */

const struct overlay overlays[] = {
	DEFINE_OVERLAY(syscall_exec16),
	DEFINE_OVERLAY(syscall_fs),
	DEFINE_OVERLAY(syscall_fs2),
	DEFINE_OVERLAY(syscall_fs3),
	DEFINE_OVERLAY(syscall_other),
	DEFINE_OVERLAY(syscall_proc),
};

const static char overlay_tab[FUZIX_SYSCALL_COUNT] = {
#include "syscallmap.h"
};

char current_overlay = 0;

void plt_idle(void)
{
}

void do_beep(void)
{
}

void program_vectors(uint16_t* pageptr)
{
	/* On the MSP430, with no banking, changing processes doesn't
	 * touch the interrupt vectors. Therefore we don't need to
	 * reprogram them and this is a nop. Go us. */
}

void plt_discard(void)
{
	/* We're done with the start code, and the kernel's about to call
	 * _execve. So we need to make sure it's in memory. */
	udata.u_callno = 23; // execve
	load_overlay_for_syscall();
}

void load_overlay_for_syscall(void)
{
	if (udata.u_callno >= FUZIX_SYSCALL_COUNT)
		return;

	int index = overlay_tab[udata.u_callno];
	if ((index != 0) && (current_overlay != index))
	{
		const struct overlay* o = &overlays[index-1];
		if (o->start)
		{
			current_overlay = index;
			load_overlay(o->start, o->stop);
		}
	}
}

