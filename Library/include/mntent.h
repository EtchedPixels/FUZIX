#ifndef _MNTENT_H
#define _MNTENT_H

struct mntent {
    char *mnt_fsname;
    char *mnt_dir;
    char *mnt_type;
    char *mnt_opts;
    int mnt_freq;
    int mnt_passno;
};

#define _MAX_MNTLEN	512

/* Standard functions */
extern FILE *setmntent(char *filep, char *type);
extern struct mntent *getmntent(FILE *fp);
extern int addmntent(FILE *fp, struct mntent *mnt);
extern int endmntent(FILE *fp);
extern char *hasmntopt(struct mntent *mnt, char *opt);

/* Extended function found in some Unixen */
extern int delmntent(FILE *fp, struct mntent *mnt);

#endif
