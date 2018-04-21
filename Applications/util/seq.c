/*
 *  Integer-only implementation of seq
 *
 *  Copyright 2017 Tormod Volden
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

static const char *appname;
static const char *newline = "\n";

static void error(void)
{
  fprintf(stderr, "%s: [-s SEP] [start] [step] end\n", appname);
  exit(1);
}

static long chk_strtol(const char *s)
{
  long value;
  char *end;

  errno = 0;
  value = strtol(s, &end, 0);
  if (errno || *end != 0) {
    fprintf(stderr, "Error: Bad numeral: %s\n", s);
    error();
  }
  return value;
}

int main(int argc, char **argv)
{
  char **argp = argv;
  long start = 1;
  long step = 1;
  long end;
  const char *sep = newline;
  bool printed = false;

  appname = argv[0];
  while(*++argp && (*argp)[0] == '-'
        && ((*argp)[1] < '0' || (*argp)[1] > '9')) {
    --argc;
    switch((*argp)[1]) {
      case 's':
        if ((*argp)[2]) {
          sep = &(*argp)[2];
        } else {
          sep = *++argp;
          --argc;
        }
        break;
      default:
        error();
    }
  }
  switch(argc) {
    case 2:
      end = chk_strtol(argp[0]);
      break;
    case 3:
      start = chk_strtol(argp[0]);
      end = chk_strtol(argp[1]);
      break;
    case 4:
      start = chk_strtol(argp[0]);
      step = chk_strtol(argp[1]);
      end = chk_strtol(argp[2]);
      break;
    default:
      error();
  }

  while ((start <= end && step > 0) || (start >= end && step < 0)) {
    if (printed)
      printf("%s", sep);
    else
      printed = true;
    printf("%ld", start);
    start += step;
  }
  if (printed)
    putchar('\n');
  return 0;
}
