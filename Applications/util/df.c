#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <mntent.h>

const char *devname(dev_t);
const char *mntpoint(const char *);
void df_path(const char *path, const char *dpath, const char *mntpath);
void df_dev(dev_t dev, const char *dpath, const char *mntpath);
void df_all(void);

int iflag=0, kflag=0, fflag=0;

int main(int argc, char *argv[])
{
    char *p;
    int i;

    for (i = 1; i < argc; ++i) {
        p = argv[i];
        if (p[0] == '-') {
            for (++p; *p; ++p) {
                switch (*p) {
                case 'i': iflag = 1; break;
                case 'k': kflag = 1; break;
                case 'f': fflag = 1; break;
                default:
                    printf("usage: %s [-ikf]\n", argv[0]);
                    return 1;
                }
            }
        } else {
            break;
        }
    }

    printf("%-16s %6s %6s %6s %6s %s\n",
           "Filesystem",
            iflag ? "Inodes" : kflag ? "KBytes" : "Blocks",
            iflag ? "IUsed" : "Used",
            iflag ? "IFree" : "Free",
            iflag ? (fflag ? "%IFree" : "%IUsed") : (fflag ? "%Free" : "%Used"),
            "Mounted on");

    if (i < argc) {
        for (; i < argc; ++i) {
            df_path(argv[i], NULL, NULL);
        }
    } else {
        df_all();
    }

    return 0;
}

void df_path(const char *path, const char *dpath, const char *mntpath)
{
    struct stat sbuf;

    if(stat(path, &sbuf) < 0){
        fprintf(stderr, "df: \"%s\": %s\n", path, strerror(errno));
    }else{
        df_dev(sbuf.st_dev, dpath, mntpath);
    }
}

void df_dev(dev_t dev, const char *dpath, const char *mntpath)
{
    unsigned int Total, Used, Free, Percent;
    struct _uzifilesys fsys;
    const char *dn;

    dn = dpath ? dpath: devname(dev);

    if(_getfsys(dev, &fsys)){
        fprintf(stderr, "df: _getfsys(%d): %s\n", dev, strerror(errno));
        return;
    }

    if(iflag){
	/* inodes */
	Total = 8 * (fsys.s_isize - 2);
	Used  = Total - fsys.s_tinode;
	Free  = fsys.s_tinode;
    }else{
	/* blocks */
	Total = fsys.s_fsize;
	Used  = Total - fsys.s_isize - fsys.s_tfree;
	Free  = fsys.s_tfree;
    }

    if (!iflag && kflag) {
        Total /= 2;
        Used /= 2;
        Free /= 2;
    }

    Percent = Total / 100;

    if (fflag) {
	if(Percent)
	    Percent = Free / Percent;
    } else {
	if(Percent)
	    Percent = Used / Percent;
    }

    printf("%-16s %6u %6u %6u %5u%% %s\n",
            dn, Total, Used, Free, Percent,
            mntpath ? mntpath : mntpoint(dn));
}

void df_all(void)
{
    FILE *f;
    struct mntent *mnt;
    
    f = setmntent("/etc/mtab", "r");
    if (f) {
        while (mnt = getmntent(f))
                df_path(mnt->mnt_dir, mnt->mnt_fsname, mnt->mnt_dir);
        endmntent(f);
    } else {
        fprintf(stderr, "df: cannot open /etc/mtab: %s\n", strerror(errno));
    }
}

/* Search /dev until an entry with the specified device number is found */

#define DEV_PATH   "/dev"

const char *devname(dev_t devno)
{
    static char namebuf[sizeof(DEV_PATH) + MAXNAMLEN + 2];
    static DIR dp;

    struct dirent *entry;
    struct stat fstat;

    if (opendir_r(&dp, DEV_PATH) != (DIR *) NULL) {
        while ((entry = readdir(&dp)) != (struct dirent *) NULL) {
            sprintf(namebuf, "%s/%s", DEV_PATH, entry->d_name);
            if (stat(namebuf, &fstat) != 0)
                continue;
            if (!S_ISBLK(fstat.st_mode))
                continue;
            if (fstat.st_rdev != devno)
                continue;
            closedir_r(&dp);
            return namebuf;
        }
    }
    closedir_r(&dp);

    sprintf(namebuf, "%d", devno);
    return namebuf;
}

/* Find the mount point in /etc/mtab for the specified device */

const char *mntpoint(const char *devname)
{
    FILE *f;
    struct mntent *mnt;
    
    f = setmntent("/etc/mtab", "r");
    if (f) {
        while (mnt = getmntent(f)) {
            if (strcmp(mnt->mnt_fsname, devname) == 0) {
                endmntent(f);
                return strdup(mnt->mnt_dir);
            }
        }
        endmntent(f);
    }

    return "???";
}
