#include "rpinfo.h"
#include <stdio.h>
#include "err.h"
#include "rpilot.h"

rpinfo *new_rpinfo()
{
  rpinfo *r;

  r = (rpinfo *)malloc( sizeof(rpinfo) );

  r->currline = NULL;
  r->linehead = NULL;
  r->lblhead = NULL;
  r->numhead = NULL;
  r->strhead = NULL;
  r->stk = NULL;
  r->error = ERR_NONE;
  r->status = 0;
  r->lastacc = NULL;
  r->strict = TRUE;
  r->filename = NULL;

  return r;
}
