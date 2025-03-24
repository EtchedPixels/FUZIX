/* Command Line Editor Buffer functions
 *
 * $Revision: 41.2 $ $Date: 1996/06/14 06:24:54 $
 */

#include "header.h"

/* This version of CLE buffers all characters destined for the terminal
 * into a buffer which is flushed just before getcomcmd is called. This
 * should hopefully reduce the number of syscalls used in the CLE. The
 * following variables hold the buffer & associated pointers.
 */

#define OUTSIZE 128		/* The size of the buffer */

#ifndef NO_COMLINED
extern int curs[2];
#endif
extern int wid;
extern bool Msb;

static char outbuf[OUTSIZE];	/* The buffer we use */
static char *outptr = outbuf;	/* Ptr into outbuf for next char */
static int outcnt = 0;		/* Number of chars in outbuf */

/* Flushbuf simply flushes the buffer to stdout */

void
flushbuf()
{
  if (outcnt == 0) return;
  write(1, outbuf, outcnt);
  outcnt = 0;
  outptr = outbuf;
}

/* Addbuf takes a string and adds it to the outbuf. If outbuf is
 * full, we flush the buffer.
 */

void
addbuf(str)
  char *str;
{
  while (1)
  {
    for (; *str && outcnt < (OUTSIZE - 1); outcnt++)
      *(outptr++) = *(str++);
    if (*str == EOS) return;
    flushbuf();
  }
}

/* Mputc prints out a character and updates the cursor position.
 * It also handles control chars by preceding them with a caret.
 */

void
mputc(b)
  int b;
{
  extern char *so, *se;
  uchar c = (uchar) b;
  char d = c;

  if (Msb && c > 0x7f)
  { addbuf(so); d &= 0x7f; }
  *(outptr++) = d;		/* We mimic addbuf for 1 char */
  if (++outcnt == (OUTSIZE - 1)) flushbuf();
  if (Msb && c > 0x7f) addbuf(se);
#ifndef NO_COMLINED
  curs[0]++;
  if (curs[0] >= wid)
  { addbuf("\n");		/* goto start of next line */
    curs[0] = curs[0] % wid;	/* hopefully gives zero */
    curs[1]++;
  }
#endif
}
