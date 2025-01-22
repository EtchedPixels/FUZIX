/* This is a version of malloc which will always
 * return a valid pointer, or exit(1) with a
 * message.
 *
 * $Revision: 41.1 $ $Date: 1995/12/29 02:10:46 $
 */

#include "header.h"

#ifdef MALLOCDEBUG
static void *StartSbrk;
static void *mallptr[1024];
static int   mallwho[1024];
static int   mallsize[1024];
#endif

char *Malloc(size, mesg)
 unsigned int size;
 char *mesg;
 {
  char *a;

  if ((a=(char *)malloc(size))==NULL)
   { fprints(2,"Malloc error: %s\n",mesg); exit(1); }
  return(a);
 }


/* The following routines are non-portable, and are used solely for
 * debugging malloc/free calls.
 */

#ifdef MALLOCDEBUG
# undef free
# undef malloc

void initmall()
 {
  int i;

  for (i=0;i<1024;i++) mallptr[i]=NULL;
  StartSbrk= sbrk(0);
 }

void *myMalloc(size)
 unsigned int size;
 {
  void *a;
  int i, *b;

  b= (int *)&size; b--;
  a= (void *)malloc(size);
  if (a)
   for (i=0;i<1024;i++)
    if (mallptr[i]==NULL)
      { mallptr[i]=a; mallwho[i]= *b; mallsize[i]=size; break; }
  fprints(2,"0x%x malloc'd 0x%x, size %d\n",*b,a,size);
  return(a);
 }

void myFree(ptr)
 void *ptr;
 {
  int *b;
  int i;

  b= (int *)&ptr; b--;
  fprints(2,"0x%x ",*b);
  if (ptr<= StartSbrk)
   { fprints(2,"Trying to free a bad pointer, 0x%x\n",ptr); exit(1); }
  else
   {
     for (i=0;i<1024;i++) if (mallptr[i]==ptr) { mallptr[i]=NULL; break; }
     if (i==1024) { fprints(2,"Unmalloc'd ptr 0x%x\n",ptr); exit(1); }
     fprints(2,"%d Freeing 0x%x\n",i,ptr);
     free(ptr);
   }
 }
#endif	/* MALLOCDEBUG */
