#!/bin/bash

for a in PASSPHRASE_ECHO PASSPHRASE_STAR PASSPHRASE_TEXT ""; do
    for b in PASSPHRASE_REALLOC ""; do
	for c in PASSPHRASE_MOVE ""; do
	    for d in PASSPHRASE_INSERT ""; do
		for e in PASSPHRASE_OVERRIDE ""; do
		    for f in PASSPHRASE_DELETE ""; do
			for g in PASSPHRASE_CONTROL ""; do
			    for h in PASSPHRASE_DEDICATED ""; do
				for i in DEFAULT_INSERT ""; do
				    make libpassphrase -B OPTIONS="$a $b $c $d $e $f $g $h $i" || exit 1
				done
			    done
			done
		    done
		done
	    done
	done
    done
done

