#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

static char name[16]; /* should be MAXNAMLEN but that's overkill */

char *ttyname(int fd)
{
   if (ttyname_r(fd, name, sizeof(name)))
      return NULL;
   return name;
}

static uint_fast8_t test_path(const char *p, struct stat *st)
{
   struct stat dst;

   if (stat(p, &dst) == 0
         && st->st_dev == dst.st_dev && st->st_ino == dst.st_ino)
         return 1;
   return 0;
}

int ttyname_r(int fd, char *name, size_t len)
{
   struct stat st;
   DIR  *fp;
   struct dirent *d;
   int noerr = errno;
   char buf[36];	/* 30 + /dev/ + \0 */

   if (fstat(fd, &st) < 0)
      return -1;
   if (!isatty(fd))
   {
      errno = ENOTTY;
      return -1;
   }
   /* Shortcut search: if the tty directory is standard then we won't end
      up doing any searches */
   strcpy(buf, "/dev/tty");
   strcpy(buf + 8, _itoa(minor(st.st_dev)));

   if (test_path(buf, &st))
      goto found;

   /* Do it the hard way */
   fp = opendir("/dev");
   if (fp == 0)
      return -1;

   while ((d = readdir(fp)) != 0) {
      strlcpy(buf + 5, d->d_name, 31);
      if (test_path(buf, &st)) {
	 closedir(fp);
         goto found;
      }
   }
   closedir(fp);
   errno = ENODEV;
   return -1;

   /* Found the path, return it if possible */
found:
   errno = noerr;
   if (strlen(buf) >= len) {
      errno = ERANGE;
      return -1;
   }
   strcpy(name, buf);
   return 0;
}
