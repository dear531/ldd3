#!/bin/bash
#for scull_block bolck and nonblok
for((i=1;i<=2047;i++));
do
	echo 0 >> /dev/scullb0
done
	echo 1 >> /dev/scullb0
