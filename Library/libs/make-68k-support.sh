#!/bin/sh
#
#	Because the Red Hat shipped one is broken
#
X=$(grep "ifdef *L_" m68k_support.S |sed -e "s/#ifdef *L_//g")
for i in $X ; do
	m68k-linux-gnu-gcc -DL_$i m68k_support.S -c -o $i.o
done
rm -f libgcc1-68000.a
for i in $X ; do
	ar rc libgcc1-68000.a $i.o
done
