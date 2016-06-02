brtR: guess.p - A simple number guessing game
 : Written by Rob Linwood (auntfloyd@biosys.net)

*start

T: I will think of a random number between 1 and 100. Can you guess it?
 :

R: Get a random number between 1 and 100 and put it in #rand
G: #rand 1 100
r:T: #rand
R: Initialize the #tries variable, which holds the number of tries
C: #tries = 0

*guess

R: Get the player's guess
T: Guess!
A: #guess

R: Increase the number of tries
C: #tries = #tries + 1

R: Jump to the proper place based on whether the player guessed the number,
 : guessed too high, or guessed too low
J(#guess = #rand): win
J(#guess > #rand): toobig
J(#guess < #rand): toosmall

*toobig
T: Too big!
 :
J: guess

*toosmall
T: Too small!
 :
J: guess

*win
T(#tries > 1): Congratulations!  You guessed it in #tries guesses!
T(#tries = 1): Congratulations!  You guessed it in 1 guess!  Lucky!
T: Play again? (Y/N)
A:
M: y yes yep sure
R: If any of the above match, (ie #matched = 1), jump to start
JY: start
E:


