/*      Interpret CPM argument list to produce C style
        argc/argv
        default dma buffer has it, form:
        ---------------------------------
        |count|characters  ...          |
        ---------------------------------
*/
int     Xargc;
int     Xargv[30];
Xarglist(ap) char *ap; {
        char qc;
        Xargc = 0;
        ap[(*ap)+1 <tel:+1>] = '\0';
        ap++;
        while (isspace(*ap)) ap++;
        Xargv[Xargc++] = "arg0";
        if (*ap)
                do {
                        if (*ap == '\'' || *ap == '\"') {
                                qc = *ap;
                                Xargv[Xargc++] = ++ap;
                                while (*ap&&*ap != qc) ap++;
                        } else {
                                Xargv[Xargc++] = ap;
                                while (*ap&&!isspace(*ap)) ap++;
                        }
                        if (!*ap) break;
                        *ap++='\0';
                        while (isspace(*ap)) ap++;
                } while(*ap);
        Xargv[Xargc] = 0;

}

