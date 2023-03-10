
extern arg_t brk_extend(uaddr_t addr);
extern arg_t stack_extend(uaddr_t sp);
extern void pagemap_setup(uaddr_t base, unsigned len);

/* FIXME: this one needs a better name and to belong to the generic page.c
   we need to create */
extern void pagemap_frames(unsigned size);
