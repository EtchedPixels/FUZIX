/*
 *	Simple map convertor for the maps we care about
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "sok.h"

static int level;
static uint8_t x, y;
static uint8_t height, width;

static int map_fd;

static struct level map;

void map_line(const char *p)
{
    uint8_t x = 0;
    while(*p) {
        switch(*p) {
        case '*':
            map.done++;
        case '$':
            map.blocks++;
        case '#':
        case '.':
        case ' ':
            map.map[y][x++] = *p;
            break;
        case '+':
            map.py = y;
            map.px = x;
            map.map[y][x++] = '.';
            break;
        case '@':
            map.py = y;
            map.px = x;
            map.map[y][x++] = ' ';
            break;
        default:
            if (level == 0)
                map.map[y][x++] = *p;
            else {
                fprintf(stderr, "Bad map line at '%s'.\n",  p);
                exit(1);
            }
            break;
        }
        p++;
        if (x > width)
            width = x;
    }
    height++;
    y++;
}

static void next_map(void)
{
    /* Reset */
    memset(&map, 0, sizeof(map));
    memset(&map.map, ' ', sizeof(map.map));
    x = 0;
    y = 0;
    width = 0;
    height = 0;
}

static void draw_check(void)
{
    int i;
    for (i = 0; i < MAP_H; i++) {
        printf("| ");
        fwrite(map.map[i], MAP_W, 1, stdout);
        printf(" |\n");
    }
}

static void level_done(void)
{
    uint8_t left = (MAP_W - width) / 2;
    uint8_t top = (MAP_H - height) / 2;
    uint8_t i;
    /* Centre the map */
    for (i = 0; i < height; i++) {
        memmove(map.map[i] + left, map.map[i], MAP_W - left);
        memset(map.map[i], ' ', left);
    }
    /* And vertically */
    memmove(((uint8_t *)map.map) + top * MAP_W,
        map.map, height * MAP_W);
    memset((uint8_t *)map.map, ' ', top * MAP_W);
    map.py += top;
    map.px += left;
    /* Write it out */
    if (lseek(map_fd, level * MAP_SEEK, SEEK_SET) < 0) {
        perror("lseek");
        exit(1);
    }
    if (write(map_fd, &map, sizeof(map)) != sizeof(map)) {
        perror("write");
        exit(1);
    }
    draw_check();
    level++;
    next_map();
}

static void process(FILE *fp)
{
    char buf[1024];
    int in_level = 0;

    while(fgets(buf, 1024, fp)) {
        char *p = strchr(buf, '\n');
        if (p == NULL) {
            fprintf(stderr, "Invalid line '%s'.\n", buf);
            exit(1);
        }
        *p = 0;
        if (in_level) {
            if (strchr(buf, '#'))
                map_line(buf);
            else {
                level_done();
                in_level = 0;
            }
        } else {
            if (strchr(buf, '#')) {
                in_level = 1;
                map_line(buf);
            }
        }
    }
    if (in_level == 1)
        level_done();
}

int main(int argc, char *argv[])
{
    FILE *fp;
    if (argc != 3) {
        fprintf(stderr, "%s infile outfile\n", argv[0]);
        exit(1);
    }
    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        perror(argv[1]);
        exit(1);
    }
    map_fd = open(argv[2], O_CREAT|O_TRUNC|O_WRONLY, 0600);
    if (map_fd == -1) {
        perror(argv[2]);
        exit(1);
    }
    next_map();
    process(fp);
    fclose(fp);
    close(map_fd);
    return 0;
}
