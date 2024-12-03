/* We assume this is a suitable alignment for anything. Must be such that
   BLKSIZE is an exact divide */
struct memh {
    struct memh *next;
    size_t size;
};

extern struct memh __mroot;
extern struct memh *__mfreeptr;

#define MH(p)	(((struct memh *)(p)) - 1)
#define BRKSIZE	(512 / sizeof(struct memh))

