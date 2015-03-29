/*  cowsay for FUZIX project
    Copyright (C) 2015 Erkin Alp Güney <erkinalp9035@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include <stdio.h>
#include <getopt.h>
#include <string.h>

char cowstring_cow[]=
"  \\ ^__^\n"
"    (oo)\\_______\n"
"    (__)\\       )\\/\\\n"
"        ||----w |\n"
"        ||     ||\n";

char cowstring_fuzix[]=
"   //------x   U    U  ========7  ====T====  \\   /    \n"
"   U           U    U        //       I       \\ /     \n"
"   U=======x   U    U       //        I        X      \n"
"   U           U    U      //         I         X     \n"
"   U            \\  /     //           I       // \\    \n"
"   U             \\/    4========  ====Y====  //   \\   \n"
"                             by Alan Cox and others   \n";

extern size_t strcnt(const char* string,const char delim) {
/** strcnt-Count occurences of a given character in a string **/
    size_t i;
    for (i=0;*string;string++) if (*string==delim) i++;
    return i;
}

extern size_t strlnlen(const char* string) {
/** strlnlen-Return maximal line length in a given string **/
    size_t maxlen=0;
    for (size_t i=0;*string;string++) 
        if (*string=='\n'&&(maxlen<i)) {maxlen=i; i=0;} else i++;
    return maxlen;
}

void cowsay (char *string,size_t linelen,size_t lines) {
    putchar(' ');
    putchar('/');
    for (size_t i=0;i<linelen+2;i++) putchar('-');
    putchar('\\');
    putchar('\n');
    for (size_t i=0;i<lines;i++) {
        putchar(' ');
        putchar('|');
        for (;*string&&(*string!='\n';string++); putchar(*string);
        putchar(' ');
        putchar('|');
        putchar('\n');       
   }
   putchar(' ');
   putchar('\\');
   for (size_t i=0;i<linelen+2;i++) if (i==5) putchar('v'); else putchar('-');
   putchar('/');
   putchar('\n');
}

int main (int argc,char *argv[]) {
    unsigned short max_len=0;
    if ((argc>1)&&(getopt(argc,argv,"n")=='n') max_len=optopt;
    char *cowstring=(getopt(argc,argv,"f")=='f')?cowstring_fuzix:cowstring_cow;
    size_t message_lines=0;
    for (;optind<=argc;optind++) {
    char *arg=argv[optind];
    message_lines+=strcnt(*argv[optind],'\n');
    if (optind!=argc) arg[strlen(arg)]=' ';
    }
    cowsay(argv[0],max_len?max_len:strlnlen(argv[0]),message_lines-4);
    puts(cowstring);
}
