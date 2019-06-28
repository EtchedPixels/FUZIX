#define _FUZIX_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mntent.h>
#include <sys/mount.h>

static int err = 0;
static int all = 0;
static char *fstype = "fuzix";

static void usage(void)
{
    fputs("Usage: mount [-a] [-t type] name [path].\n", stderr);
    exit(1);
}

static int lsmtab(void)
{
    FILE *f;
    struct mntent *mnt;
    
    f = setmntent("/etc/mtab", "r");
    if (f) {
        while (mnt = getmntent(f)) {
            if (strcmp(mnt->mnt_type, "swap") == 0)
                printf("%s is swapspace\n", mnt->mnt_fsname);
            else
                printf("%s mounted on %s read-%s\n",
                    mnt->mnt_fsname,
                    mnt->mnt_dir,
                    hasmntopt(mnt, "ro") ? "only" : "write");
        }
        endmntent(f);
    } else
        err = 1;
    
    return 0;
}

static void add2mtab(struct mntent *mnt)
{
    FILE *f;

    f = setmntent("/etc/mtab", "a");
    if (f == NULL) {
        perror("/etc/mtab");
        exit(1);
    }
    addmntent(f, mnt);
    endmntent(f);
}

static int flagsof(struct mntent *mnt)
{
    int f = 0;
    if (hasmntopt(mnt, "ro"))
        f |= MS_RDONLY;
    if (hasmntopt(mnt, "nosuid"))
        f |= MS_NOSUID;
    return f;
}

static int is_mounted(const char *path)
{
    FILE *f = setmntent("/etc/mtab", "r");
    struct mntent mnt;
    static char buf[_MAX_MNTLEN];
    
    while(getmntent_r(f, &mnt, buf, _MAX_MNTLEN)) {
        if (strcmp(mnt.mnt_fsname, path) == 0) {
            endmntent(f);
            return 1;
        }
    }
    endmntent(f);
    return 0;
}

static void do_mount(struct mntent *mnt)
{
    const char *p;
    if (strcmp(mnt->mnt_type, "swap") == 0)
        return;
    p = mnt_device_path(mnt);
    if (mount(p, mnt->mnt_dir, flagsof(mnt)) == -1) {
        err = errno;
        perror(p);
        return;
    }
    add2mtab(mnt);
}

static void automount(char *match)
{
    FILE *f = setmntent("/etc/fstab", "r");
    const char *p;
    struct mntent *mnt;

    while (mnt = getmntent(f)) {
        p = mnt_device_path(mnt);
        /* Warning - mnt contents go invalid if we do work on mtab so
           be careful here and use getmntent_r in is_mounted */
        if (is_mounted(p))
            continue;
        if (match == NULL || strcmp(p, match) == 0 ||
                                strcmp(mnt->mnt_dir, match) == 0)
                do_mount(mnt);
        if (err)
            break;
    }
    endmntent(f);
}

int main(int argc, char *argv[])
{
    int opt;

    while((opt = getopt(argc, argv, "at:")) != -1) {
        if (opt == 'a')
            all = 1;
        else if (opt == 't')
            fstype = optarg;
        else
            usage();
    }

    if (optind >= argc) {
        if (all)
            automount(NULL);
        else
            lsmtab();
        return err;
    }
    if (optind == argc - 1)
        automount(argv[optind]);
    else if (optind <= argc - 2) {
        static struct mntent mnt;
        mnt.mnt_fsname = argv[optind++];
        mnt.mnt_dir = argv[optind++];
        mnt.mnt_type = fstype;
        if (optind != argc)
            mnt.mnt_opts = argv[optind++];
        else
            mnt.mnt_opts = "rw";
        if (optind != argc)
            usage();
        do_mount(&mnt);
    }
    return err;
}
