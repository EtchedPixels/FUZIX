#ifndef _SYS_STATFS_H
#define _SYS_STATFS_H

struct statfs {
  short	f_fstype;
  long  f_bsize;
  long  f_frsze;
  long  f_blocks;
  long  f_bfree;
  long  f_files;
  long  f_ffree;
  char	f_fname[6];
  char  f_fpack[6];
};

#endif