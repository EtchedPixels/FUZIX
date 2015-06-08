
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
 * char * set_entry(int namespace, char * name, void * value);
 *        returns a pointer to the copy of the name;
 *
 * void * read_entry(int namespace, char * name);
 *        returns the value;
 */

struct hashentry
{
   struct hashentry * next;
   void * value;
   int    namespace;
   char	  word[1];
};

struct hashentry ** hashtable;
int  hashsize  = 0xFF;	/* 2^X -1 */
int  hashcount = 0;
static int hashvalue P((int namespace, char * word));

void *
read_entry(namespace, word)
int namespace;
char * word;
{
   int hash_val;
   struct hashentry * hashline;
   if( hashtable == 0 ) return 0;
   hash_val = hashvalue(namespace, word);

   hashline = hashtable[hash_val];

   for(; hashline; hashline = hashline->next)
   {
      if(namespace != hashline->namespace) continue;
      if(word[0] != hashline->word[0])     continue;
      if(strcmp(word, hashline->word) )    continue;
      return hashline->value;
   }
   return 0;
}

char *
set_entry(namespace, word, value)
int namespace;
char * word;
void * value;
{
   int hash_val, i;
   struct hashentry * hashline, *prev;
   hash_val = hashvalue(namespace, word);

   if( hashtable )
   {
      hashline = hashtable[hash_val];

      for(prev=0; hashline; prev=hashline, hashline = hashline->next)
      {
         if(namespace != hashline->namespace) continue;
         if(word[0] != hashline->word[0])     continue;
         if(strcmp(word, hashline->word) )    continue;
         if( value ) hashline->value = value;
         else
         {
            if( prev == 0 ) hashtable[hash_val] = hashline->next;
            else            prev->next = hashline->next;
            free(hashline);
            return 0;
         }
         return hashline->word;
      }
   }
   if( value == 0 ) return 0;
   if( hashtable == 0 )
   {
      hashtable = malloc((hashsize+1)*sizeof(char*));
      if( hashtable == 0 ) cfatal("Out of memory");
      for(i=0; i<=hashsize; i++) hashtable[i] = 0;
   }
   /* Add record */
   hashline = malloc(sizeof(struct hashentry)+strlen(word));
   if( hashline == 0 ) cfatal("Out of memory");
   else
   {
      hashline->next  = hashtable[hash_val];
      hashline->namespace = namespace;
      hashline->value = value;
      strcpy(hashline->word, word);
      hashtable[hash_val] = hashline;
   }
   return hashline->word;
}

static int hashvalue(namespace, word)
int namespace;
char * word;
{
   int val = namespace;
   char *p = word;

   while(*p)
   {
      val = ((val<<4)^((val>>12)&0xF)^((*p++)&0xFF));
   }
   val &= hashsize;
   return val;
}
