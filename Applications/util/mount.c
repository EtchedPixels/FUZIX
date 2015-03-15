#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Assumed length of a line in /etc/mtab */
#define MTAB_LINE 160

int lsmtab(void)
{
    FILE *f;
	char tmp[MTAB_LINE];
	char* dev;
	char* mntpt;
	char* fstype;
	char* rwflag;
    
    f = fopen("/etc/mtab", "r");
    if (f) {
        while (fgets(tmp, sizeof(tmp), f)) {
			dev = strtok(tmp, " ");
			mntpt = strtok(NULL, " ");
			fstype = strtok(NULL, " ");
			rwflag = strtok(NULL, "\n");
            if (strcmp(fstype, "swap") == 0)
                printf("%s is swapspace\n", dev);
            else
                printf("%s mounted on %s read-%s\n", dev, mntpt,
                       (strcmp(rwflag, "ro") == 0) ? "only" : "write");
        }
        fclose(f);
    }
    
    return 0;
}

int add2mtab(char *dev, char *mntpt, char *fstype, char *rwflag)
{
    FILE *inpf, *outf;
    char *tmp_fname;
	int c;

    if ((tmp_fname = tmpnam(NULL)) == NULL) {
        perror("Error getting temporary file name");
        exit(1);
    }
    inpf = fopen("/etc/mtab", "r");
    outf = fopen(tmp_fname, "w");
    if (!outf) {
        perror("Can't create temporary file");
        exit(1);
    }
    if (inpf) {
		for (;;) {
			c = fgetc(inpf);
			if (c == EOF)
				break;
			fputc(c, outf);
        }
        fclose(inpf);
    }
    fprintf(outf, "%s %s %s %s\n", dev, mntpt, fstype, rwflag);
    fclose(outf);
    if (inpf && unlink("/etc/mtab") < 0) {
        perror("Can't delete old /etc/mtab file");
        exit(1);
    }
    if (rename(tmp_fname, "/etc/mtab") < 0) {
        perror("Error installing /etc/mtab");
        exit(1);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc == 1) {
        lsmtab();
        return 0;
    }

    if (argc != 3) {
        printf("usage: mount device path\n");
        return 1;
    }

    if (mount(argv[1], argv[2], 0) == 0) {
        add2mtab(argv[1], argv[2], "uzi", "rw");
    } else {
        perror("mount");
        return 1;
    }
    return 0;
}
