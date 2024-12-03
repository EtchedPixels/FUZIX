/**
 * ascii invaders - A curses clone of the classic video game Space Invaders
 * (c) 2001 Thomas Munro
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * https://github.com/macdice/ascii-invaders
 * Thomas Munro <munro@ip9.org>
 *
 * $Id: invaders.h,v 1.3 2002/07/21 21:52:14 munro Exp $
 */

struct Bomb {
    int x;
    int y;
    int anim;
    struct Bomb *next;
};

/* Not supported in Fuzix curses */
#if 0
#define USE_COLORS 1 
#define USE_KEYS 1
#endif

/*#define BULLET_PROOF 1 *//* debug */

#define BOMB_ANIM_SIZE 4 /* "frames" in bomb anim */

#define FPS 10          /* frames per second */
#define PAINT_WAIT 2    /* how many frames between row repaints */

#define ALIEN30 3
#define ALIEN20 2
#define ALIEN10 1
#define ALIEN_EMPTY 0
#define ALIEN_EXPLODE1 -1
#define ALIEN_EXPLODE2 -2

#define ALIEN_WIDTH 6
#define ALIEN_HEIGHT 3
#define GUNNER_WIDTH 7
#define GUNNER_HEIGHT 2
#define SHELTER_WIDTH 7
#define SHELTER_HEIGHT 3
#define MA_HEIGHT 2
#define MA_WIDTH 6

#define GUNNER_ENTRANCE 40 /* how many frames before gunner appears */
#define MA_ENTRANCE 400 /* how many frames before MA comes on the screen */

#define STATE_INTRO 1
#define STATE_PLAY 2
#define STATE_EXPLODE 3
#define STATE_WAIT 4
#define STATE_GAMEOVER 5

