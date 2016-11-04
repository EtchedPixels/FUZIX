/* passwd.c
 * description:  change user password
 * author:	 Shane Kerr <kerr@wizard.net>
 * copyright:	 1998 by Shane Kerr <kerr@wizard.net>, under terms of GPL
 */  
    
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>
#include <time.h>
    
/* maximum allowed errors */ 
#define MAX_FAILURE 5
    
/* valid characters for a salt value */ 
static char salt_char[] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

char nbuf1[128], nbuf2[128];

int time_zone = 0;

int main(int argc, char *argv[])
{
    char *s, *pbuf, *tmp_fname;
    const char *pwf = "/etc/passwd";
    char salt[3];
    int  n, failure_cnt, uid, gid;
    struct passwd *pwd;
    FILE * passwd_fp;
    time_t now;

    umask(022);

    switch (argc) {
    case 1:
	if ((pwd = getpwuid(getuid())) == NULL) {
	    fprintf(stderr, "%s: error reading password file\n", argv[0]);
	    return 1;
	}
	break;

    case 2:
	if ((pwd = getpwnam(argv[1])) == NULL) {
	    fprintf(stderr, "%s: unknown user %s\n", argv[0], argv[1]);
	    return 1;
	}
	if ((getuid() != 0) && (getuid() != pwd->pw_uid)) {
	    fprintf(stderr, "You may not change the password for %s.\n",
	            argv[1]);
	    return 1;
	}
	break;

    default:
	fprintf(stderr, "usage: %s [name]\n", argv[0]);
	return 1;
    }

    printf("Changing password for %s\n", pwd->pw_name);
    if ((getuid() != 0) && (pwd->pw_passwd[0] != 0)) {
	pbuf = getpass("Old password: ");
	salt[0] = pwd->pw_passwd[0];
	salt[1] = pwd->pw_passwd[1];
	salt[2] = 0;
	if (strcmp(crypt(pbuf, salt), pwd->pw_passwd)) {
	    fprintf(stderr, "Incorrect password for %s\n", pwd->pw_name);
	    return 1;
	}
	memset(pbuf, 0, strlen(pbuf));
    }
    failure_cnt = 0;
    for (;;) {
	pbuf = getpass("New password: ");
	strcpy(nbuf1, pbuf);
	pbuf = getpass("Re-enter new password: ");
	strcpy(nbuf2, pbuf);
	if (strcmp(nbuf1, nbuf2) == 0) {
	    /* need to create a tmp file for the new password */ 
	    if ((tmp_fname = tmpnam(NULL)) == NULL) {
		perror("Error getting temporary file name");
		return 1;
	    }
	    if ((passwd_fp = fopen(tmp_fname, "w")) == NULL) {
		perror(tmp_fname);
		return 1;
	    }
	    
	    /* save off our UID */ 
	    uid = pwd->pw_uid;
	    gid = pwd->pw_gid;
	    time(&now);
	    salt[0] = salt_char[(now) & 0x3F];
	    salt[1] = salt_char[(now >> 6) & 0x3F];
	    salt[2] = 0;
	    s = crypt(nbuf1, salt);
	    
	    /* save entries to the new file */ 
	    setpwent();
	    for (n = 0; (pwd = getpwent()) != NULL; ++n) {
		if (pwd->pw_uid == uid && pwd->pw_gid == gid)
		    pwd->pw_passwd = s;
		if (putpwent(pwd, passwd_fp) == -1) {
		    perror("Error writing password file");
		    return 1;
		}
	    }
	    endpwent();
	    
	    /* update the file and move it into position */ 
	    fclose(passwd_fp);
	    if (n < 1) {
		fprintf(stderr, "Can't rewrite %s\n", pwf);
	    }
	    strcpy(nbuf2, pwf);
	    strcat(nbuf2, "~");
	    unlink(nbuf2);
	    if (rename(pwf, nbuf2) < 0) {
		perror("Error renaming old password file");
		return 1;
	    }
	    if (rename(tmp_fname, pwf) < 0) {
		perror("Error installing new password file");
		return 1;
	    }
	    
	    /* celebrate our success */ 
	    printf("Password changed.\n");
	    return 0;
	}
	if (++failure_cnt >= MAX_FAILURE) {
	    fprintf(stderr, "The password for %s is unchanged.\n",
		    pwd->pw_name);
	    return 1;
	}
	fprintf(stderr, "They don't match; try again.\n");
    }
    return 0;
}
