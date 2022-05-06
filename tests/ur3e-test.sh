#!/bin/sh

test_log=test-ur3e.log

rm -f $client_log $test_log

ip_address=10.79.125.2

echo "Starting test"
for ((i = 0 ; i < 10 ; i++)); do

    for ((channel = 0 ; channel < 8 ; channel++)); do
        ./ur3e.exe write $ip_address $channel high  > $test_log 2>&1 
        rc=$?
        echo W ch: $channel bit: 1
    done

    for ((channel = 0 ; channel < 8 ; channel++)); do
        ./ur3e.exe read $ip_address $channel  > $test_log 2>&1 
        rc=$?
        echo R ch: $channel bit: $rc
    done

    for ((channel = 0 ; channel < 8 ; channel++)); do
        ./ur3e.exe write $ip_address $channel low  > $test_log 2>&1 
        rc=$?
        echo W ch: $channel bit: 0
    done

    for ((channel = 0 ; channel < 8 ; channel++)); do
        ./ur3e.exe read $ip_address $channel  > $test_log 2>&1 
        rc=$?
        echo R ch: $channel bit: $rc
    done


done


exit $rc
