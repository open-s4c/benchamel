#!/bin/sh

CNT=1
CNT_THRESHOLD=$1

while [ $CNT -ne $CNT_THRESHOLD ]; do
	echo "Console0: iteration $CNT"
	
	wget -q -O con0.out http://127.0.0.1/index.html

	CNT=`expr $CNT + 1`
done
