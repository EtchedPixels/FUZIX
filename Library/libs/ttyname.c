#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>

static char dev[] = "/dev";
static char name[16]; /* should be MAXNAMLEN but that's overkill */

char *ttyname(int fd)
{
   if (ttyname_r(fd, name, sizeof(name)))
      return NULL;
   return name;
}

int ttyname_r(int fd, char *buf, size_t len)
{
   struct stat st, dst;
   DIR  *fp;
   struct dirent *d;
   int noerr = errno;

   if (fstat(fd, &st) < 0)
      return -1;
   if (!isatty(fd))
   {
      errno = ENOTTY;
      return -1;
   }

   fp = opendir(dev);
   if (fp == 0)
      return -1;
   strlcpy(buf, dev, len);
   strlcat(buf, "/", len);

   while ((d = readdir(fp)) != 0)
   {
      if( strlen(d->d_name) > sizeof(name) - sizeof(dev) - 1)
         continue;
      strcpy(name + sizeof(dev), d->d_name);
      if (stat(name, &dst) == 0
         && st.st_dev == dst.st_dev && st.st_ino == dst.st_ino)
      {
	 closedir(fp);
	 errno = noerr;
	 return 0;
      }
   }
   closedir(fp);
   errno = noerr;
   return -1;
}
