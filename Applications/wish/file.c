/* File does the work for opening & reading lines from files. We keep a
 * stack of open fds and buffers for each file. This is to minimise the
 * overhead or read()ing, but avoiding the use of stdio.
 *
 * $Revision: 41.1 $ $Date: 1995/12/29 02:10:46 $
 */

#include "header.h"

static int currfile = 0;
static int filedesc[10], bufbytes[10];
static char *bufptr[10], *filebuf[10];


/* Fileopen opens the named file read-only, and sets the number of bytes
 * buffered in yankbuf to zero. NOTE if filename==NULL, we assume that
 * fd has already been open.
 */
bool
fileopen(filename, fd)
  char *filename;
  int fd;
{
  if (currfile == 9) return (FALSE);		/* No space on stack left */
  currfile++;
  if (filename)
  { if ((filedesc[currfile] = open(filename, O_RDONLY, 0)) == -1)
    { currfile--; fprints(2, "Can't open %s\n", filename); return (FALSE); }
  }
  else
    filedesc[currfile] = fd;

  filebuf[currfile] = (char *) malloc(MAXLL);
  if (filebuf[currfile] == NULL)
  { close(filedesc[currfile]); currfile--;
    fprints(2, "Can't malloc file buffer\n"); return (FALSE);
  }
  bufptr[currfile] = filebuf[currfile];
  bufbytes[currfile] = 0;
  return (TRUE);
}

void
fileclose()
{
  free(filebuf[currfile]);
  close(filedesc[currfile]);
  currfile--;
}

/* Getfileline obtains a line from the opened file.
 */
bool
getfileline(line, nosave)
  uchar *line;
  int *nosave;
{
  int in_line = 0;

  *nosave = 0;
  while (1)			/* Until we have a line, copy stuff */
  {				/* We've already got some to copy */
    if (!in_line)
    {
      for (; bufbytes[currfile] && (*bufptr[currfile] == ' ' ||
	*bufptr[currfile] == '\t'); bufbytes[currfile]--, bufptr[currfile]++);
      in_line = 0;
    }
    if (bufbytes[currfile] == 0)
    { in_line = 1; goto doread; }			/* Yuk, a goto */

    if (*bufptr[currfile] == '#') *nosave = 1;
    for (; bufbytes[currfile] && *bufptr[currfile] != '\n'; bufbytes[currfile]--)
      *line++ = *bufptr[currfile]++;

    if (bufbytes[currfile])	/* We hit a \n, so return */
    { *line = EOS; bufptr[currfile]++;
      bufbytes[currfile]--; return (TRUE);
    }

doread:
    bufptr[currfile] = filebuf[currfile];
							/* Get more characters */
    bufbytes[currfile] = read(filedesc[currfile], filebuf[currfile], MAXLL);
    if (bufbytes[currfile] < 1) return (FALSE);		/* End of file */
  }
}

int
source(argc, argv)
  int argc;
  char *argv[];
{
  extern int saveh;
#ifdef PROTO
  extern bool(*getaline) (uchar *line , int *nosave );
  bool(*oldgetline) (uchar *line , int *nosave );
#else
  extern bool(*getaline) ();
  bool(*oldgetline) ();
#endif

  int oldsaveh;

  if (argc != 2)
  { prints("Usage: source file\n"); return (1); }

#ifdef DEBUG
  prints("About to fileopen %s\n", argv[1]);
#endif

  if (fileopen(argv[1], 0) == FALSE)
  { prints("Unable to source file %s\n", argv[1]); return (1); }
  else
  {
#ifdef DEBUG
    prints("About to send the file through doline()\n");
#endif
    oldsaveh = saveh;
    saveh = FALSE;
    oldgetline = getaline;
    getaline = getfileline;
    doline(FALSE);
    getaline = oldgetline;
    saveh = oldsaveh;
  }
  fileclose();
  return (0);
}
