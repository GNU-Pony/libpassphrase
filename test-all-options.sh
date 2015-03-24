#!/bin/bash

for a in PASSPHRASE_ECHO ""; do
    for b in PASSPHRASE_STAR PASSPHRASE_TEXT ""; do
	for c in PASSPHRASE_REALLOC ""; do
	    for d in PASSPHRASE_MOVE ""; do
		for e in PASSPHRASE_INSERT ""; do
		    for f in PASSPHRASE_OVERRIDE ""; do
			for g in PASSPHRASE_DELETE ""; do
			    for h in PASSPHRASE_CONTROL ""; do
				for i in PASSPHRASE_DEDICATED ""; do
				    for j in DEFAULT_INSERT ""; do
					make libpassphrase -B OPTIONS="$a $b $c $d $e $f $g $h $i $j $k" || exit 1
				    done
				done
			    done
			done
		    done
		done
	    done
	done
    done
done

