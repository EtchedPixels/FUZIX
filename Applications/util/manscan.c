/*
 *	Extract markdown man pages from source code
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

static char buf[512];

static char *makeupper(char *buf)
{
    char *p = buf;
    while(*p) {
        if (islower(*p))
            *p = toupper(*p);
        p++;
    }
    return buf;
}

static void extract_content(FILE *fp,  const char *p)
{
    char *ns = buf + 3;
    char *name;
    FILE *fo;
    
    while(*ns && isspace(*ns))
        ns++;
    name = ns;
    
    while(*name && *name != '(') {
        if (!isalnum(*name) && *name != '_')
            goto bad;
        name++;
    }
    if (*name != '(' || name[2] != ')' || !isalnum(name[1]))
        goto bad;
    *name = '.';
    name[2] = 0;
    
    fo = fopen(ns, "w");
    if (fo == NULL) {
        perror(ns);
        exit(1);
    }
    fprintf(fo, "%s\n", makeupper(ns));
    while(fgets(buf, 512, fp)) {
        if (strncmp(buf, "|*/", 3) == 0) {
            fclose(fo);
            return;
        }
        fputs(buf, fo);
    }
    fprintf(stderr, "%s: unexpected EOF\n", p);
    exit(1);
bad:
    fprintf(stderr, "%s: invalid manual name: %s", p, buf);
    exit(1);
}

static void scan_file(FILE *fp, const char *p)
{
    while(fgets(buf, 512, fp)) {
        if (strncmp(buf, "/*|", 3) == 0)
            extract_content(fp, p);
    }
    fclose(fp);
}
    

int main(int argc, char *argv[])
{
    char *n;
    argv++;
    if (*argv == NULL)
        scan_file(stdin, "<stdin>");
    else while((n = *argv++) != NULL) {
        FILE *fp = fopen(n, "r");
        if (fp == NULL) {
            perror(n);
            exit(1);
        }
        scan_file(fp, n);
        fclose(fp);
    }
    exit(0);
}