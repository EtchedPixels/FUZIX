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

static void mem_free(struct mem *m)
{
	struct memblk *p = &m->memblk[0];
	int i;
	if (m->users == 0)
		panic("mref");
	m->users--;
	if (m->users == 0) {
		for (i = 0; i < MAX_BLOCKS; i++) {
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
	if (n) {
                copy_blocks(n, p, (e - p) >> 9);
	}
	return n;
}

static struct mem *mem_clone(struct mem *m, uint8_t owner)
{
	struct mem *n = mem_alloc();
	struct memblk *p = &m->memblk[0];
	struct memblk *t = &n->memblk[0];
	int i;
	/* FIXME: need a per block 'RO' flag for non-copied blocks */
	for (i = 0; i < MAX_BLOCKS; i++) {
		if (p->start) {
			t->start = kdup(p->start, p->end, owner);
			if (t->start == NULL) {
				mem_free(n);
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

	for (i = 0; i < MAX_BLOCKS; i++) {
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

	for (i = 0; i < MAX_BLOCKS; i++) {
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
	if (platform_udata_set(p))
		return ENOMEM;
	/* Init is special */
	if (p->p_pid == 1) {
		struct memblk *mb;
		udata_shadow = p->p_udata;
		store[nproc] = mem_alloc();
		mem[nproc] = store[nproc];
		mem[nproc]->last = 0;
		mb = &mem[nproc]->memblk[0];
		/* Must be a multiple of 512 */
		mb->start = kmalloc(8192, 0xFD	/* Dummy owner */);
		mb->end = mb->start + 8192;
		if (mb->start == 0)
			panic("alloc");
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
	kprintf("%d: ps:store %p mem %p\n", proc, store[proc], mem[proc]);
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
			kprintf
			    ("%d: pagemap_free: busy non live - giving away to %d.\n",
			     proc, n);
#endif

			pagemap_switch(&ptab[n], 1);
			/* We gave our copy away, so free the store copy
			   we just got donated */
			mem_free(store[proc]);
		}
#ifdef DEBUG
		kprintf("%d: pagemap_free: own live copy.\n", proc);
#endif
		/* Drop the reference count on the mem */
		mem_free(m);
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
	mem_free(m);
	store[proc] = NULL;
	mem[proc] = NULL;
}

/* Called on execve: we should ideally work out whether freeing the
   old map would allow us to make the new allocation but in practice we almost
   always fork/exec so it's not that important.

   TODO: support multiple blocks and allocate code/data separately allowing
   for the alignment. We should probably trim the alignment to 64 bytes as
   well

   Supporting re-entrant binaries will need the binaries to have the data
   aligned so maybe force it in the link ? */
int pagemap_realloc(struct exec *hdr, usize_t size)
{
	unsigned int proc = udata.u_page;
	struct memblk *mb;
	struct mem *m;


	m = mem_alloc();

#ifdef DEBUG
	kprintf("%d:pr:store %p mem %p\n", proc, store[proc], mem[proc]);
#endif

	mb = &m->memblk[0];

	/* Snap to a block boundary for a fast memcpy/swap */
	size = (size + 511) & ~511;

	mb->start = kmalloc(size, proc);
	mb->end = mb->start + size;

	if (mb->start == NULL)
		return ENOMEM;
	/* Free the old map */
	pagemap_free(udata.u_ptab);
	store[proc] = mem[proc] = m;
	return 0;
}

unsigned long pagemap_mem_used(void)
{
	return kmemused() >> 10;	/* In kBytes */
}

/* Extra helper for exec32 */

uaddr_t pagemap_base(void)
{
	unsigned int proc = udata.u_page;
	return (uaddr_t)mem[proc]->memblk[0].start;
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

	/* Map 0 is the image, the user doesn't get to play with that one */
	for (i = 1; i < MAX_BLOCKS; i++) {
		if (m->start == NULL) {
			m->start = kmalloc(size, proc);
			if (m->start == NULL) {
				udata.u_error = ENOMEM;
				return -1;
			}
			m->end = m->start + size;
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

	for (i = 1; i < MAX_BLOCKS; i++) {
		if (m->start == base) {
			kfree_s(m->start, m->end - m->start);
			m->start = NULL;
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
 *
 *	FIXME: need to fix this due to code/data alignment ?
 */
usize_t valaddr(const uint8_t *pp, usize_t l)
{
	const void *p = pp;
	const void *e = p + l;
	unsigned int proc = udata.u_page;
	int n = 0;
	struct memblk *m = &mem[proc]->memblk[0];

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
		m++;
		n++;
	}
	udata.u_error = EFAULT;
	return 0;
}

#endif
