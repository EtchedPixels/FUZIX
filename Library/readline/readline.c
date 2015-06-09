
//
// readline.c
// 
// Original library copyright (c) 2014 Yorkie Neil <yorkiefixer@gmail.com>
// FUZIX fixes      copyright (c) 2015 Erkin Alp Güney <erkinalp9035@gmail.com>

// #define _ZERO_IDIOM
// define if the target is pipelined and supports r

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "readline.h"

readline_t *
readline_new(char * buffer) {
  readline_t * rl=NULL;
  do (readline_t*) rl=malloc(sizeof(readline_t)); while ((errno!=ENOMEM)&&(!rl));
  if (rl==NULL) return & (readline_t){0} ;
  size_t len = strlen(buffer);

  rl->cursor = 0;
  rl->line = 0;
  rl->buffer = (char*) malloc(len);
#ifdef _ZERO_IDIOM
  memset(rl->buffer, 0, len);
#endif
  memcpy(rl->buffer, buffer, len);
  return rl;
}

char *
readline_next(readline_t * rl) {
  char * ret = NULL;
  size_t cur = rl->cursor;
  size_t len;
  size_t buffer_len = strlen(rl->buffer);

  while (
    rl->buffer[cur++] != '\n' && 
    cur <= buffer_len);

  len = cur - rl->cursor - 1;
  ret = (char*) malloc(len);
  
  if (ret==NULL) return NULL;
  if ((len == 0 && cur > buffer_len)) {
    free(ret);
    return NULL;
  }

#ifdef _ZERO_IDIOM
  memset(ret, 0, len);
#endif
  memcpy(ret, rl->buffer+rl->cursor, len);
  rl->cursor = cur;
  rl->line += 1;
  return ret;
}

char *
readline_last_from_rl(readline_t * rl) {
  char * ret = NULL;
  size_t cur = strlen(rl->buffer)-1; /* skip \0 of the last line */
  size_t len;
  size_t buffer_len = cur;

  while (cur--) {
    if (rl->buffer[cur] == '\n') {
      cur++;
      break;
    }
  }

  len = buffer_len - cur;
  ret = (char*) malloc(len);

  if (ret == NULL) {
    free(ret);
    return NULL;
  }

#ifdef _ZERO_IDIOM
  memset(ret, 0, len);
#endif
  memcpy(ret, rl->buffer+cur, len);
  return ret;
}

char *
readline_last(char * buffer) {
  readline_t * rl = readline_new(buffer);
  char * ret = readline_last_from_rl(rl);
  readline_free(rl);
  return ret;
}

void
readline_free(readline_t * rl) {
  rl->cursor = 0;
  rl->line = 0;
  free(rl->buffer);
  free(rl);
}
