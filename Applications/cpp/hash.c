
#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#include <string.h>
#else
#include <malloc.h>
#endif
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include "cc.h"

/*
 * Two functions:
 * char * set_entry(char * name, void * value);
 *        returns a pointer to the copy of the name;
 *
 * void * read_entry(char * name);
 *        returns the value;
 */

#define register

struct hashentry {
	struct hashentry *next;
	unsigned size;			/* Size */
	struct define_item *d;		/* Block of data (NULL swapped) */
	unsigned offset;		/* Offset on disk in 16 byte chunks */
	char word[1];
};

#define HASHSIZE	256

static struct hashentry *hashtable[HASHSIZE];
int hashsize = 0xFF;		/* 2^X -1 */
int hashcount = 0;

static int swap_fd;

static void page_out(struct hashentry *h)
{
	if (lseek(swap_fd, h->offset << 4, 0) < 0 ||
		write(swap_fd, h->d, h->size) != h->size) {
		cerror("swap error");
		exit(1);
	}
}

static void page_in(struct hashentry *h)
{
	h->d = xmalloc(h->size);
	if (lseek(swap_fd, h->offset << 4, 0) < 0 ||
		read(swap_fd, h->d, h->size) != h->size) {
		cerror("swap error");
		exit(1);
	}
	h->d->name = h->word;
}

static int hashvalue(char *word);

struct define_item *read_entry(char *word)
{
	unsigned hash_val;
	register struct hashentry *hashline;
	hash_val = hashvalue(word);

	hashline = hashtable[hash_val];

	for (; hashline; hashline = hashline->next) {
		if (word[0] != hashline->word[0])
			continue;
		if (strcmp(word, hashline->word))
			continue;
		if (hashline->d == NULL)
			page_in(hashline);
		hashline->d->flags |= F_BUSY;
		return hashline->d;
	}
	return 0;
}

void unlock_entry(struct define_item *d)
{
	if (d)
		d->flags &= ~F_BUSY;
}

char *set_entry(char *word, struct define_item *d, unsigned size)
{
	unsigned hash_val;
	register struct hashentry *hashline, *prev;
	hash_val = hashvalue(word);

	hashline = hashtable[hash_val];

	for (prev = 0; hashline; prev = hashline, hashline = hashline->next) {
		if (word[0] != hashline->word[0])
			continue;
		if (strcmp(word, hashline->word))
			continue;
		/* This is not a normal case so we just grow the page file */
		if (d) {
			hashline->d = d;
			hashline->size = size;
			hashline->offset = 0xFFFF;
		} else {
			if (prev == 0)
				hashtable[hash_val] = hashline->next;
			else
				prev->next = hashline->next;
			free(hashline);
			return 0;
		}
		return hashline->word;
	}
	if (d == 0)
		return 0;

	/* Add record */
	hashline = xmalloc(sizeof(struct hashentry) + strlen(word));
	hashline->next = hashtable[hash_val];
	hashline->d = d;
	hashline->size = size;
	hashline->offset = 0xFFFF;
	strcpy(hashline->word, word);
	hashtable[hash_val] = hashline;
	return hashline->word;
}

static int hashvalue(char *word)
{
	register unsigned val = 0;
	register char *p = word;

	while (*p) {
		val = ((val << 4) ^ ((val >> 12) & 0xF) ^ ((*p++) & 0xFF));
	}
	val &= HASHSIZE - 1;
	return val;
}

static unsigned page_next(void)
{
	return (lseek(swap_fd, 0L, SEEK_END) + 15) >> 4;
}

static unsigned flush_nodes(struct hashentry *hp)
{
	register struct hashentry *h = hp;
	unsigned n = 0;
	while(h) {
		if (h->d && !(h->d->flags & F_INUSE)) {
			if (h->offset == 0xFFFF)
				h->offset = page_next();
			n++;
			page_out(h);
			free(h->d);
			h->d = NULL;
		}
		h = h->next;
	}
	return n;
}

unsigned memory_short(void)
{
	struct hashentry **h = hashtable;
	unsigned n = 0;
	while(h < hashtable + HASHSIZE)
		n += flush_nodes(*h++);
	return n;
}

void hash_init(void)
{
	/* FIXME: name based on pid etc */
	swap_fd = open(".cppswap", O_RDWR|O_CREAT|O_TRUNC, 0600);
	if (swap_fd == -1) {
		perror("swap");
		exit(1);
	}
	unlink(".cppswap");
}
