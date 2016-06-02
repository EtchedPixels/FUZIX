R: recures.p - an example of a recursive PILOT program
 : Written by Rob Linwood (auntfloyd@biosys.net)

R: Ask a question

T: Would you like to recurse again? (Y/N)

R: Get the answer and put it in $answer,
 : because no variable name was given

A:

R: Try to match any of the strings listed after the "M:"

M: y yes sure yep

R: If any of them matched, then #matched was set to 1
 : Next we Jump to "*recurse" if #matched equals 1
 
JY: recurse

R: If nothing matched, we end the program

E:

R: Define a label called "recurse"

*recurse

R: Call rpilot again, and start running this file again
 : Note: whether this will work or not depends on where the `recurse.p' file
 : is located.  We'll check in two locations.

S: rpilot examples/recurse.p
J( #retcode = 0 ): done
S: rpilot recurse.p
J( #retcode = 0 ): done

T: Sorry, I can't run the program! Return code: #retcode

*done
