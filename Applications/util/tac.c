/* vi: set sw=4 ts=4: */
/*
 * tac implementation for FUZIX
 * based on busybox one
 *
 * Copyright (C) 2003  Yang Xiaopeng  <yxp at hanwang.com.cn>
 * Copyright (C) 2007  Natanael Copa  <natanael.copa@gmail.com>
 * Copyright (C) 2007  Tito Ragusa    <farmatito@tiscali.it>
 * Copyright (C) 2015  Erkin Alp Güney<erkinalp9035@gmail.com>
 *
 * Licensed under GPLv2, see file LICENCE in this source tree.
 *
 */

/* tac - concatenate and print files in reverse */

/* Based on Yang Xiaopeng's (yxp at hanwang.com.cn) patch
 * http://www.uclibc.org/lists/busybox/2003-July/008813.html
 */

//usage:#define tac_trivial_usage
//usage:    "[FILE]..."
//usage:#define tac_full_usage "\n\n"
//usage:    "Concatenate FILEs and print them in reverse"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

struct lstring {
      int size;
      char buf[1];
};

struct llist_t {
      char *data;
      struct list_t *link;
};
typedef struct llist_t llist;
int main(int argc, char **argv)
{
      char **name;
      FILE *f;
      struct lstring *line = NULL;
      struct llist_t *list = & ((struct llist_t){0});
      int retval = EXIT_SUCCESS;

/* tac from coreutils 6.9 supports:
       -b, --before
              attach the separator before instead of after
       -r, --regex
              interpret the separator as a regular expression
       -s, --separator=STRING
              use STRING as the separator instead of newline
We support none, but at least we will complain or handle "--":
*/
      getopt(argv, "");
      argv += optind;
      if (!*argv)
            *--argv = (char *)"-";
      /* We will read from last file to first */
      name = argv;
      while (*name)
            name++;
      do {
            int ch, i;

            name--;
            f = fopen(*name,"r+");
            if (f == NULL) {
                  perror(*name);
                  retval = EXIT_FAILURE;
                  continue;
            }

            errno = i = 0;
            do {
                  ch = fgetc(f);
                  if (ch != EOF) {
                        if (!(i & 0x7f))
                        /* Grow on every 128th char */
                        line =(struct lstring*) realloc(line, i + 0x7f + sizeof(int) + 1);
                        line->buf[i++] = ch;
                  }
                  if (ch == '\n' || (ch == EOF && i != 0)) {
                        line = realloc(line, i + sizeof(int));
                        line->size = i;
                        llist *strings=list;
                        for(;(strings->link)!=(llist*)NULL;strings=strings->next);
                        llist *item=(llist*)malloc(sizeof(llist));
                        strings->next=(char*)item;
                        item->next=(llist*)NULL;
                        item->data=(char*)line;
                        line = (struct lstring*)NULL;
                        i = 0;
                  }
            } while (ch != EOF);
            /* fgetc sets errno to ENOENT on EOF, we don't want
             * to warn on this non-error! */
            if (errno && errno != ENOENT) {
                  perror(*name);
                  retval = EXIT_FAILURE;
            }
      } while (name != argv);
      char i=0;
      while (list) {
            line = (struct lstring *)list->data;
            write(STDOUT_FILENO, line->buf, line->size);
            *(list)=(llist){0};
            if (i) free(list); else i++;
            list = list->link;
      }

      return retval;
}


