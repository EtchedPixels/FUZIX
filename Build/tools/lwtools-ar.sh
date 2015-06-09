#!/bin/sh
set -e

realar="$1"
command="$2"
arfile="$3"
shift
shift
shift

if [ "$command" = "-rc" ]; then
	tmpdir=/tmp/lwtools-ar.$$
	mkdir $tmpdir
	trap "rm -rf $tmpdir" EXIT
	cp "$@" $tmpdir
	(cd $tmpdir && "$realar" -rc out.a *.o)
	cp $tmpdir/out.a "$arfile"
else
	"$realar" "$command" "$arfile" "$@"
fi

