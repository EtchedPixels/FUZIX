R: dosmenu.p - A simple DOS menu program
 : Written by Rob Linwood (auntfloyd@biosys.net)

R: We put this label here so we can jump back to it later
*menu

R: Print out the menu
T: ÿÿÿÿÿDOS Menu
 : ==================
 : 1. List Files
 : 2. Edit some Files
 : 3. Play Ultima VI
 : 4. Find a file
 : 5. Delete a file
 : 6. Rename a file
 : 7. Copy a file
 : 8. Move a file
 : 9. Quit
 : 
 : What to do? (Enter 1, 2, 3, 4, 5, 6, 7, 8, or 9)

R: Get the user's response
A: #what

R: Jump to the specified labels

J(#what = 1): list
J(#what = 2): edit
J(#what = 3): play
J(#what = 4): find
J(#what = 5): delete
J(#what = 6): rename
J(#what = 7): copy
J(#what = 8): move
J(#what = 9): quit

R: If nothing above works, print an error message
T:
 : I don't understand!
 :
J: menu

*list
R: First off, list the files
S: dir
R: then return
J: menu

*edit
T: Edit which File?
A: $file

R: Here we make $command equal to "edit" plus whatever is in $files
C: $command = edit $file

T: $command

R: Run the newly-constructed command
S: $command
J: menu

*play
S: C:\ULTIMA\U6\ULTIMA6
J: menu

*find
R: Requires "Whereis" of some sort
 : This is just like what happens in "edit"

T: Find which file?
A: $file
C: $command = whereis $file
S: $command
J: menu

*delete
T: Delete which file?
A: $file
C: $command = del $file
S: $command
J: menu

*rename
T: Rename which file?
A: $file
T: To what?
A: $newname
C: $command = ren $file $newname
S: $command
J: menu

*copy
T: Copy which file?
A: $file
T: To where?
A: $newname
C: $command = copy $file $newname
S: $command
J: menu

*move
T: Move which file?
A: $file
T: To where?
A: $newname
C: $command = move $file $newname
S: $command
J: menu

*quit
E:

