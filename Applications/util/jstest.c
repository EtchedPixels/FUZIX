#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/input.h>

int main(int argc, char *argv[])
{
    int fd = open("/dev/input", O_RDONLY);
    char buf[2];
    int l;

    if (fd == -1) {
        perror("/dev/input");
        exit(1);
    }
    while(1) {
        l = read(fd, buf, 2);
        if (l != 2)
            continue;
        if ((buf[0] & 0xF0) != STICK_DIGITAL)
            continue;
        printf("Joystick %d: %c%c%c%c%c%c%c%c\n",
            buf[0] & 0x0F,
            (buf[1]&STICK_DIGITAL_U)?'U':'-',
            (buf[1]&STICK_DIGITAL_D)?'D':'-',
            (buf[1]&STICK_DIGITAL_L)?'L':'-',
            (buf[1]&STICK_DIGITAL_R)?'R':'-',
            (buf[1]&BUTTON(0))?'0':'-',
            (buf[1]&BUTTON(1))?'1':'-',
            (buf[1]&BUTTON(2))?'2':'-',
            (buf[1]&BUTTON(3))?'3':'-');
        fflush(stdout);
    }
}
