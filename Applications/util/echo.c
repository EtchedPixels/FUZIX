/* echo command */

#include <stdio.h>
#include <string.h>

int main(int argc, const char *argv[])
{
    register const char *p;
    char c;
    int  i, nflag, eflag;

    nflag = eflag = 0;
    p = *(++argv);

    while (p && *p == '-') {
        c = *(++p);
	if (c == 'n')
	    nflag = 1;
	else if (c == 'e')
	    eflag = 1;
	else
	    break;
	if (*(++p)) break;
	p = *(++argv);
    }
    
    while ((p = *argv++) != NULL) {
	while ((c = *p++) != '\0') {
	    if (c == '\\' && eflag) {
	        switch (*p++) {
	        case '0':  c = 0;
	        	   for (i = 0; i < 3 && *p >= '0' && *p <= '7'; ++i)
	        	       c = (c << 3) + *p++ - '0';
	        	   break;
		case 'b':  c = '\b'; break;
		case 'c':  return 0;
		case 'f':  c = '\f'; break;
		case 'n':  c = '\n'; break;
		case 'r':  c = '\r'; break;
		case 't':  c = '\t'; break;
		case 'v':  c = '\v'; break;
		case '\\': break;
		default:   --p; break;
		}
	    }
	    putchar(c);
	}
	if (*argv) putchar(' ');
    }
    if (!nflag) putchar('\n');
    
    return 0;
}
