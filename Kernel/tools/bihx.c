/*
 *	Banked ihx processor
 *
 *	Split the ihx file into base and bank blocks
 *	Perform the patch ups on the file
 *	Write the stubs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

FILE *fseg[10];
static char fbuf[64];

void open_fseg(int n)
{
  if (n == 0)
    sprintf(fbuf, "common.ihx");
  else
    sprintf(fbuf, "bank%d.ihx", n);
  fseg[n] = fopen(fbuf, "w");
  if (fseg[n] == NULL) {
    perror(fbuf);
    exit(1);
  }
}

void put_seg(int n, char *bp)
{
  if (fseg[n] == NULL)
    open_fseg(n);
  fputs(bp, fseg[n]);
}

void put_all_segs(char *bp)
{
  int i;
  for (i = 0; i < 10; i++) {
    if (fseg[i])
      fputs(bp, fseg[i]);
  }
}

void close_segs(void)
{
  int i;
  for (i = 0; i < 10; i++) {
    if (fseg[i])
      if(fclose(fseg[i]))
        perror("fclose");
  }
}

void bin_segs(void)
{
  int i;
  system("makebin -s 65536 -p common.ihx >common.bin\n");
  for (i = 1; i < 10; i++) {
    if (fseg[i]) {
      snprintf(fbuf, 64, "makebin -s 65536 -p bank%d.ihx >bank%d.bin\n",
        i, i);
      system(fbuf);
    }
  }
}

void save_patch_rule(FILE *fr, char *buf)
{
  fputs(buf, fr);
}

void split_bihx(char *name)
{
  int n;
  char buf[256];
  FILE *fp = fopen(name, "r");
  FILE *fr;

  if (fp == NULL) {
    perror(name);
    exit(1);
  }
  
  fr = fopen("relocs.dat", "w");
  if (fr == NULL) {
    perror("relocs.dat");
    exit(1);
  }

  while(fgets(buf, 256, fp)) {
    /* Leading hex means ihx data for this target */
    if (isdigit(*buf)) {
      sscanf(buf, "%x", &n);
      put_seg(n, buf + 2);
    } else if (*buf == ':')	/* ihx for all (end marker) */
      put_all_segs(buf);	
    else if (*buf == ';')
      continue;
    else if (*buf == 'B')
      save_patch_rule(fr, buf);
    else
      fprintf(stderr, "%s: invalid bihx line.\n", name, buf);
  }
  fclose(fp);
  fclose(fr);
}

int main(int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "%s bihxfile\n", argv[0]);
    exit(1);
  }
  split_bihx(argv[1]);
  close_segs();
  bin_segs();
}

      
      