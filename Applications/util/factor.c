/* factor.c - Factor integers

Copyright (C) 2006, 2013 by Rob Landley <rob@landley.net>
Copyright (C) 2015 by Erkin Alp Güney <erkinalp9035@gmail.com>
Original by Rob-FUZIX porting by Erkin Alp
Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

config FACTOR
  bool "factor"
  default y
  help
    usage: factor NUMBER...

    Factor integers.
*/

#include  <stdio.h>
#include  <ctype.h>
#include  <string.h>
#include  <stdlib.h>

static void factor(char *s)
{
  long l, ll;

  for (;;) {
    char *err = s;

    while(isspace(*s)) s++;
    if (!*s) return;

    l = strtol(s, &s, 0);
    if (*s && !isspace(*s)) {
      fprintf(stderr,"%s: not integer", err);

      return;
    }

    printf("%ld:", l);

    // Negative numbers have -1 as a factor
    if (l < 0) {
      printf(" -1");
      l *= -1;
    }

    // Nothing below 4 has factors
    if (l < 4) {
      printf(" %ld\n", l);
      continue;
    }

    // Special case factors of 2
    while (l && !(l&1)) {
      printf(" 2");
      l >>= 1;
    }

    // test odd numbers.
    for (ll=3; ;ll += 2) {
      long lll = ll*ll;

      if (lll>l || lll<ll) {
        if (l>1) printf(" %ld", l);
        break;
      }
      while (!(l%ll)) {
        printf(" %ld", ll);
        l /= ll;
      }
    }
    putc('\n', stdout);
  }
}

#define LINE_MAX 256

int main(int argc,char *argv[])
{
  if (argc > 1) {
    char **ss;

    for (ss = argv + 1; *ss; ss++) factor(*ss);
  } else for (;;) {
    char s[LINE_MAX];

    if (NULL == gets_s(s, LINE_MAX)) break;
    factor(s);
  }
  return 0;
}
