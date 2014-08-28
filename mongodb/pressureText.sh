#! /bin/bash

while true
do
    now=$(date +"%I:%M:%S")
    node SendDataClient.js 01	BL20135	1000000	1	"$now"	1	0.0.0.0	3	E35	6957	0	0	0	03
    #sleep 3
done
