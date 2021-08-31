#include <kernel.h>
#include <kdata.h>

struct svc_frame
{
  uint32_t r12;
  uint32_t pc;
  uint32_t lr;
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
};

void syscall_handler(struct svc_frame *eh)
{
  udata.u_callno = *((uint8_t *)((eh->pc) - 2U));
  udata.u_argn = eh->r0;
  udata.u_argn1 = eh->r1;
  udata.u_argn2 = eh->r2;
  udata.u_argn3 = eh->r3;
  udata.u_insys = true;
  unix_syscall();
  udata.u_insys = false;
  eh->r0 = udata.u_retval;
  eh->r1 = udata.u_error;
  /* TODO - signal delivery */
}
