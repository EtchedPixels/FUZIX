#!/bin/sh
set -e

realld="$1"
shift

args=
for a in "$@"; do
	case "$a" in
		-g) ;;
		--relax) ;;
		--start-group) ;;
		--end-group) ;;
		--gc-sections) ;;

		*)
			args="$args $a"
			;;
	esac
done

exec "$realld" "$args"

