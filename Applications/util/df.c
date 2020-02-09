#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <mntent.h>

const char *devname(dev_t);
const char *mntpoint(const char *);
void df_path(const char *path, const char *dpath, const char *mntpath);
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

void df_path(const char *path, const char *dpath, const char *mpath)
{
    static struct statvfs vfs;
    static struct stat st;
    /* We do not currently need the extra bits here but will in future with
       any kind of block extent scaling */
    unsigned long total, used, unused;
    unsigned int percent;

    if (statvfs(path, &vfs) < 0 || stat(path, &st) < 0) {
        fprintf(stderr, "df: \"%s\": %s\n", path, strerror(errno));
        return;
    }

    if (dpath == NULL)
        dpath = devname(st.st_dev);
    if (mpath == NULL)
        mpath = mntpoint(dpath);

    if (iflag) {
	/* inodes */
	total = vfs.f_files;
	unused  = vfs.f_favail;
    } else {
	/* blocks in 512 byte counts */
	total = vfs.f_blocks * (vfs.f_bsize / 512);
	unused  = vfs.f_bavail * (vfs.f_bsize / 512);
    }
    used  = total - unused;

    /* Are we working in Kbytes or blocks ? */
    if (!iflag && kflag) {
        total /= 2;
        used /= 2;
        unused /= 2;
    }

    percent = total / 100;

    if (fflag) {
	if(percent)
	    percent = unused / percent;
    } else {
	if(percent)
	    percent = used / percent;
    }

    printf("%-16s %6lu %6lu %6lu %5u%% %s\n",
            dpath, total, used, unused, percent, mpath);
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
