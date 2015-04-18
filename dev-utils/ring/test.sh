#!/bin/bash

function echo_number()
{
	local num=0

	while True
	do
		if [ $num -gt 9 ]
		then
			num=0
		fi
		echo -n $num$num$num$num$num
		num=`expr $num + 1`
		sleep 1
	done
}

echo_number | ./ring -o result.txt -l 48
