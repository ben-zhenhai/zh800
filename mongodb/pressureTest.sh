#! /bin/bash

while true
do
    now=$(date +"%I:%M:%S")
    #hostname=$(hostname)
    ip=$(ifconfig eth0 | grep -Eo 'inet (addr:)?([0-9]*\.){3}[0-9]*' | grep -Eo '([0-9]*\.){3}[0-9]*' | grep -v '127.0.0.1')
    node SendDataClient.js 01	BL20135	1000000	1	"$now"	1	"$ip"	3	E35	6957	0	0	0	03
    #echo $now
    #echo $hostname
    #echo $ip
    #sleep 3
done
