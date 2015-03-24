#!/bin/bash

for a in PASSPHRASE_ECHO ""; do
    for b in PASSPHRASE_STAR ""; do
	for c in PASSPHRASE_TEXT ""; do
	    for d in PASSPHRASE_REALLOC ""; do
		for e in PASSPHRASE_MOVE ""; do
		    for f in PASSPHRASE_INSERT ""; do
			for g in PASSPHRASE_OVERRIDE ""; do
			    for h in PASSPHRASE_DELETE ""; do
				for i in PASSPHRASE_CONTROL ""; do
				    for j in PASSPHRASE_DEDICATED ""; do
					for k in DEFAULT_INSERT ""; do
					    for l in PASSPHRASE_INVALID ""; do
						make libpassphrase -B OPTIONS="$a $b $c $d $e $f $g $h $i $j $k $l" || exit 1
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
    done
done

