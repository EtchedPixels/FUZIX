/*
 *	FPU context handling. We can't simply do this by CPU type because
 *	our world is a bit more complicated than that.
 */

#ifndef _FPU_H
#define _FPU_H

#ifdef CONFIG_FPU_AMD9511

struct fpu_context {
    uint8_t stack[16];
};

#endif



extern void fpu_save(void);
extern void fpu_restore(void);
extern uint_fast8_t fpu_detect(void);
extern uint8_t fpu_present;

#endif
