R: jump-use.p - An example of J and U
 : Written by Rob Linwood (auntfloyd@biosys.net)

T: We are about to jump far away to label1
J: label1
T: This text is never seen!  If you can see this, you've got problems!
E:

*label1

T: Now, we're going to call the subroutine "Woof"
U: woof
T: We're back from the subroutine!
E:

*woof
T: Now we're in "Woof"
E:








