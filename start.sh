#!/bin/bash

./identity 8081 &
./reverse 8082 &
./lower 8084 &
./upper 8083 &
./ceasar 8085 &
./yours 8086 &


identity_pid=$(pidof ./identity)
reverse_pid=$(pidof ./reverse)
lower_pid=$(pidof ./lower)
upper_pid=$(pidof ./upper)
yours_pid=$(pidof ./yours)

read varname

if [ $varname == "yes" ];
then
    kill $identity_pid &
    kill $reverse_pid &
    kill $ceasar_pid &
    kill $lower_pid &
    kill $upper_pid &
    kill $yours_pid &
fi

exit 0
