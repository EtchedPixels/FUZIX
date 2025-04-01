/* This file does builtins
 *
 * $Revision: 41.2 $ $Date: 1996/06/14 06:24:54 $
 */

#include "header.h"

#ifdef PROTO
static int Echo(int argc, char *argv[]),  Cd(int argc, char *argv[]);
static int Tilde(int argc, char *argv[]), Untilde(int argc, char *argv[]);
static int Exit(int argc, char *argv[]),  Exec(int argc, char *argv[]);
static int Umask(int argc, char *argv[]);
#else
static int Echo(),  Cd();
static int Tilde(), Untilde();
static int Exit(),  Exec();
static int Umask();
#endif

struct builptr buillist[] = {
  {"cd", Cd},
  {"echo", Echo},
  {"exit", Exit},
  {"exec", Exec},
  {"umask", Umask},
  {"source", source},
#ifndef NO_BIND
  {"bind", Bind},
  {"unbind", unbind},
#endif
#ifndef NO_HISTORY
  {"history", history},
#endif
#ifndef NO_VAR
  {"export", export},
  {"unexport", unset},
  {"set", set},
  {"unset", unset},
  {"setenv", set},
  {"shift", shift},
#endif
#ifndef NO_ALIAS
  {"alias", alias},
  {"unalias", unalias},
#endif
#ifndef NO_TILDE
  {"tilde", Tilde},
  {"untilde", Untilde},
#endif
#ifndef NO_JOB
#if defined(UCBJOB) || defined(POSIXJOB) || defined(V7JOB)
  {"bg", bg},
  {"fg", fg},
#endif
  {"jobs", joblist},
  {"kill", Kill},
#endif
  {NULL, NULL}};


static int 
Exit(argc, argv)
  int argc;
  char *argv[];
{
  int how;

  if (argc == 2)
    how = atoi(argv[1]);
  else
    how = 0;
  leave_shell(how);
  return (1);
}

static int 
Exec(argc, argv)
  int argc;
  char *argv[];
{
  argv++;
  execvp(argv[0], argv);
  fprints(2, "Can't exec %s\n", argv[0]);
  return (1);
}

static int 
Echo(argc, argv)
  int argc;
  char *argv[];
{
  int doreturn = 1;
  int firstarg = 1;

  if (argc > 1 && !strcmp(argv[1], "-n"))
  {
    doreturn = 0;
    firstarg = 2;
  }

  for (; firstarg < argc; firstarg++)
  {
    (void) write(1, argv[firstarg], strlen(argv[firstarg]));
    (void) write(1, " ", 1);
  }
  if (doreturn)
    (void) write(1, "\n", 1);
  return (0);
}

/* Here is the global definition for the current directory variable */
char currdir[128];

static int 
Cd(argc, argv)
  int argc;
  char *argv[];
{
  extern struct vallist vlist;
  char *path;

  if (argc > 1)
    path = argv[1];
  else if ((path = EVget("HOME")) == NULL)
    path = ".";
  if (chdir(path) == -1)
  {
    fprints(2, "%s: bad directory\n", path);
    return (1);
  }
#ifdef USES_GETWD
  if (getwd(currdir))
#else
  if (getcwd(currdir, MAXPL))
#endif
  {
    setval("cwd", currdir, &vlist);
    return (0);
  }
  else
  {
    (void) write(2, "Can't get cwd properly\n", 23);
    return (1);
  }
}


#ifndef NO_TILDE
/* Tilde is a builtin which associates a dir name
 * with a shorthand for that dir.
 * If called with -l, print out all the passwd file too.
 *
 * Name holds the shorthand name (starting with a ~).
 * Var holds the actual name of the directory.
 */

struct vallist tlist;		/* The tilde list */

static int 
Tilde(argc, argv)
  int argc;
  char *argv[];
{
  struct val *t;
  struct passwd *entry;
  bool printall = FALSE;

  switch (argc)
  {
    case 3:
	setval(argv[1], argv[2], &tlist);
	return (0);
      break;
    case 2:
      if (strcmp(argv[1], "-l"))
      {
	if ((t = searchval(&tlist, argv[1], TRUE, FALSE)) != NULL)
	  prints("%s  %s\n", t->name, t->val);
	break;
      }
      else
	printall = TRUE;
    case 1:
      for (t = tlist.head; t; t = t->next)
	prints("%s  %s\n", t->name, t->val);
      if (printall)
      {
	while ((entry = getpwent()) != NULL)
	  prints("%s  %s\n", entry->pw_name, entry->pw_dir);
	endpwent();
      }
      break;
    default:
      prints("Usage: tilde [shorthand [dir]] or tilde -l\n");
      return (1);
  }
  return (0);
}


/* Untilde removes a shorthand from the tilde list */

static int 
Untilde(argc, argv)
  int argc;
  char *argv[];
{
  int i;

  if (argc < 2)
  {
    prints("Usage: %s [short] [short] ...\n", argv[0]);
    return (1);
  }
  for (i = 1; i < argc; i++)
    if (!searchval(&tlist, argv[i], FALSE, FALSE))
      prints("No such shorthand: %s\n", argv[i]);
  return (0);
}
#endif


static int Umask(argc,argv)
 int argc;
 char *argv[];
 {
  int umaskval;
  char *cmd;
  
  switch(argc)
   {
    case 1: umaskval=umask(0); umask(umaskval);
	    prints("0%o\n",umaskval);
	    return(0);
    case 2: if (*argv[1]=='0')		/* Convert octal */
	     {
  	      cmd= argv[1]; umaskval=0;
	      while (isdigit(*cmd))
                umaskval = (umaskval << 3) + (*(cmd++) - 48);
	     }
	    else umaskval=atoi(argv[1]);
	    umask(umaskval);
	    return(0);
    default:
  	fprints(2,"Usage: umask [value]\n"); return(1);
   }
 }

/* Do builtin. This returns either the positive exit status of the builtin,
 * or -1 indicating there was no builtin. We also return 0 for the `pid'
 * of the builtin. Note that `fg' is an exception - it returns the new
 * fg pid (i.e exit status for fg of 0) OR 0 (i.e exit status for fg of 1).
 * So we have to do some mangling with the fg return value only.
 */
int 
builtin(argc, argv, rtnpid)
  int argc;
  char *argv[];
  int *rtnpid;
{
  struct builptr *bptr;

  *rtnpid= 0;				/* Usually */
  for (bptr = buillist; bptr->name != NULL; bptr++)
   {
    if (!strcmp(argv[0], bptr->name))
      if (strcmp(argv[0],"fg"))		/* Not fg */
        return ((*(bptr->fptr)) (argc, argv));
      else
	{ *rtnpid= (*(bptr->fptr)) (argc, argv);
	  return((*rtnpid)?0:1);
	}
   }
  return (-1);
}
