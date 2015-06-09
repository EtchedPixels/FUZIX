
//
// readline.c
//
// Copyright (c) 2014 Yorkie Neil <yorkiefixer@gmail.com>
//

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "readline.h"

readline_t *
readline_new(char * buffer) {
  readline_t * rl = (readline_t*) malloc(sizeof(readline_t));
  size_t len = strlen(buffer);

  rl->cursor = 0;
  rl->line = 0;
  rl->buffer = (char*) malloc(len);
  memset(rl->buffer, 0, len);
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

  if (ret == NULL || (len == 0 && cur > buffer_len)) {
    free(ret);
    return NULL;
  }

  memset(ret, 0, len);
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

  memset(ret, 0, len);
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
