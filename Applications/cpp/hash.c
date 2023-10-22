
#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#include <string.h>
#else
#include <malloc.h>
#endif
#include "cc.h"

/*
 * Two functions:
 * char * set_entry(char * name, void * value);
 *        returns a pointer to the copy of the name;
 *
 * void * read_entry(char * name);
 *        returns the value;
 */

struct hashentry {
	struct hashentry *next;
	void *value;
	char word[1];
};

#define HASHSIZE	256

static struct hashentry *hashtable[HASHSIZE];
int hashsize = 0xFF;		/* 2^X -1 */
int hashcount = 0;

static int hashvalue(char *word);

void *read_entry(char *word)
{
	unsigned hash_val;
	register struct hashentry *hashline;
	if (hashtable == 0)
		return 0;
	hash_val = hashvalue(word);

	hashline = hashtable[hash_val];

	for (; hashline; hashline = hashline->next) {
		if (word[0] != hashline->word[0])
			continue;
		if (strcmp(word, hashline->word))
			continue;
		return hashline->value;
	}
	return 0;
}

char *set_entry(char *word, void *value)
{
	unsigned hash_val, i;
	register struct hashentry *hashline, *prev;
	hash_val = hashvalue(word);

	hashline = hashtable[hash_val];

	for (prev = 0; hashline; prev = hashline, hashline = hashline->next) {
		if (word[0] != hashline->word[0])
			continue;
		if (strcmp(word, hashline->word))
			continue;
		if (value)
			hashline->value = value;
		else {
			if (prev == 0)
				hashtable[hash_val] = hashline->next;
			else
				prev->next = hashline->next;
			free(hashline);
			return 0;
		}
		return hashline->word;
	}
	if (value == 0)
		return 0;

	/* Add record */
	hashline = xmalloc(sizeof(struct hashentry) + strlen(word));
	hashline->next = hashtable[hash_val];
	hashline->value = value;
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
