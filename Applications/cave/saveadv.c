
    /* SAVEADV.C  no mods for V 1.43 
     *
     * Modified calls of fcreat() to call fopen() with a mode setting of write.
     * Modified close() call to use fclose() and removed call to fflush().
     * Cahnged "savefd" from and int to pointer to FILE. -- Eco-C88 V2.72 -- BW
     */

#include "advent.h"
void saveadv(void)
{
	int savefd;
	char username[64];
	writes("What do you want to call the saved game? ");
	getinp(username, 64);
	if (*username == 0)
		return;
	writes("Opening save file \"");
	writes(username);
	writes("\".\n");
	if ((savefd = open(username, O_WRONLY | O_CREAT | O_TRUNC, 0600)) == -1) {
		writes("Sorry, I can't open the file...\n");
		return;
	}
	if (write(savefd, &game, sizeof(game)) != sizeof(game)) {
		writes("write error on save file...\n");
		return;
	}
	if (close(savefd) == -1) {
		writes("can't close save file...\n");
		return;
	}
	writes("Game saved.\n\n");
}
