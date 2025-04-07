# 2048 Terminal Game

This repo adapts Frost-Phoenix's implementation of the 2048 game for Linux
terminal to build and run under Fuzix using termcap.  By default it will use
the colour extensions for vt52 or xterm.

Changes are:

- Replace screen codes with equivalents from termcap
- Reformat default layout for 24 line terminals
- Small display option (-s) fits everything in 32x16
- No special graphics characters, just ASCII art boxes
- Empty cells are white
- Accept WASD, HJKL and cursors
- Fix: reverts old score when taking back a move
- Mono option (-m) and help text (-h)

With sundry other compatibility changes.

In addition, only changed tiles are redrawn.  This is hardly necessary under
Linux, but if you were running over a slow serial terminal, you'd really notice
the change.

Tested on the CoCo 3 platform.

Selected excerpts from Frost-Phoenix's original README follow.


## Table of Contents

- [Features](#features)
- [How to Play](#how-to-play)
- [Credits](#credits)
- [License](#license)


## Features

- Classic 2048 gameplay in the terminal.
- Responsive arrow key controls.
- Score tracking.
- Ability to undo one move.


## How to Play

The goal of the game is to combine number tiles by moving them in different
directions using arrow keys, with the objective of reaching the tile with the
value of 2048. When two tiles with the same value collide during a move, they
merge into a new tile with the sum of their values. After each move, a new tile
(either 2 or 4) will appear in an empty spot on the board. The game continues
until there are no empty spots left on the board, and the player can no longer
make valid moves, resulting in the game coming to an end.


## Credits

This project is inspired by and incorporates ideas from the following project:

- [mevdschee/2048.c](https://github.com/mevdschee/2048.c)
- [alewmoose/2048-in-terminal](https://github.com/alewmoose/2048-in-terminal)


## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE)
file for details.
