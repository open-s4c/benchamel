#!/bin/sh

CNT=1
CNT_THRESHOLD=$1

while [ $CNT -ne $CNT_THRESHOLD ]; do
	echo "Console1: iteration $CNT"

	wget -q -O con1.out http://127.0.0.1/apache_pb22.png	

	CNT=`expr $CNT + 1`
done
