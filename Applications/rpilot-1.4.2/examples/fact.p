R: fact.p - A recursive PILOT program for solving factorials
 : Written by Rob Linwood (auntfloyd@biosys.net)

R: A factorial of number n is written as n!  It is the equal to the expression
 : n! = n * (n-1) * (n-2) * (n - 3) * ... * (n - (n-1))
 : For example, 4! = 4 * 3 * 2 * 1 = 24.  The way we solve this in PILOT is to
 : use a programming technique called "recursion".  In a recursive program,
 : one part of the program (generally a subroutine) calls itself.  This a 
 : powerful technique which is crucial in handling numerical sequences.  Note
 : that this is a simple example of recursion, but a useful one.
 
T: This program can generate factorials of a number (n!) which you give it.
 : Because the value of n! rises very rapidly with respect to n, RPilot will
 : not be able to handle the result of n! for values of n larger than a
 : certain number.  On 32-bit systems, this number is 16.
 :
 : So please give me a number.
A: #num

R: If #num is 0, 1, or 2, we don't need to solve.  Instead, we go straight to
 : the end.
 : These are "special cases" which can be handled more quickly than others
J( #num = 0 ): zero
J( #num = 1): one
J( #num = 2): two

R: Initialize the result (#res) to 0
C: #res = #num

*bang
J( #num = 1 ): done
C: #tmp = #num - 1
C: #res = #tmp * #res
C: #num = #num -1
J: bang

*zero
T: The answer is Zero
E:

*one
T: The answer is One
E:

*two
T: The answer is Two
E:

*done
T: The answer is #res


