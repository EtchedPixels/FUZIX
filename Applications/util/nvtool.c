/*
 *	NVRAM tool
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/rtc.h>

static int rtc_fd;
static unsigned int nvmax;
static struct cmos_nvram nv;

int dump_rtc(void)
{
    unsigned int i = 0;
    for (i = 0; i < nvmax; i++) {
        if (i && !(i & 7))
            putchar('\n');
        nv.offset = i;
        if (ioctl(rtc_fd, RTCIO_NVGET, &nv) == -1) {
            perror("ioctl");
            return 1;
        }
        printf("%02X ", nv.val);
    }
    putchar('\n');
    return 0;
}

int update_rtc(char *argv[])
{
    int val;
    nv.offset = atoi(argv[1]);
    val = atoi(argv[2]);
    nv.val = val;
    if (val < 0 || val > 255 || nv.offset > nvmax) {
        fprintf(stderr, "%s: invalid range.\n", argv[0]);
        return 1;
    }
    if (ioctl(rtc_fd, RTCIO_NVSET, &nv) == -1) {
        perror("ioctl");
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    rtc_fd = open("/dev/rtc", O_RDONLY);
    if (rtc_fd == -1) {
        perror("/dev/rtc");
        return 1;
    }
    nvmax = ioctl(rtc_fd, RTCIO_NVSIZE, NULL);
    if (nvmax == -1) {
        fprintf(stderr, "%s: no NVRAM support.\n", argv[0]);
        return 1;
    }
    if (argc == 1)
        return dump_rtc();
    else if (argc == 3)
        return update_rtc(argv);
    fprintf(stderr, "%s: [reg] [val].\n", argv[0]);
    return 1;
}
