#include <unistd.h>
#include <sys/stat.h>

static int statfix(struct stat *st, struct _uzistat *s)
{
  st->st_dev = s->st_dev;
  st->st_ino = s->st_ino;
  st->st_mode = s->st_mode;
  st->st_nlink = s->st_nlink;
  st->st_uid = s->st_uid;
  st->st_gid = s->st_gid;
  st->st_rdev = s->st_rdev;
  st->st_size = s->st_size;
  /* FIXME: these 3 will change when the kernel API is fixed to pass
     some "high bits" stuffed somewhere else */
  st->st_atime = s->st_atime;
  st->st_mtime = s->st_mtime;
  st->st_ctime = s->st_ctime;
  return 0;
}

int fstat(int fd, struct stat *st)
{
  struct _uzistat s;
  if (_fstat(fd, &s) != 0)
    return -1;
  return statfix(st, &s);
}

int stat(const char *path, struct stat *st)
{
  struct _uzistat s;
  if (_stat(path, &s) != 0)
    return -1;
  return statfix(st, &s);
}
