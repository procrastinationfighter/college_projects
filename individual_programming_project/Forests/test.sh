#!/bin/bash
program=$1
dir=$2
for f in $dir/*.in
do
	echo "RUNNING TEST" ${f%.in}
	testout=${f%.in}TESTED.out
	testerr=${f%.in}TESTED.err
	valgrind --leak-check=full -q ./${program} < $f >${testout} 2>${testerr}
	testresult="OK"
	if (! diff ${testout} ${f%.in}.out)
	then 
		echo "Standard output differs from test example"
		testresult="ERROR"
	fi
	if (! diff ${testerr} ${f%.in}.err)
	then
		echo "Standard error differs from test example"
		testresult="ERROR"
	fi
	echo "TEST" ${f%.in} "ENDED WITH RESULT:" ${testresult}
	rm ${testout} ${testerr}
done
