/* This file contains functions relevant to aliases, including alias
 * creation in two ways, listing, interpreting.
 *
 * $Revision: 41.2 $ $Date: 1996/06/14 06:24:54 $
 */

#include "header.h"

#ifndef NO_ALIAS
struct vallist alist=		/* The list of aliases */
	{ NULL, NULL};

static struct val *aline;	/* Pointer to one alias */

/* Checkalias returns a pointer to the alias named by aname. If
 * no alias is found, it returns NULL. It also points aline to the
 * alias definition.
 */
struct val *checkalias(aname)
  char *aname;
{
  aline= searchval(&alist, aname, TRUE, FALSE);
  return(aline);
}

/* Getaliasline returns the alias one line at a time.
 * It depends on having aline initialised correctly.
 */
bool getaliasline(line, nosave)
 uchar *line;
 int *nosave;
 {

  *nosave=0;
  if (aline==NULL) return(FALSE);	/* No alias to return */
  strcpy(line,aline->val);
  aline=NULL;
  return(TRUE);
 }

int alias(argc,argv)
  int argc;
  char *argv[];
{
  struct val *v;

#ifdef DEBUG
  fprints(2,"In alias with argc %d argv[1] %s\n",argc,argv[1]);
#endif

  if (argc==1)
    for (v=alist.head; v; v=v->next) prints("%s\t'%s'\n",v->name, v->val);
  if (argc==2)
  {
    v= searchval(&alist,argv[1],TRUE,FALSE);
    prints("%s\t'%s'\n",v->name, v->val);
  }
  if (argc==3)
    setval(argv[1], argv[2], &alist);
  return(0);
}

int unalias(argc,argv)
  int argc;
  char *argv[];
{
  int i;

  for (i=1;argv[i];i++)
    searchval(&alist,argv[i],FALSE,FALSE);
  return(0);
}
#endif
