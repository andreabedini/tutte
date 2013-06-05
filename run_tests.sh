#!/bin/bash

if ! [ -f "$1" ] || ! [ -x "$1" ]; then
    echo "Please supply the executable to test as an argument."
    exit -1
fi

failed=0
for f in tests/*.input; do
	$1 --input-file $f 2>/dev/null | diff -w - ${f%.input}.output
	ret_code=$?
	if [ $ret_code == 0 ]; then
		echo "$f PASS"
	else
		echo "$f FAIL"
		failed=1
	fi
done

exit $failed
