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
 *	MAX_POOLS	maximum number of discontiguous memory pools in the
 *			machine
 *	MAX_BLOCKS	maximum blocks per user
 *
 *	TODO:
 *	Reference counting (for sticky binaries, vfork, fork emulation)
 *	Switch around on free when doing fork emulation (if we have the same
 *	addresses with many users we need to keep the memory available but free
 *	the spare for the next copy when it is copied into the real range).
 *
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>

#ifdef CONFIG_FLAT

#define MAX_BLOCKS	15	/* Packs to a power of two */

struct memblk {
	void *start;
	void *end;
};

struct mem {
	int users;
	ptptr last;
	struct memblk memblk[MAX_BLOCKS];
};

/* Eventually abstract page/page2 for this */
static struct mem *mem[PTABSIZE];		/* The map we use */
static struct mem *store[PTABSIZE];	/* Where our memory currently lives */
static struct mem memblock[PTABSIZE];

static void mem_free(struct mem *m)
{
	struct memblk *p = &m->memblk[0];
	int i;
	m->users--;
	if (m->users == 0) {
		for (i = 0; i < MAX_BLOCKS; i++) {
			kfree(p->start);
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
			p->last = NULL;
			return p;
		}
	}
	panic(PANIC_MLEAK);
}

static void *kdup(void *p, void *e)
{
	void *n = kmalloc(e - p);
	if (n)
		memcpy(n, p, e - p);
	return n;
}

static struct mem *mem_clone(struct mem *m)
{
	struct mem *n = mem_alloc();
	struct memblk *p = &m->memblk[0];
	struct memblk *t = &m->memblk[0];
	int i;
	for (i = 0; i < MAX_BLOCKS; i++) {
		t->start = kdup(p->start, p->end);
		if (t->start == NULL) {
			mem_free(n);
			return NULL;
		}
		t->end = t->start + (p->end - p->start);
		t++;
		p++;
	}
	m->users++;
	return n;
}

/*
 *	We make an assumption here: The user process is not guaranteed that
 *	two allocations are adjacent, therefore we don't allow a copy across
 *	what happens to be a join of two banks. We could fix this but it's not
 *	clear it would be wise!
 */
usize_t valaddr(const char *pp, usize_t l)
{
	const void *p = pp;
	const void *e = p + l;
	unsigned int proc = udata.u_page;
	int n = 0;
	struct memblk *m = &mem[proc]->memblk[0];

	while (n < MAX_BLOCKS) {
		/* Found the right block ? */
		if (m->start && m->start >= p && m->end < p) {
			/* Check the actual space */
			if (e >= m->end)
				e = m->end;
			/* Return the size we can copy */
			return e - m->start;
		}
		m++;
		n++;
	}
	return 0;
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

	p->p_page = nproc;
	/* Init is special */
	if (p->p_pid == 1) {
		store[nproc] = mem_alloc();
		store[nproc]->last = p;
		mem[nproc] = store[nproc];
		return 0;
	}
	/* Allocate memory for the old process as a copy of the new as the
	   new will run first. We know that mem[proc] = store[proc] as proc
	   is running the fork() */
	store[proc] = mem_clone(mem[proc]);
	if (store[proc] == NULL)
		return ENOMEM;
	mem[nproc] = mem[proc];
	/* Last for our child is us */
	store[nproc]->last = udata.u_ptab;
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
void pagemap_switch(ptptr p)
{
	unsigned int proc = udata.u_page;
	int lproc;

	/* We have the right map (unique or we ran the forked copy last) */
	if (store[proc] == mem[proc])
		return;
	/* Who had our memory last ? */
	lproc = mem[proc]->last - ptab;
	/* Give them our store */
	store[lproc] = store[proc];
	/* Take over the correctly mapped copy */
	store[proc] = mem[proc];
	/* Exchange the actual data */
	/* FIXME: mem_switch(proc); */
	/* Admit to owning it */
	mem[proc]->last = p;
}


/* Called on exit */

void pagemap_free(ptptr p)
{
	unsigned int proc = udata.u_page;
	struct mem *m;

	m = mem[proc];
	/*
	 *	Not a saved copy: easy
	 */
	if (store[proc] == mem[proc]) {
		mem_free(m);
		mem[proc] = NULL;
		store[proc] = NULL;
		return;
	}
	/*
	 *	Give the live mapping to the previous user
	 */
	if (m->users > 1)
		pagemap_switch(m->last);
	mem_free(store[proc]);
	store[proc] = NULL;
	mem[proc] = NULL;
}

/* Called on execve */
int pagemap_realloc(usize_t size)
{
	unsigned int proc = udata.u_page;
	struct memblk *mb;
	
	pagemap_free(udata.u_ptab);
	
	store[proc] = mem[proc] = mem_alloc();
	mb = &mem[proc]->memblk[0];

	mb->start = kmalloc(size);
	mb->end = mb->start + size;
	if (mb->start == NULL)
		return ENOMEM;
	return 0;
}

unsigned long pagemap_mem_used(void)
{
	return kmemused();
}

/* Extra helper for exec32 */

uaddr_t pagemap_base(void)
{
	unsigned int proc = udata.u_page;
	return mem[proc]->memblk[0].start;
}

/* Uget/Uput 32bit */

uint32_t ugetl(void *uaddr, int *err)
{
	if (!valaddr(uaddr, 4)) {
		if (err)
			*err = -1;
		return -1;
	}
	if (err)
		*err = 0;
	return *(uint32_t *)uaddr;

}

int uputl(uint32_t val, void *uaddr)
{
	if (!valaddr(uaddr, 4))
		return -1;
	return *(uint32_t *)uaddr;
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
			m->start = kzalloc(size);
			if (m->start == NULL) {
				udata.u_error = ENOMEM;
				return -1;
			}
			m->end = m->start + size;
			return (arg_t)m->start;
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
			kfree(base);
			m->start = NULL;
			return 0;
		}
		m++;
	}
	udata.u_error = EINVAL;
	return -1;
}

#undef size

#endif
