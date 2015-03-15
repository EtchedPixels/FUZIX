#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Assumed length of a line in /etc/mtab */
#define MTAB_LINE 160

char *getdev(char *arg)
{
    FILE *f;
	char tmp[MTAB_LINE];
	char* dev;
	char* mntpt;
    
    f = fopen("/etc/mtab", "r");
    if (f) {
        while (fgets(tmp, sizeof(tmp), f)) {
			dev = strtok(tmp, " ");
			mntpt = strtok(NULL, " ");
            if ((strcmp(dev, arg) == 0) || (strcmp(mntpt, arg) == 0)) {
                fclose(f);
                return strdup(dev);
            }
        }
        fclose(f);
    }

    return NULL;
}


int rm_mtab(char *devname)
{
    FILE *inpf, *outf;
    char *tmp_fname;
	char tmp[MTAB_LINE];
	char* dev;
	char* mntpt;

    if ((tmp_fname = tmpnam(NULL)) == NULL) {
        perror("Error getting temporary file name");
        exit(1);
    }
    inpf = fopen("/etc/mtab", "r");
    if (!inpf) {
        perror("Can't open /etc/mtab");
        exit(1);
    }
    outf = fopen(tmp_fname, "w");
    if (!outf) {
        perror("Can't create temporary file");
        exit(1);
    }
    while (fgets(tmp, sizeof(tmp), inpf)) {
		dev = strtok(tmp, " ");
		mntpt = strtok(NULL, " ");
        if (strcmp(dev, devname) == 0) {
            continue;
        } else {
            fprintf(outf, "%s", tmp);
        }
    }
    fclose(inpf);
    fclose(outf);
    if (unlink("/etc/mtab") < 0) {
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
    char *dev;

    if (argc != 2) {
        printf("usage: umount device\n");
        return 1;
    }

    dev = getdev(argv[1]);
    if (!dev) dev = argv[1];

    if (umount(dev) == 0) {
        rm_mtab(dev);
    } else {
        perror("umount");
        return 1;
    }
    return 0;
}
