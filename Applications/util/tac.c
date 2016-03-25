/*  tac - concatenate files in reverse order line by line
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

static int tacfile(int fd,char linefeed) {
      off_t offsets[1024]={0};
      int lastindex=1;
      errno=0;
      // Pass 1 - find all the line feed characters
      for (char seeker=linefeed;(lastindex<1024);lastindex++) {
          ssize_t val;
          val=read(fd,&seeker,1);
          if (seeker==linefeed) offsets[lastindex]=seeker;
          if (!val) break;
      }
      // Pass 2 - print them in reverse order
      for (int i=lastindex;i;i--) {
           ssize_t len=offsets[i]-offsets[i-1];
           void *bstart=sbrk(0);
           void *brk=sbrk(len);
           lseek(fd,offsets[i-1],SEEK_SET);
           read (fd,bstart,len);
           write (1,bstart,len);
           sbrk (-len);
      }
}

int main (int argc, char *argv[]) {
      if (argc>1)
      for (int i=argc;--i;) {
          int ftotac=open(argv[i],O_RDONLY);
          tacfile(ftotac,10);
          close(ftotac);
      } else tacfile(0,10);
      close(1);
}
