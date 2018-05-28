/*
A C version of "cowsay"

Copyright (C) 2012 Federico Pomar & Federico Bond
https://github.com/federicobond/fedux

Lightly modified for Fuzix by Brett Gordon


*/

#include <stdio.h>
#include <string.h>

#define MULTILINE_WARNING   "Blah blah blah, try something shorter."
#define BLANK_WARNING       "Nothing to say."

typedef struct {
    const char *opt;
    const char *eyes;
    const char tongue;
} design_t;

static const design_t designs[] = {
    { "-b", "==", ' ' },
    { "-d", "XX", ' ' },
    { "-g", "$$", ' ' },
    { "-p", "@@", ' ' },
    { "-s", "**", 'U' },
    { "-t", "--", ' ' },
    { "-w", "OO", ' ' },
    { "-y", "..", ' ' },
    { NULL, "\0\0", 0 }
};

static const design_t defdesign =  { NULL, "oo", ' ' };

static int args_shift(int n, int *argc, char ***argv)
{
    if (*argc >= n)
    {
        *argc -= n;
        *argv += n;
    }
    return *argc - n;
}

int
cowsay_get_length(int argc, char **argv)
{
    int i;
    int sum = 0;

    for (i = 0; i < argc; i++)
        sum += strlen(argv[i]);
    sum += argc - 1;
    return sum;
}

void
cowsay_print_singleline(int argc, char **argv, int length)
{
    int i;
    putchar(' ');
    for (i = 0; i < length + 2; i++)
        putchar('_');
    printf("\n< ");

    for (i = 0; i < argc; i++)
    {
        printf("%s", argv[i]);
        putchar(' ');
    }
    printf(">\n ");
    for (i = 0; i < length + 2; i++)
        putchar('-');
    putchar('\n');
}

void
cowsay_print_string(char *str)
{
    int i;
    int length = strlen(str);
    putchar(' ');
    for (i = 0; i < length + 2; i++)
        putchar('_');
    printf("\n< ");

    printf("%s", str);
    putchar(' ');

    printf(">\n ");
    for (i = 0; i < length + 2; i++)
        putchar('-');
    putchar('\n');
}

void
cowsay_print_multiline(int argc, char **argv)
{
    /* TODO: Add multiline support */
    cowsay_print_string(MULTILINE_WARNING);
}

void
cowsay_print_bubble(int argc, char **argv)
{
    int length = cowsay_get_length(argc, argv);
    if (length < 40)
        cowsay_print_singleline(argc, argv, length);
    else
        cowsay_print_multiline(argc, argv);
}

void
cowsay_print_cow(const design_t *design)
{
    printf("        \\   ^__^\n"
           "         \\  (%s)\\_______\n"
           "            (__)\\       )\\/\\\n"
           "             %c  ||----w |\n"
           "                ||     ||\n",
           design->eyes, design->tongue);
}

int
main(int argc, char **argv)
{
    int i;
    const design_t *current = &defdesign;

    args_shift(1, &argc, &argv);

    if (argc == 1 && strcmp(argv[0], "--version") == 0)
    {
        printf("cowsay 1.0 (Fedux release 1.0)\n");
        printf("Copyright (C) 2012 Federico Pomar & Federico Bond\n");
        return 0;
    }

    if (argc == 1 && strcmp(argv[0], "--help") == 0)
    {
        /* Texts stolen from the Wikipedia article on cowsay: */
        /* http://en.wikipedia.org/wiki/Cowsay */
        printf("Usage: cowsay [OPTION] MESSAGE\n"
               "Prints a cute cow with the message in a speech bubble.\n\n"
               "Available designs:\n"
               "  -b    \"Borg\" mode, uses == in place of oo for the cow eyes.\n"
               "  -d    \"Dead\", uses XX.\n"
               "  -g    \"Greedy\", uses $$.\n"
               "  -p    \"Paranoid\", uses @@.\n"
               "  -s    \"Stoned\", uses ** to represent bloodshot eyes, plus a descending U to\n"
               "        represent an extruded tongue.\n"
               "  -t    \"Tired\", uses --.\n"
               "  -w    \"Wired\", uses OO.\n"
               "  -y    \"Youthful\", uses .. to represent smaller eyes.\n\n"
               "General help available by running 'help' command.\n");
        return 0;
    }

    for (i = 0; designs[i].opt != NULL; i++)
    {
        if (strcmp(argv[0], designs[i].opt) == 0)
        {
            current = designs + i;
            args_shift(1, &argc, &argv);
        }
    }

    if (argc > 0)
        cowsay_print_bubble(argc, argv);
    else
        cowsay_print_string(BLANK_WARNING);

    cowsay_print_cow(current);

    return 0;
}
