R: french.p - A quiz on your skills in the French language
 : Written by Rob Linwood (auntfloyd@biosys.net)

T: Welcome to the French Quiz!
 : You will be given a word in French, and asked to pick the BEST
 : translation for it.
 :
 : Example:
 :
 :    Fromage
 :
 : 1. Cat
 : 2. Dog
 : 3. Cheese
 : 4. Mouse
 :
 : Here you would enter 1, 2, 3, or 4 depending on what you think
 : the best translation for the word "Fromage" is.  (btw, the answer is 3)
 :
 : Hit "Enter" to get started!
 :
A: $junk

R: Initialize the variables
C: #right = 0
C: #wrong = 0

R: Let's get started with the quiz...

T:
 : Q1. Porte
 :
 :  1. Ring
 :  2. Cup
 :  3. Belt
 :  4. Door
U: answer
C(#ans = 4): #right = #right + 1
C(#ans <> 4): #wrong = #wrong + 1

T:
 : Q2. Ceinture
 :
 :  1. Helmet
 :  2. Belt
 :  3. Bicycle
 :  4. Cat
U: answer
C(#ans = 2): #right = #right + 1
C(#ans <> 2): #wrong = #wrong + 1

T:
 : Q3. Cambriolage
 :
 :  1. Floor wax
 :  2. Robbery
 :  3. Bus station
 :  4. Expression
U: answer
C(#ans = 2): #right = #right + 1
C(#ans <> 2): #wrong = #wrong + 1

T:
 : Q4. Chapeau
 :
 :  1. Hat
 :  2. Chair
 :  3. Earring
 :  4. Wig
U: answer
C(#ans = 1): #right = #right + 1
C(#ans <> 1): #wrong = #wrong + 1

T:
 : Q5. Conduire
 :
 :  1. To run
 :  2. To remove
 :  3. To drive
 :  4. To type
U: answer
C(#ans = 3): #right = #right + 1
C(#ans <> 3): #wrong = #wrong + 1

T:
 : Q6. Chien
 :
 :  1. Cat
 :  2. Rabbit
 :  3. Mouse
 :  4. Dog
U: answer
C(#ans = 4): #right = #right + 1
C(#ans <> 4): #wrong = #wrong + 1

T:
 : Q7. Jeterai
 :
 :  1. Will speak
 :  2. Will run
 :  3. Will throw
 :  4. Will eat
U: answer
C(#ans = 3): #right = #right + 1
C(#ans <> 3): #wrong = #wrong + 1

T:
 : Q8. Irais
 :
 :  1. Will go
 :  2. Will become
 :  3. Will have
 :  4. Will be
U: answer
C(#ans = 1): #right = #right + 1
C(#ans <> 1): #wrong = #wrong + 1

T:
 : Q9. Zut Alors!
 :
 :  1. Shucks!
 :  2. Darn!
 :  3. Oh my!
 :  4. Sacre' Bleu!
U: answer
C: #right = #right + 1

T:
 : Q10. Parlez-vous anglais?
 :
 :  1. I am lost.
 :  2. I am a tourist.
 :  3. I am looking to be mugged.
 :  4. Do you speak English?
U: answer
C(#ans = 4): #right = #right + 1
C(#ans <> 4): #wrong = #wrong + 1
J: done

*answer
A: #ans
T(#ans > 4): Please enter a number between 1 and 4
T(#ans < 1): Please enter a number between 1 and 4
J(#ans < 1): answer
J(#ans > 4): answer
E:

*done

C: #percent = #right * 10
T: You got #right out of 10.  This is #percent percent correct.

