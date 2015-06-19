/*  rewind - concatenate files in reverse order and backwards
 *  Copyright (C) 2015 Erkin Alp Güney
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

static int backwards(int fd) {
      for (off_t ind=lseek(fd,0,SEEK_END);ind>=0;ind=lseek(fd,-2,SEEK_CUR)) {
           char buf;
           read (fd,&buf,1);
           write (1,&buf,1);
      }
}

int main (int argc, char *argv[]) {
      if (argc>1)
      for (int i=argc;--i;) {
          int ftotac=open(argv[i],O_RDONLY);
          backwards(ftotac);
          close(ftotac);
      } else backwards(0);
      close(1);
}
