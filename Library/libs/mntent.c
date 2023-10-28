/*
 *	An implementation of the mountpoint handling library. This one
 *	handles all the modern quoting rules for spaces in mount entries
 *	but does not currently implement the delmntent() extension as we
 *	can't do that easily without ftruncate().
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/file.h>
#include <mntent.h>

static char mntbuf[_MAX_MNTLEN];

FILE *setmntent(char *filep, char *type)
{
	int lock;
	FILE *fp = fopen(filep, type);
	
	if (fp == NULL)
		return NULL;
	if (strchr(type,'w') || strchr(type,'a'))
		lock = LOCK_EX;
	else
		lock = LOCK_SH;
	if (flock(fileno(fp), lock) == -1) {
		fclose(fp);
		return NULL;
	}
	return fp;
}

static int isoct(char c)
{
	if (c >= '0' && c <= '7')
		return 1;
	return 0;
}

static int mntparse(char *p, char **t, char *def)
{
	char *d = strtok(p, " \t\n");
	char *ds = d;
	if (d == NULL)
		d = def;
	else {
		/* Dequote */
		char *s = d;
		while (*s) {
			if (*s == '\\' && isoct(s[1]) && isoct(s[2]) && isoct(s[3])) {
				*d++ = ((s[1] - '0') << 6) | ((s[2] - '0') << 3) | (s[3] - '0');
				s += 4;
			} else
				*d++ = *s++;
		}
	}
	if (t)
		*t = ds;
	return atoi(ds);
}

struct mntent *getmntent_r(FILE * fp, struct mntent *me, char *buf, int len)
{
	char *p = buf;
	/* Skip blank lines and comments */
	do {
		if (fgets(buf, len, fp) == NULL)
			return NULL;
	} while(*p == '\n' || *p == '#');
	mntparse(buf, &me->mnt_fsname, NULL);
	mntparse(NULL, &me->mnt_dir, NULL);
	mntparse(NULL, &me->mnt_type, "fuzix");
	mntparse(NULL, &me->mnt_opts, "");
	me->mnt_freq = mntparse(NULL, NULL, "0");
	me->mnt_passno = mntparse(NULL, NULL, "0");
	if (me->mnt_fsname == NULL || me->mnt_dir == NULL)
		return NULL;
	return me;
}

struct mntent *getmntent(FILE * fp)
{
	static struct mntent me;
	return getmntent_r(fp, &me, mntbuf, _MAX_MNTLEN);
}

static char *quote_out(char *t, const char *s)
{
	if (t == NULL)
		return NULL;
	while (t < mntbuf + _MAX_MNTLEN - 1) {
		if (*s == 0) {
			*t++ = ' ';
//			fprintf(stderr, "[%x]", t);
			return t;
		}
		if (*s == ' ' || *s == '\n' || *s == '\t' || *s == '\\') {
			if (t < mntbuf + _MAX_MNTLEN - 5) {
				sprintf(t, "\\%3o", *s);
				t += 4;
				s++;
			} else
				break;
		} else {
//			fprintf(stderr, "%02x", *s);
			*t++ = *s++;
		}
	}
	/* Overrun */
	return NULL;
}

static char *quote_out_int(char *t, int s)
{
	return quote_out(t, _itoa(s));
}

int addmntent(FILE * fp, struct mntent *mnt)
{
	char *p = mntbuf;
	p = quote_out(p, mnt->mnt_fsname);
	p = quote_out(p, mnt->mnt_dir);
	p = quote_out(p, mnt->mnt_type);
	p = quote_out(p, mnt->mnt_opts);
	p = quote_out_int(p, mnt->mnt_freq);
	p = quote_out_int(p, mnt->mnt_passno);
	if (p) {
		p[-1] = '\n';
		if (fwrite(mntbuf, p - mntbuf, 1, fp) == 1)
			return 0;
	}
	return 1;
}

int endmntent(FILE *fp)
{
	/* Will automatically drop the flock */
	return fclose(fp);
}

char *hasmntopt(struct mntent *mnt, char *opt)
{
	char *p = mnt->mnt_opts;
	ssize_t o = strlen(opt);

	while (p) {
		if (memcmp(p, opt, o) == 0 && (p[o] == ',' || isspace(p[o]) || p[o] == 0 || p[o] == '='))
			return p;
		p = strchr(p, ',');
		if (p)
			p++;
	}
	return NULL;
}
