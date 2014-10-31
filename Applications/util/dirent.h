#ifndef	__DIRENT_H
#define	__DIRENT_H

#include "unix.h"

#ifndef	MAXNAMLEN
#define	MAXNAMLEN	13
#endif

/* Directory stream type */

typedef struct {
  int dd_fd;			/* file descriptor */
  int dd_loc;			/* offset in buffer */
  int dd_size;			/* # of valid entries in buffer */
  struct direct dd_buf;		/* directory entry buffer */
} DIR;				/* stream data from opendir() */

extern DIR *opendir(char *);
extern int closedir(DIR *);
extern struct direct *readdir(DIR *);

#endif /* dirent.h  */

