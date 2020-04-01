#ifndef __VFD_DEBUG_DOT_H__
#define __VFD_DEBUG_DOT_H__

__sfr __at (0) VFD_C;
__sfr __at (1) VFD_D;

void vfd_debug_init(void);

#define vprtch(c) VFD_D=(c);

#endif
