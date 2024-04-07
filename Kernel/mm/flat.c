/*
 *	Memory allocation modules for systems with a single flat address
 *	space and no bank or limit registers (eg 68000). These platforms use
 *	a malloc style allocator to manage the memory pool. As flat systems
 *	require a different implementation of uget/uput and friends these are
 *	also kept in this file, and the usual usermem.c is not used
 *
 *	We keep a fixed maximum blocks per user for speed, and to stop bad
 *	processes. We could swap that for a linked list (using 8 bytes at
 *	the head of each allocation).
 *
 *	Other requirements
 *	- fork() is not currently supported in a flat machine (and expensive)
 *	- there is no swap support
 *
 *	Set:
 *	CONFIG_FLAT
 *
 *	TODO:
 *	Reference counting (for sticky binaries, vfork, fork emulation)
 *	Switch around on free when doing fork emulation (if we have the same
 *	addresses with many users we need to keep the memory available but free
 *	the spare for the next copy when it is copied into the real range).
 *
 *	For speed we chunk in 512 byte blocks and use 512 byte wide fast
 *	copier/exchange calls.
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#ifdef CONFIG_FLAT

#undef DEBUG

#define MAX_BLOCKS	14	/* Packs to a power of two */

#ifdef CONFIG_SPLIT_ID
#define COPY_BASE	1	/* First block we dup the memory for */
#define MALLOC_BASE	2	/* First block for malloc use */
#else
#define COPY_BASE	0	/* First block we dup the memory for */
#define MALLOC_BASE	2	/* First block for malloc use */
#endif

struct memblk {
	void *start;
	void *end;
};

struct mem {
	int users;
	int last;
	struct memblk memblk[MAX_BLOCKS];
};

static struct mem *mem[PTABSIZE];	/* The map we use */
static struct mem *store[PTABSIZE];	/* Where our memory currently lives */
static struct mem memblock[PTABSIZE];

extern struct u_data *udata_shadow;

/* We are called with an indidcator of whether this is the last user
   of the page sets. If it is we can free the shared code, if it isn't
   we can't */
static void mem_free(struct mem *m, unsigned base)
{
	struct memblk *p = &m->memblk[0];
	int i;
	if (m->users == 0)
		panic("mref");
	m->users--;
	if (m->users == 0) {
		for (i = base; i < MAX_BLOCKS; i++) {
			kfree_s(p->start, p->end - p->start);
			p->start = NULL;
			p++;
		}
	}
}

static struct mem *mem_alloc(void)
{
	struct mem *p = &memblock[0];
	int i;
	for (i = 0; i < PTABSIZE; i++) {
		if (p->users == 0) {
			p->users++;
			p->last = -1;
			return p;
		}
		p++;
	}
	panic(PANIC_MLEAK);
}

static void *kdup(void *p, void *e, uint8_t owner)
{
	void *n = kmalloc(e - p, owner);
	if (n)
                copy_blocks(n, p, (e - p) >> 9);
	return n;
}

static struct mem *mem_clone(struct mem *m, uint8_t owner)
{
	struct mem *n = mem_alloc();
	struct memblk *p = &m->memblk[0];
	struct memblk *t = &n->memblk[0];
	int i;
	/* FIXME: need a per block 'RO' flag for non-copied blocks */
	for (i = 0; i < COPY_BASE; i++) {
		t->start = p->start;
		t->end = p->end;
	}
	for (i = COPY_BASE; i < MAX_BLOCKS; i++) {
		if (p->start) {
			t->start = kdup(p->start, p->end, owner);
			if (t->start == NULL) {
				mem_free(n, COPY_BASE);
				return NULL;
			}
			t->end = t->start + (p->end - p->start);
		}
		t++;
		p++;
	}
	m->users++;
	return n;
}

/* May need to switch owners on objects when we do swap etc */
static void mem_switch(struct mem *a, struct mem *b)
{
	struct memblk *t1 = &a->memblk[0];
	struct memblk *t2 = &b->memblk[0];
	unsigned int i;

	for (i = COPY_BASE; i < MAX_BLOCKS; i++) {
		if (t1->start)
			swap_blocks(t1->start, t2->start,
				    (t1->end - t1->start) >> 9);
		t1++;
		t2++;
	}
}

static void mem_copy(struct mem *to, struct mem *from)
{
	struct memblk *t1 = &from->memblk[0];
	struct memblk *t2 = &to->memblk[0];
	unsigned int i;

	for (i = COPY_BASE; i < MAX_BLOCKS; i++) {
		if (t1->start)
			copy_blocks(t2->start, t1->start,
				(t1->end - t1->start) >> 9);
		t1++;
		t2++;
	}
}

/* Called on a fork and similar

   We make a copy of the pages in the memory, but we don't actually create
   anything useful because the copy can't be mapped at the right address
   
   p is the process we are going to create maps for, udata.u_ptab is our
   own process. init is a special case!
   
*/

int pagemap_alloc(ptptr p)
{
	unsigned int proc = udata.u_page;
	unsigned int nproc = p - ptab;

#ifdef DEBUG
	kprintf("%d: pagemap_alloc %p\n", proc, p);
#endif
	p->p_page = nproc;
	if (plt_udata_set(p))
		return ENOMEM;
	/* Init is special */
	if (p->p_pid == 1) {
		struct memblk *mb;
#if defined udata
		udata_shadow = p->p_udata;
#endif
		store[nproc] = mem_alloc();
		mem[nproc] = store[nproc];
		mem[nproc]->last = 0;
		mb = &mem[nproc]->memblk[0];
		/* Must be a multiple of 512 */
		mb->start = kmalloc(8192, 0xFD	/* Dummy owner */);
		mb->end = mb->start + 8192;
		if (mb->start == 0)
			panic("alloc");
		udata.u_codebase = (uaddr_t)mb->start;
#ifdef DEBUG
		kprintf("init at %p\n", mb->start);
#endif
		return 0;
	}
	/* Allocate memory for the new process. The old will run first
	   We know that mem[proc] = store[proc] as proc is running the fork() */
#ifdef DEBUG
	kprintf("%d: Cloning %d as %d\n", proc, proc, nproc);
#endif
	store[nproc] = mem_clone(mem[proc], nproc);
	if (store[nproc] == NULL)
		return ENOMEM;
	mem[nproc] = mem[proc];
	/* Last for our child is us */
#ifdef DEBUG
	kprintf("%d: pa:store %p mem %p\n", proc, store[nproc],
		mem[nproc]);
#endif
	mem[nproc]->last = proc;
	return 0;
}

/*
 *	store == mem
 *		Which means we ran last with these mappings so all is good
 *
 *	store != mem
 *		In which case our store needs to be swapped
 *		We
 *		- exchange the bytes
 *		- set our mem = store
 *		- set the last users store to be our old "mem"
 */
void pagemap_switch(ptptr p, int death)
{
	unsigned int proc = p->p_page;
	int lproc;

#ifdef DEBUG
	kprintf("%d: ps:store %p mem %p death %d\n", proc, store[proc], mem[proc], death);
#endif
	/* We have the right map (unique or we ran the forked copy last) */
	if (store[proc] == mem[proc]) {
#ifdef DEBUG
		kprintf("Slot %d was mapped already.\n", proc);
#endif
		return;
	}
	/* Who had our memory last ? */
	lproc = mem[proc]->last;
#ifdef DEBUG
	kprintf("%d: Slot %d last had our memory.\n", proc, lproc);
	kprintf("%d: ps:store lp is %p mem %p\n", proc, store[lproc],
		mem[lproc]);
#endif
	/* Give them our store */
	store[lproc] = store[proc];
	/* Take over the correctly mapped copy */
	store[proc] = mem[proc];
	/* Exchange the actual data */
	if (death)
		mem_copy(store[proc], store[lproc]);
	else
		mem_switch(store[proc], store[lproc]);
	/* Admit to owning it */
	mem[lproc]->last = proc;
#ifdef DEBUG
	kprintf("%d:Swapped over (now owned by %d not %d).\n", proc, proc,
		lproc);
	kprintf("%d:pse:store %p mem %p\n", proc, store[proc], mem[proc]);
	kprintf("%d:pse:store lp is %p mem %p\n", proc, store[lproc],
		mem[lproc]);
	kprintf("%d:lp->next %d p->next %d\n", proc, mem[lproc]->last,
		mem[proc]->last);
#endif
}

static int pagemap_sharer(struct mem *ms)
{
	struct mem **m = mem;
	int i;
	for (i = 0; i < PTABSIZE; i++) {
		if (*m == ms && store[i] != ms)
			return i;
		m++;
	}
	panic("share");
}

/* Called on exit */

void pagemap_free(ptptr p)
{
	unsigned int proc = udata.u_page;
	struct mem *m;

	m = mem[proc];
	/*
	 *      Not a saved copy: easy
	 */
	if (m == store[proc]) {
		/* We own the live space but we can't free up the live space
		   if it has another user */
		if (m->users > 1) {
			int n = pagemap_sharer(m);
#ifdef DEBUG
			kprintf("%d: pagemap_free: busy non live - giving away to %d.\n",
			     proc, n);
#endif

			pagemap_switch(&ptab[n], 1);
			/* We gave our copy away, so free the store copy
			   we just got donated */
			mem_free(store[proc], COPY_BASE);
		}
#ifdef DEBUG
		kprintf("%d: pagemap_free: own live copy.\n", proc);
#endif
		/* Drop the reference count on the mem */
		mem_free(m, 0);
		mem[proc] = NULL;
		store[proc] = NULL;
		return;
	}
	/*      Our copy is not the live copy. This cannot normally occur.
	 *      If we hit the case we can just flush it out.
	 */
#ifdef DEBUG
	kprintf("%d:pagemap_free:freeing our copy.\n", proc);
#endif
	/* If it was not a live copy then it wasn't the only copy */
	mem_free(m, COPY_BASE);
	store[proc] = NULL;
	mem[proc] = NULL;
}

/* Called on execve: we should ideally work out whether freeing the
   old map would allow us to make the new allocation but in practice we almost
   always fork/exec so it's not that important.

   TODO: support multiple blocks and allocate code/data separately allowing
   for the alignment. We should probably trim the alignment to 64 bytes as
   well
*/

int pagemap_realloc(struct exec *a, usize_t unused)
{
	unsigned int proc = udata.u_page;
	struct memblk *mb;
	struct mem *m;
	usize_t csize, size;

	m = mem_alloc();

#ifdef DEBUG
	kprintf("%d:pr:store %p mem %p\n", proc, store[proc], mem[proc]);
#endif

	mb = &m->memblk[0];

	/* On a system where we can split code from data the code
	   ends up shared across fork and we occupy two memory blocks
	   independently allocated. On a system where we can't we
	   allocate a single block for everything and the database is
	   just offset */
#ifdef CONFIG_SPLIT_ID
	/* Pad to our block size */
	csize = (a->a_text + 511) & ~511;
	/* Again pad to our block size */
	size = a->a_data + a->a_bss + a->stacksize;
	size = (size + 511) & ~511;

	mb->start = kmalloc(csize, proc);
	if (mb->start == NULL) {
		mem_free(m, 0);
		return ENOMEM;
	}
	mb->end = mb->start + a->a_text;

	mb++;

	mb->start = kmalloc(size, proc);
	if (mb->start == NULL) {
		mem_free(m, 0);
		return ENOMEM;
	}
	mb->end = mb->start + size;

	/* Free the old map */
	pagemap_free(udata.u_ptab);

	/* Set up the new map and pointers */
	udata.u_database = (uaddr_t)mb->start;
	mb--;
	udata.u_codebase = (uaddr_t)mb->start;
	udata.u_ptab->p_size = (csize + size + 0x03FF) >> 10;
#ifdef DEBUG
	kprintf("code %p - %p, data %p - %p\n", udata.u_codebase, udata.u_codebase + csize - 1, udata.u_database, udata.u_database + size - 1);
#endif
#else
	size = a->a_text + a->a_data + a->a_bss + a->stacksize;
	size = (size + 511) & ~511;
	mb->start = kmalloc(size, proc);
	if (mb->start == NULL) {
		mem_free(m, 0);
		return ENOMEM;
	}
	mb->end = mb->start + size;
	pagemap_free(udata.u_ptab);
	udata.u_codebase = (uaddr_t)mb->start;
	udata.u_database = udata.u_codebase + a->a_text;
	udata.u_ptab->p_size = (size + 0x03FF) >> 10;
#ifdef DEBUG
	kprintf("code %p data %p - %p\n", udata.u_codebase, udata.u_database, udata.u_codebase + size - 1);
#endif
#endif
	/* Tell the exec code where the top of the resulting memory (and
	   thus where to build the stack) is */
	udata.u_top = udata.u_database + a->a_data + a->a_bss + a->stacksize;
	store[proc] = mem[proc] = m;
	return 0;
}

usize_t pagemap_mem_used(void)
{
	return kmemused() >> 10;	/* In kBytes */
}

/* The extra syscalls for the pool allocator */

#define size (uint32_t)udata.u_argn
#define flags (uint32_t)udata.u_argn1

/*
 *	void *memalloc(size, flags)
 *
 *	Allocate memory. Flags should currently be set to zero.
 */
arg_t _memalloc(void)
{
	unsigned int proc = udata.u_page;
	struct memblk *m = &mem[proc]->memblk[0];
	int i;

	size = (size + 511) & ~511;
	/* Map 0 and 1 are the image, the user doesn't get to play with that one */
	for (i = MALLOC_BASE; i < MAX_BLOCKS; i++) {
		if (m->start == NULL) {
			m->start = kmalloc(size, proc);
			if (m->start == NULL) {
				udata.u_error = ENOMEM;
				return -1;
			}
			m->end = m->start + size;
			udata.u_ptab->p_size += (size + 0x03FF) >> 10;
			return (arg_t) m->start;
		}
		m++;
	}
	udata.u_error = EMLINK;
	return -1;
}

#undef size
#undef flags

#define base (void *)udata.u_argn

/*
 *	int memfree(void *ptr)
 *
 *	Free a block
 */
arg_t _memfree(void)
{
	unsigned int proc = udata.u_page;
	struct memblk *m = &mem[proc]->memblk[0];
	int i;

	for (i = MALLOC_BASE; i < MAX_BLOCKS; i++) {
		if (m->start == base) {
			kfree_s(m->start, m->end - m->start);
			m->start = NULL;
			udata.u_ptab->p_size -= (m->end - m->start) >> 10;
			return 0;
		}
		m++;
	}
	udata.u_error = EINVAL;
	return -1;
}

#undef size

/*
 *	We make an assumption here: The user process is not guaranteed that
 *	two allocations are adjacent, therefore we don't allow a copy across
 *	what happens to be a join of two banks. We could fix this but it's not
 *	clear it would be wise!
 */
usize_t valaddr(const uint8_t *pp, usize_t l, uint_fast8_t is_write)
{
	const void *p = pp;
	const void *e = p + l;
	unsigned int proc = udata.u_page;
	unsigned int n = 0;
	struct memblk *m = &mem[proc]->memblk[0];

#ifdef CONFIG_SPLIT_ID
	/* First block is R/O */
	if (is_write)
		n++;
#endif		
	while (n < MAX_BLOCKS) {
		/* Found the right block ? */
		if (m->start && p >= m->start && p < m->end) {
			/* Check the actual space */
			if (e >= m->end)
				e = m->end;
			/* Fault error ? */
			if (e == p)
				break;
			/* Return the size we can copy */
			return e - p;
		}
/*		if (m->start)
			kprintf("%p not in %p to %p\n",
				pp, m->start, m->end); */
		m++;
		n++;
	}
	udata.u_error = EFAULT;
	return 0;
}

usize_t valaddr_r(const uint8_t *pp, usize_t l)
{
	return valaddr(pp, l, 0);
}

usize_t valaddr_w(const uint8_t *pp, usize_t l)
{
	return valaddr(pp, l, 1);
}

#ifdef CONFIG_LEVEL_2		/* Coredump support */

/* Write out each segment of memory we have. We don't do anything with the flags
   yet - that will comne later */
void coredump_image(inoptr ino)
{
	unsigned int i = 0;
	unsigned int proc = udata.u_page;
	struct memblk *m = &mem[proc]->memblk[0];
	while(i < MAX_BLOCKS) {
		if (m->start)
			coredump_memory(ino, (uaddr_t)m->start, m->end - m->start, 0);
		m++;
		i++;
	}
}

#endif
#endif
