#!/bin/bash

while true; do
	id=$(printf "%03X" $((RANDOM % 2)))
	message=4142434445464748
	echo "$id#$message"
	cansend can0 $id#$message
	sleep $((RANDOM % 3 + 1))
done
