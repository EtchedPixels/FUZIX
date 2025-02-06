/* Variables
 *
 * $Revision: 41.2 $ $Date: 1996/06/14 06:24:54 $
 */

#include "header.h"

struct vallist vlist=		/* The linked list of vars */
	{ NULL, NULL};

char *
EVget(name)			/* Get value of variable */
  char *name;
{
  struct val *v;

  if ((v = searchval(&vlist, name, TRUE, FALSE)) == NULL || v->name == NULL)
    return (NULL);
  return (v->val);
}


bool 
EVinit()			/* Initialise symtable from environment */
{
  extern char **environ;
  int i;
  char c, *name, *val;
  struct val *v;

  for (i = 0; environ[i] != NULL; i++)
  {
    name = environ[i];
    val = strchr(name, '=');
    c = *val;
    *(val++) = '\0';
    v= (struct val *)Malloc(sizeof(struct val), "EVinit val malloc");
    v->name= Malloc(strlen(name)+1, "EVinit name malloc");
    v->val= Malloc(strlen(val)+1, "EVinit val malloc");
    strcpy(v->name, name); strcpy(v->val, val);
    v->exported=TRUE;
    saveval(&vlist, v);
    *(--val) = c;
  }
  return (TRUE);
}

#ifndef NO_VAR
bool 
EVupdate()			/* Build envp from symbol table */
{
  extern char **environ;
  int i, envi, nvlen;
  struct val *v;
  static bool updated = FALSE;

  for (i = 0, v = vlist.head; v != NULL; v = v->next)
    i += v->exported;

  if (!updated)
    if ((environ = (char **) malloc((i + 1) * sizeof(char *))) == NULL)
      return (FALSE);
  envi = 0;
  for (v = vlist.head; v != NULL; v = v->next)
  {
    if (v->name == NULL || !v->exported)
      continue;
    nvlen = strlen(v->name) + strlen(v->val) + 2;
    if (!updated)
    {
      if ((environ[envi] = (char *) malloc(nvlen)) == NULL)
	return (FALSE);
    }
    else if ((environ[envi] = (char *) realloc(environ[envi], nvlen)) == NULL)
      return (FALSE);
    sprints(environ[envi], "%s=%s", v->name, v->val);
    envi++;
  }
  environ[envi] = NULL;
  updated = TRUE;
  return (TRUE);
}

int 
export(argc, argv)		/* Export command */
  int argc;
  char *argv[];
{
  int i;
  struct val *v;

  if (argc < 2)
  {
    prints("Usage: export var [var] ...\n");
    return (1);
  }
  for (i = 1; i < argc; i++)
   {
     v = searchval(&vlist, argv[i], TRUE, FALSE);      /* Setenv */
        if (v == NULL)
          prints("No such variable: %s\n", argv[i]);
        else
          v->exported = TRUE;
   }
  return (0);
}

int 
shift(argc, argv)
  int argc;
  char *argv[];
{
  extern int Argc;
  extern char **Argv;
  int i = 1;

  if (argc > 2)
  {
    prints("Usage: shift [val]\n");
    return (1);
  }
  if (argc == 2)
    i = atoi(argv[1]);
  if (i > Argc)
  {
    prints("Not enough vars to shift\n");
    return (1);
  }
  Argv += i;
  Argc -= i;
  return (0);
}

int 
unset(argc, argv)
  int argc;
  char *argv[];
{
  int i, j;
  struct val *v;

  j = strcmp(argv[0], "unexport");
  if (argc < 2)
  {
    prints("Usage: %s [var] [var] ...\n", argv[0]);
    return (1);
  }

  for (i = 1; i < argc; i++)
    switch (j)
    {
      case 0:
	v = searchval(&vlist, argv[i], TRUE, FALSE);	/* Unsetenv */
	if (v == NULL)
	  prints("No such variable: %s\n", argv[i]);
	else
	  v->exported = FALSE;
	break;
      default:
	if (!searchval(&vlist, argv[i], FALSE, FALSE))
	  prints("No such variable: %s\n", argv[i]);
    }
  return (0);
}

int
set(argc, argv)
  int argc;
  char *argv[];
{
  struct val *v;

  if ((argc !=1) && (argc!=3))
  { prints("Usage: %s variable value, or %s\n", argv[0], argv[0]); return (1); }

  if (argc==3)
   { setval(argv[1], argv[2], &vlist);
     if (!strcmp(argv[0], "setenv")) 
      {
       v = searchval(&vlist, argv[1], TRUE, FALSE);      /* Setenv */
       if (v != NULL) v->exported = TRUE;
      }
   }
  else
   {
    if (!strcmp(argv[0], "setenv"))
     {
      for (v = vlist.head; v != NULL; v = v->next)
       if (v->name != NULL && v->exported)
        prints("%s=%s\n", v->name, v->val);
     }
    else
     {
      for (v = vlist.head; v != NULL; v = v->next)
       if (v->name != NULL)
        prints("%s=%s\n", v->name, v->val);
     }
   }
  return (0);
}
#endif	/* NO_VAR */
