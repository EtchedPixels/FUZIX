/* This little wrapper seems needed when linking with libgcc */

int putchar(int ch)
{
        unsigned char c = ch;
        return write(1, &c, 1);
}
