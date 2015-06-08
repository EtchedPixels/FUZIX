#!/bin/sh -
#
# This file is simply an example of what can be done using the new binary
# and symbol table output functions. It produces a byte array with the
# symbols in the object as array references within it.
#
# The output is a Linux OMAGIC binary created by ld86 -N, this means the
# object can be linked directly to C-functions created by the same GCC that
# compiled ld86.
#
# Use it in a makefile:
#
# .s86.o:
#	as86_to_data $*.s86 $*.o $(AS86FLAGS)
#

[ $# -lt 2 ] && {
   echo "Usage: `basename $0` infile outfile [as86 opts]" 1>&2
   exit 1
}

trap "rm -f _$$.* ; exit 99" 1 2 3 15 

LIBDIR='%%LIBDIR%%'	# Set by make install
[ -d "$LIBDIR" ] || LIBDIR='%%BINDIR%%'
[ -d "$LIBDIR" ] || LIBDIR=/usr/bin

IFILE="$1"
OFILE="$2"

shift ; shift
RV=0

$LIBDIR/as86 "$@" "$IFILE" -b _$$.bin -s _$$.sym || RV=$?

[ "$RV" = 0 ] && {
  (
    cat _$$.sym
    echo %%%%
    od -v -t uC _$$.bin
    echo %%%%
  ) | \
  awk > _$$.v ' BEGIN{
       startaddr="";
       printf ".text\n.data\n";
    }
    /^%%%%$/ { flg++; 
       next;
    }
    flg==0 {
       if(NF == 0) next;
       if( substr($2,1,4) == "0000" ) $2=substr($2,5);
       if( $1 == "+" && $4 == "$start" )
       {
	  if( $2 != "0000" ) startaddr=" - $" $2;
       }
       else if( substr($3, 1, 1) == "E" && $4 != "start" && $4 != "size" && $4 != "data" )
       {
          printf "export _%s\n", $4
          printf "_%s = * + $%s%s\n\n", $4, $2, startaddr;
       }
       next;
    }
    flg==1 {
        if(NF <= 1) next;
	printf "  .byte ";
	for(i=2;i<NF;i++) printf("%3d,", $i);
	                  printf("%3d", $NF);
	printf "\n";
    }
  '
  RV=$?
}

[ "$RV" = 0 ] || { rm -f _$$.* ; exit $RV ; }

# If you want to see the assembler.
# cp _$$.v `basename $2 .o`.asm

$LIBDIR/as86 -o _$$.o86 _$$.v || RV=$?

[ "$RV" = 0 ] || { rm -f _$$.* ; exit $RV ; }

$LIBDIR/ld86 -r -N _$$.o86 -o _$$.o || RV=$?

[ "$RV" = 0 ] || { rm -f _$$.* ; exit $RV ; }

if [ "X$OFILE" = "X-" ]
then cat _$$.o
else mv -f _$$.o "$OFILE" || RV=$?
fi

rm -f _$$.*
exit $RV
