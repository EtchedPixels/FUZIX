#include <kernel.h>
#include <kdata.h>

struct u_data *udata_shadow = NULL;

_Noreturn void _doexec(uaddr_t start_addr, void *sp);

_Noreturn void doexec(uaddr_t start_addr)
{
  udata.u_insys = false;
  _doexec(start_addr, udata.u_isp);
}

uint32_t ptab_pid(ptptr ptab)
{
  return ptab->p_pid;
}

void stash_sp(void *sp)
{
  udata.u_sp = sp;
}

void *restore_sp(void)
{
  return udata.u_sp;
}

void set_running(ptptr ptab)
{
  ptab->p_status = P_RUNNING;
}
