#include "vfd-debug.h"

void vfd_debug_init(void) {
  VFD_C = 0x30;
  VFD_D = 0;
  VFD_C = 1;
  VFD_C = 0x0F;
  VFD_D = '>';
}

